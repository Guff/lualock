//      lualock.c - A highly configurable screenlocker
//      Copyright ©2011 Guff <cassmacguff@gmail.com>
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <security/pam_appl.h>
#include <sys/file.h>
#include <errno.h>

#define PW_BUFF_SIZE 32
#define DEFAULT_FONT "Sans 12"

#include "lualock.h"
#include "misc.h"
#include "drawing.h"
#include "lua_api.h"
#include "clib/keybinder.h"

lualock_t lualock;


time_t test_timer;
int frames_drawn;

struct {
    gboolean no_daemon;
    gboolean test;
} prefs;

static const char *hook_names[] = { "lock",
                                    "unlock",
                                    "auth-failed",
                                    "key-press",
                                    NULL };

static GOptionEntry options[] = {
    { "no-daemon", 'n', 0, G_OPTION_ARG_NONE, &prefs.no_daemon, "Don't run as a"
      " daemon; lock the screen immediately and exit when done", NULL },
    { "test", 't', 0, G_OPTION_ARG_NONE, &prefs.test, "Don't lock the screen;"
      " just run once and exit on first keypress", NULL },
    // get gcc to shut up about missing initializers in this instance
    { NULL, 0, 0, 0, NULL, NULL, NULL }
};

gboolean on_key_press(GdkEvent *ev);
void show_lock();
void hide_lock();

