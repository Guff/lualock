#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <gdk/gdkkeysyms.h>
#include <security/pam_appl.h>

#define PW_BUFF_SIZE 32
#define DEFAULT_FONT "Sans 12"

#include "lualock.h"
#include "misc.h"
#include "drawing.h"
#include "lua_api.h"

lualock_t lualock;

time_t test_timer;
int frames_drawn;

struct {
    gboolean no_daemon;
} prefs;

static GOptionEntry options[] = {
    { "no-daemon", 'n', 0, G_OPTION_ARG_NONE, &prefs.no_daemon, "Don't run as a"
        " daemon; lock the screen immediately and exit when done", NULL },
    { NULL }
};

gboolean on_key_press(GtkWidget *widget, GdkEvent *ev, gpointer data);

void init_display() {
    lualock.scr = gdk_screen_get_default();
}

void init_window() {
    lualock.win = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_default_size(GTK_WINDOW(lualock.win),
                                gdk_screen_get_width(lualock.scr),
                                gdk_screen_get_height(lualock.scr));
    GtkWidget *stage_widget = gtk_clutter_embed_new();
    lualock.stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(stage_widget));
    gtk_container_add(GTK_CONTAINER(lualock.win), stage_widget);
    g_signal_connect(lualock.win, "key-press-event", G_CALLBACK(on_key_press), NULL);
    gtk_widget_show_all(lualock.win);
}

void init_clutter() {
    lualock.pw_actor = clutter_cairo_texture_new(lualock.style.width,
                                                 lualock.style.height);
    clutter_actor_set_position(lualock.pw_actor, lualock.style.x, lualock.style.y);
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
    lualock.L = luaL_newstate();
    
    lualock_lua_loadrc(lualock.L);
}

void reset_password() {
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
}

gboolean authenticate_user() {
    return (pam_authenticate(lualock.pam_handle, 0) == PAM_SUCCESS);
}

gboolean on_key_press(GtkWidget *widget, GdkEvent *ev, gpointer data) {
    guint keyval = ((GdkEventKey *)ev)->keyval;
    switch(keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            if (authenticate_user())
                gtk_main_quit();
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
    
    draw_password_mask();
    
    return TRUE;
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

int main(int argc, char **argv) {   
    GError *error = NULL;
    GOptionContext *opt_context = g_option_context_new("- screenlocker");
    g_option_context_add_main_entries(opt_context, options, NULL);
    g_option_context_add_group(opt_context, gtk_get_option_group(TRUE));
    g_option_context_add_group(opt_context, cogl_get_option_group());
    g_option_context_add_group(opt_context, clutter_get_option_group_without_init());
    g_option_context_add_group(opt_context, gtk_clutter_get_option_group());
    if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(1);
    }
    
    Display *dpy;
    GdkWindow *win;
    
    lualock.password = calloc(PW_BUFF_SIZE, sizeof(char));
    lualock.pw_length = 0;
    lualock.pw_alloc = PW_BUFF_SIZE;
    
    lualock.using_dpms = FALSE;
    
    init_display();
    init_window();
    init_style();
    init_clutter();
    
    dpy = XOpenDisplay(NULL);
    win = gtk_widget_get_window(lualock.win);

    init_lua();
    
    draw_password_mask();
    clutter_container_add_actor(CLUTTER_CONTAINER(lualock.stage), lualock.pw_actor);
    
    CARD16 dummy;
    DPMSInfo(dpy, &dummy, &lualock.dpms_enabled);
    if (DPMSCapable(dpy) && lualock.using_dpms) {
        DPMSGetTimeouts(dpy, &lualock.dpms_standby,
                        &lualock.dpms_suspend, &lualock.dpms_off);
        printf("%i\n", DPMSSetTimeouts(dpy, lualock.dpms_cfg_standby, lualock.dpms_cfg_suspend,
                        lualock.dpms_cfg_off));
    }
    
    gdk_window_show(win);
    
    struct pam_conv conv = {pam_conv_cb, NULL};
    int ret = pam_start("lualock", getenv("USER"), &conv, &lualock.pam_handle);
    // if PAM doesn't get set up correctly, we can't authenticate. so, bail out
    if (ret != PAM_SUCCESS)
        exit(EXIT_FAILURE);
        
    gdk_keyboard_grab(win, TRUE, GDK_CURRENT_TIME);
    gdk_pointer_grab(win, TRUE, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                     | GDK_POINTER_MOTION_MASK, NULL, NULL, GDK_CURRENT_TIME);
    printf("%i", prefs.no_daemon);
    gtk_main();
    
    DPMSSetTimeouts(dpy, lualock.dpms_standby, lualock.dpms_suspend, lualock.dpms_off);
    //XCloseDisplay(dpy);
    
    return 0;
}
