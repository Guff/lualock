#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <security/pam_appl.h>

#define PW_BUFF_SIZE 32
#define DEFAULT_FONT "Sans 12"

#include "lualock.h"
#include "misc.h"
#include "drawing.h"
#include "lua_api.h"

extern void lualock_lua_background_init(lua_State *L);

lualock_t lualock;

time_t test_timer;
int frames_drawn;

void init_display() {
    lualock.dpy = XOpenDisplay(NULL);
    lualock.scr = DefaultScreen(lualock.dpy);
}

void init_window() {
    Display *dpy = lualock.dpy;
    int scr = lualock.scr;
    
    XSetWindowAttributes attrs;
    attrs.override_redirect = 1;
    attrs.background_pixel = BlackPixel(dpy, scr);
    unsigned long attr_mask = CWOverrideRedirect | CWBackPixel;
    
    lualock.win = XCreateWindow(dpy, RootWindow(dpy, scr), 0, 0,
                                   DisplayWidth(dpy, scr),
                                   DisplayHeight(dpy, scr),
                                   0, DefaultDepth(dpy, scr),
                                   CopyFromParent, DefaultVisual(dpy, scr),
                                   attr_mask, &attrs);
}

void init_cairo() {
    Display *dpy = lualock.dpy;
    int scr = lualock.scr;
    
    lualock.surfaces_alloc = 20;
    lualock.surfaces = malloc(lualock.surfaces_alloc * sizeof(cairo_surface_t *));
    lualock.surfaces[0] = NULL;
    
    lualock.surface = cairo_xlib_surface_create(dpy, lualock.win,
                                                DefaultVisual(dpy, scr),
                                                DisplayWidth(dpy, scr),
                                                DisplayHeight(dpy, scr));
    
    lualock.surface_buf = create_surface();
    
    lualock.pw_surface = create_surface();
}

void init_style() {
    lualock.style.font = DEFAULT_FONT;
    lualock.style.x = 400;
    lualock.style.y = 540;
    lualock.style.off_x = 5;
    lualock.style.off_y = 5;
    lualock.style.width = 200;
    lualock.style.height = 24;
    lualock.style.r = 0;
    lualock.style.g = 0;
    lualock.style.b = 0;
    lualock.style.a = 1;
}

void init_lua() {
    xdgHandle xdg;
    xdgInitHandle(&xdg);

    lualock.L = luaL_newstate();
    
    luaL_openlibs(lualock.L);
    
    lualock_lua_image_init(lualock.L);
    // stuff's already loaded; don't need it on the stack
    lua_pop(lualock.L, 1);
    
    lualock_lua_background_init(lualock.L);
    lua_pop(lualock.L, 1);
    
    lualock_lua_text_init(lualock.L);
    lua_pop(lualock.L, 1);
    
    lualock_lua_timer_init(lualock.L);
    lua_pop(lualock.L, 1);
    
    lualock_lua_style_init(lualock.L);
    lua_pop(lualock.L, 1);
    
    lualock_lua_loadrc(lualock.L, &xdg);    
}

Bool on_key_press(XEvent ev) {
    char buf[32];
    KeySym keysym;
    XLookupString(&ev.xkey, buf, sizeof(buf), &keysym, NULL);
    
    switch(keysym) {
        case XK_Return:
        case XK_KP_Enter:
            return False;
        case XK_Escape:
            lualock.password[0] = '\0';
            lualock.pw_length = 0;
            break;
        case XK_BackSpace:
        case XK_KP_Delete:
        case XK_Delete:
            if (lualock.pw_length > 0) {
                lualock.pw_length--;
                lualock.password[lualock.pw_length] = '\0';
            }
        default:
            if (isprint(buf[0]) && (keysym < XK_Shift_L || keysym > XK_Hyper_R)) {
                // if we're running short on memory for the buffer, grow it
                if (lualock.pw_alloc <= lualock.pw_length + strlen(buf)) {
                    lualock.password = realloc(lualock.password,
                        lualock.pw_alloc + PW_BUFF_SIZE);
                    lualock.pw_alloc += PW_BUFF_SIZE;
                }
                strcat(lualock.password, buf);
                lualock.password[lualock.pw_length + strlen(buf)] = '\0';
                lualock.pw_length++;
            }
    }
    draw_password_mask();
    
    return True;
}

void reset_password() {
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
}

void event_handler() {
    XEvent ev;
    
    while (!XNextEvent(lualock.dpy, &ev)) {
        switch (ev.type) {
            case KeyPress:
                // if enter was pressed, leave so password can be checked
                if (!on_key_press(ev)) {
                    return;
                }
                break;
            case Expose:
                //draw();
                break;
            default:
                break;
        }
    }
}

static int pam_conv_cb(int msgs, const struct pam_message **msg,
                       struct pam_response **resp, void *data) {
    event_handler();
    
    *resp = (struct pam_response *) calloc(msgs, sizeof(struct pam_message));
    if (msgs == 0 || *resp == NULL)
        return 1;
    for (int i = 0; i < msgs; i++) {
        if (msg[i]->msg_style != PAM_PROMPT_ECHO_OFF &&
            msg[i]->msg_style != PAM_PROMPT_ECHO_ON)
            continue;

        // return code is currently not used but should be set to zero
        resp[i]->resp_retcode = 0;
        if ((resp[i]->resp = strdup(lualock.password)) == NULL)
            return 1;
    }
    
    reset_password();
    
    return 0;
}

bool authenticate_user() {
    return (pam_authenticate(lualock.pam_handle, 0) == PAM_SUCCESS);
}

int main() {    
    Display *dpy;
    Window win;
    
    lualock.password = calloc(PW_BUFF_SIZE, sizeof(char));
    lualock.pw_length = 0;
    lualock.pw_alloc = PW_BUFF_SIZE;
    
    init_display();
    init_window();
    init_style();
    
    dpy = lualock.dpy;
    win = lualock.win;
    
    XMapWindow(dpy, win);

    init_cairo();
    init_lua();
    
    struct pam_conv conv = {pam_conv_cb, NULL};
    int ret = pam_start("lualock", getenv("USER"), &conv, &lualock.pam_handle);
    // if PAM doesn't get set up correctly, we can't authenticate. so, bail out
    if (ret != PAM_SUCCESS)
        exit(EXIT_FAILURE);
        
    XGrabKeyboard(dpy, lualock.win, True, GrabModeAsync, GrabModeAsync,
                  CurrentTime);
    XGrabPointer(dpy, lualock.win, False, ButtonPressMask | ButtonReleaseMask
                 | PointerMotionMask, GrabModeAsync, GrabModeAsync, lualock.win,
                 None, CurrentTime);

    start_drawing();
    
    while (True) {
        if (authenticate_user())
            break;
    }
    
    XCloseDisplay(dpy);
    
    return 0;
}