void init_window() {
    GdkWindowAttr attrs;

    lualock.scr = gdk_screen_get_default();
    
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

void init_timers() {
    lualock.timers = g_array_new(TRUE, TRUE, sizeof(guint));
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

void init_cairo() {
    lualock.layers = g_ptr_array_new();
    
    lualock.surface_buf = create_surface(0, 0);
    
    lualock.pw_surface = create_surface(0, 0);
    
    lualock.bg_surface = create_surface(0, 0);
    
    lualock.updates_needed = cairo_region_create();
}

void init_lua() {
    lualock.L = luaL_newstate();
    
    lualock_lua_loadrc(lualock.L);
}

void init_hook_table() {
    lualock.hooks = g_hash_table_new(g_str_hash, g_str_equal);
    
    for (int i = 0; hook_names[i]; i++) {
        GHookList *hook_list = g_malloc(sizeof(GHookList));
        g_hash_table_insert(lualock.hooks, strdup(hook_names[i]), hook_list);
    }
}

void init_hooks() {
    for (int i = 0; hook_names[i]; i++)
        g_hook_list_init(g_hash_table_lookup(lualock.hooks, hook_names[i]),
                         sizeof(GHook));
}

void init_keybinds() {
    lualock.keybinds = g_ptr_array_new();
}

void clear_keybinds() {
    if (lualock.keybinds->len)
        g_ptr_array_remove_range(lualock.keybinds, 0, lualock.keybinds->len);
}

void clear_hooks() {
    for (int i = 0; hook_names[i]; i++)
        g_hook_list_clear(g_hash_table_lookup(lualock.hooks, hook_names[i]));
}

void clear_layers() {
    for (guint i = 0; i < lualock.layers->len; i++)
        layer_destroy(g_ptr_array_index(lualock.layers, i));
    
    if (lualock.layers->len)
        g_ptr_array_remove_range(lualock.layers, 0, lualock.layers->len);
}        

void reset_password() {
    lualock.password[0] = '\0';
    lualock.pw_length = 0;
}

gboolean authenticate_user() {
    return (pam_authenticate(lualock.pam_handle, 0) == PAM_SUCCESS);
}

gboolean on_key_press(GdkEvent *ev) {
    guint keyval = ((GdkEventKey *)ev)->keyval;
    GdkModifierType mod = ((GdkEventKey *)ev)->state;
    gchar buf[6];
    guint32 uc;
    guint8 uc_len;
    switch(keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            return FALSE;
            break;
        case GDK_KEY_BackSpace:
            if (lualock.pw_length > 0)
                lualock.pw_length--;
            lualock.password[lualock.pw_length] = '\0';
            break;
        case GDK_KEY_Escape:
            reset_password();
            break;
        default:
            uc = gdk_keyval_to_unicode(keyval);
            uc_len = g_unichar_to_utf8(uc, buf);
            if (g_unichar_isprint(uc)) {
                // if we're running short on memory for the buffer, grow it
                if (lualock.pw_alloc <= lualock.pw_length + uc_len) {
                    lualock.password = realloc(lualock.password, lualock.pw_alloc + 32);
                    lualock.pw_alloc += 32;
                }
                strncat(lualock.password, buf, uc_len);
                lualock.pw_length += uc_len;
            }
    }
    
    draw_password_mask();
    
    // FIXME: I don't like having the lua stuff mixed in with the core stuff
    for (guint i = 0; i < lualock.keybinds->len; i++) {
        keybind_t *bind = g_ptr_array_index(lualock.keybinds, i);
        if (bind->val == keyval && bind->mod == mod) {
            lua_rawgeti(lualock.L, LUA_REGISTRYINDEX, bind->r);
            lualock_lua_do_function(lualock.L);
        }
    }
    
    g_hook_list_invoke(g_hash_table_lookup(lualock.hooks, "key-press"), FALSE);
    return TRUE;
}

void event_handler(GdkEvent *ev, gpointer data) {
    switch (ev->type) {
        case GDK_KEY_PRESS:
            if (prefs.test)
                g_main_loop_quit(lualock.loop);
        // if enter was pressed, check password
            if (!on_key_press(ev)) {
                if (authenticate_user())
                    g_main_loop_quit(lualock.loop);
                else
                    g_hook_list_invoke(
                        g_hash_table_lookup(lualock.hooks, "auth-failed"),
                        FALSE);
            }
            break;
        case GDK_EXPOSE:
            update_screen();
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

void show_lock() {
    init_hooks();
    init_lua();
    update_screen();
    gdk_keyboard_grab(lualock.win, TRUE, GDK_CURRENT_TIME);
    gdk_pointer_grab(lualock.win, TRUE, GDK_BUTTON_PRESS_MASK
                     | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK, NULL,
                     NULL, GDK_CURRENT_TIME);
    draw_password_mask();
    g_hook_list_invoke(g_hash_table_lookup(lualock.hooks, "lock"), FALSE);
    
    lualock.frame_timer_id = g_timeout_add(1000.0 / 20, draw, NULL);
    g_main_loop_run(lualock.loop);
    
    hide_lock();
}

void hide_lock() {
    g_hook_list_invoke(g_hash_table_lookup(lualock.hooks, "unlock"), FALSE);
    g_source_remove(lualock.frame_timer_id);
    lua_close(lualock.L);
    clear_timers();
    clear_updates();
    clear_keybinds();
    gdk_keyboard_ungrab(GDK_CURRENT_TIME);
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gdk_window_hide(lualock.win);
    clear_layers();
    
    clear_hooks();
}

int seconds_idle(Display *dpy, XScreenSaverInfo *xss_info) {
    XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), xss_info);
    return xss_info->idle / 1000;
}

int main(int argc, char **argv) {
    int lock_file = open("/var/lock/lualock.lock", O_CREAT | O_RDWR, 0666);
    int rc = flock(lock_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno)
            exit(1);
    }
    GError *error = NULL;
    GOptionContext *opt_context = g_option_context_new("- screenlocker");
    g_option_context_add_main_entries(opt_context, options, NULL);
    g_option_context_add_group(opt_context, gtk_get_option_group(TRUE));
    if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(1);
    }
    
    lualock.password = calloc(PW_BUFF_SIZE, sizeof(char));
    lualock.pw_length = 0;
    lualock.pw_alloc = PW_BUFF_SIZE;
        
    init_window();
    init_style();
    init_cairo();
    init_timers();
    init_hook_table();
    init_keybinds();
    
    struct pam_conv conv = {pam_conv_cb, NULL};
    int ret = pam_start("lualock", getenv("USER"), &conv, &lualock.pam_handle);
    // if PAM doesn't get set up correctly, we can't authenticate. so, bail out
    if (ret != PAM_SUCCESS)
        exit(EXIT_FAILURE);
    
    gdk_event_handler_set(event_handler, NULL, NULL);
    lualock.loop = g_main_loop_new(NULL, TRUE);

    Display *dpy = XOpenDisplay(NULL);
    XScreenSaverInfo *xss_info = XScreenSaverAllocInfo();
    
    if (prefs.no_daemon || prefs.test) {
        show_lock();
        return 0;
    }    
    
    /* dirty hack: the loop is dependent on values set by rc.lua, but we don't
     * want lua running the whole time. the easy solution is to execute rc.lua
     * and immediately close the lua instance, but it's not pretty */
    init_hooks();
    init_lua();
    lua_close(lualock.L);
    clear_hooks();
    
    int idle_time;
    while (TRUE) {
        while ((idle_time = seconds_idle(dpy, xss_info)) < lualock.timeout) {
            sleep(lualock.timeout - idle_time - 1);
        }
        show_lock();
    }
    
    return 0;
}
