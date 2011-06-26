#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <cairo-xlib.h>
#include <X11/Intrinsic.h>
#include <security/pam_appl.h>


#include "globalconf.h"
#include "lua_api.h"

extern void lualock_lua_background_init(lua_State *L);

lualock_t lualock;

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
    
    lualock.pmap = XCreatePixmap(dpy, lualock.win,
                                    DisplayWidth(dpy, scr),
                                    DisplayHeight(dpy, scr),
                                    DefaultDepth(dpy, scr));
        
    XSetWindowBackgroundPixmap(dpy, lualock.win, lualock.pmap);
    
}

void init_cairo() {
    Display *dpy = lualock.dpy;
    int scr = lualock.scr;
    lualock.surface = cairo_xlib_surface_create(dpy, lualock.pmap,
                                                DefaultVisual(dpy, scr),
                                                DisplayWidth(dpy, scr),
                                                DisplayHeight(dpy, scr));
    
    lualock.cr = cairo_create(lualock.surface);
    
}

void init_lua() {
    xdgHandle xdg;
    xdgInitHandle(&xdg);

    lualock.L = luaL_newstate();
    
    lualock_lua_image_init(lualock.L);
    // stuff's already loaded; don't need it on the stack
    lua_pop(lualock.L, 1);
    
    lualock_lua_background_init(lualock.L);
    lua_pop(lualock.L, 1);

    lualock_lua_loadrc(lualock.L, &xdg);    
}

bool on_key_press(XEvent ev) {
    char ascii;
    KeySym keysym;
    XComposeStatus compstatus;
    XLookupString(&ev.xkey, &ascii, 1, &keysym, &compstatus);
    
    switch(keysym) {
        case XK_Return:
        case XK_KP_Enter:
            return false;
        default:
            if (isprint(ascii) && (keysym < XK_Shift_L || keysym > XK_Hyper_R)) {
                // if we're running short on memory for the buffer, double it
                if (lualock.pw_alloc <= lualock.pw_length + 1) {
                    lualock.password = realloc(lualock.password, 2 * lualock.pw_alloc);
                    lualock.pw_alloc *= 2;
                }
                lualock.password[lualock.pw_length] = ascii;
                lualock.password[lualock.pw_length + 1] = '\0';
                lualock.pw_length++;
            }
    }
    
    return true;
}

void on_expose() {
    XClearWindow(lualock.dpy, lualock.win);
}

void reset_password() {
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
}

void event_handler(Display *dpy, Window win) {
    XEvent ev;
    XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(dpy, win, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
    
    while (!XNextEvent(dpy, &ev)) {
        if (ev.type == KeyPress) {
            // if enter was pressed, leave so password can be checked
            if (!on_key_press(ev)) {
                break;
            }
        } else if (ev.type == Expose) {
            on_expose();
        }
    }
}

static int pam_conv_cb(int msgs, const struct pam_message **msg,
                       struct pam_response **resp, void *appdata_ptr) {
    
    event_handler(lualock.dpy, lualock.win);
    
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
    
    init_display();
    init_window();
    init_cairo();
    init_lua();
    
    dpy = lualock.dpy;
    win = lualock.win;
    
    XMapRaised(dpy, win);
    XClearWindow(dpy, win);
    
    lualock.password = malloc(PW_BUFF_SIZE * sizeof(char));
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
    lualock.pw_alloc = PW_BUFF_SIZE;
    
    struct pam_conv conv = {pam_conv_cb, NULL};
    int ret = pam_start("slimlock", getenv("USER"), &conv, &lualock.pam_handle);
    // if PAM doesn't get set up correctly, we can't authenticate. so, bail out
    if (ret != PAM_SUCCESS)
        exit(EXIT_FAILURE);
    
    while (True) {
        if (authenticate_user())
            break;
    }
    
    
    XCloseDisplay(dpy);
    
    return 0;
}
