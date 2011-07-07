#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <pango/pangocairo.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
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
    lualock.scr = gdk_screen_get_default();
}

void init_window() {
    GdkWindowAttr attrs;
    attrs.override_redirect = TRUE;
    attrs.width = gdk_screen_get_width(lualock.scr);
    attrs.height = gdk_screen_get_height(lualock.scr);
    attrs.window_type = GDK_WINDOW_TOPLEVEL;
    attrs.wclass = GDK_INPUT_OUTPUT;
    attrs.x = 0;
    attrs.y = 0;
    unsigned long attr_mask = GDK_WA_NOREDIR | GDK_WA_X | GDK_WA_Y;
    
    lualock.win = gdk_window_new(gdk_get_default_root_window(), &attrs, attr_mask);
                                 
    gdk_window_show(lualock.win);
}

void init_cairo() {
    lualock.layers_alloc = 20;
    lualock.layers = malloc(lualock.layers_alloc * sizeof(layer_t));
    lualock.layers[0] = NULL;
    
    lualock.surface_buf = create_surface(0, 0);
    
    lualock.pw_surface = create_surface(0, 0);
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
    
    lualock_lua_loadrc(lualock.L, &xdg);    
}

void reset_password() {
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
}


gboolean on_key_press(GdkEvent *ev) {
    guint keyval = ((GdkEventKey *)ev)->keyval;
    switch(keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            return FALSE;
        case GDK_KEY_BackSpace:
            lualock.pw_length--;
            lualock.password[lualock.pw_length] = '\0';
            break;
        case GDK_KEY_Escape:
            reset_password();
            break;
        default:
            if (isprint(gdk_keyval_to_unicode(keyval))) {
                // if we're running short on memory for the buffer, grow it
                if (lualock.pw_alloc <= lualock.pw_length + 1) {
                    lualock.password = realloc(lualock.password, lualock.pw_alloc + 32);
                    lualock.pw_alloc += 32;
                }
                lualock.password[lualock.pw_length] = gdk_keyval_to_unicode(keyval);
                lualock.password[lualock.pw_length + 1] = '\0';
                lualock.pw_length++;
            }
    }
    
    lualock.need_updates = TRUE;
    draw_password_mask();
    
    return TRUE;
}


gboolean authenticate_user() {
    return (pam_authenticate(lualock.pam_handle, 0) == PAM_SUCCESS);
}

void event_handler(GdkEvent *ev) {
    switch (ev->type) {
        case GDK_KEY_PRESS:
        // if enter was pressed, check password
            if (!on_key_press(ev))
                if (authenticate_user())
                    g_main_loop_quit(lualock.loop);
            break;
        case GDK_EXPOSE:
            lualock.need_updates = TRUE;
            break;
        default:
            break;
    }
}

static int pam_conv_cb(int msgs, const struct pam_message **msg,
                       struct pam_response **resp, void *data) {
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

void restore_dpms_settings() {
    if (lualock.dpms_enabled) {
        int err = DPMSSetTimeouts(lualock.dpy,
                                  lualock.dpms_standby, lualock.dpms_suspend,
                                  lualock.dpms_off);
        printf("%i %i %i %i", lualock.dpms_standby, lualock.dpms_suspend, lualock.dpms_off, err);
    }
}

int main(int argc, char **argv) {    
    Display *dpy;
    GdkWindow *win;
    
    gdk_init(&argc, &argv);
    
    lualock.password = calloc(PW_BUFF_SIZE, sizeof(char));
    lualock.pw_length = 0;
    lualock.pw_alloc = PW_BUFF_SIZE;
    
    lualock.need_updates = FALSE;
    
    init_display();
    init_window();
    init_style();
    init_cairo();
    
    dpy = lualock.dpy;
    win = lualock.win;

    CARD16 dummy;
    DPMSInfo(dpy, &dummy, &lualock.dpms_enabled);
    if (lualock.dpms_enabled)
        DPMSGetTimeouts(dpy, &lualock.dpms_standby,
                        &lualock.dpms_suspend, &lualock.dpms_off);
    
    atexit(restore_dpms_settings);
    printf("%i %i %i\n", lualock.dpms_standby, lualock.dpms_suspend, lualock.dpms_off);
    
    gdk_window_show(win);
    
    gdk_event_handler_set((GdkEventFunc)event_handler, NULL, NULL);

    init_lua();
    
    struct pam_conv conv = {pam_conv_cb, NULL};
    int ret = pam_start("lualock", getenv("USER"), &conv, &lualock.pam_handle);
    // if PAM doesn't get set up correctly, we can't authenticate. so, bail out
    if (ret != PAM_SUCCESS)
        exit(EXIT_FAILURE);
        
    gdk_keyboard_grab(win, TRUE, GDK_CURRENT_TIME);
    gdk_pointer_grab(win, TRUE, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                     | GDK_POINTER_MOTION_MASK, NULL, NULL, GDK_CURRENT_TIME);
    
    g_timeout_add(1000.0 / 10, draw, NULL);
    lualock.loop = g_main_new(TRUE);
    g_main_run(lualock.loop);
    
    //XCloseDisplay(dpy);
    
    return 0;
}
