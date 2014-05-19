#define luaL_register(L,n,l)    (luaL_openlib(L,(n),(l),0))
#include <glib.h>
#include <lauxlib.h>
#include <unistd.h>

#include "lualock.h"
#include "misc.h"
#include "drawing.h"
#include "clib/utils.h"

static void do_nothing_event_handler(GdkEvent *ev, gpointer data) {
}

static gboolean sleep_timeout_cb(gpointer data) {
    reset_password();
    gdk_event_handler_set(event_handler, NULL, NULL);
    draw_password_mask();
    return FALSE;
}

static void spawn_child_callback(gpointer data) {
    sigset_t empty;

    sigemptyset(&empty);
    sigprocmask(SIG_SETMASK, &empty, NULL);
    setsid();
}

static GPid spawn_command(const gchar *command_line, GError **error) {
    gboolean retval;
    GPid pid;
    gchar **argv = NULL;

    if(!g_shell_parse_argv(command_line, NULL, &argv, error))
        return 0;

    retval = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
                           spawn_child_callback, NULL, &pid, error);
    g_strfreev (argv);

    if (!retval)
        return 0;
    return pid;
}

static gint lualock_lua_spawn(lua_State *L) {
    lua_settop(L, 1);
    GError *error = NULL;
    GPid pid = spawn_command(luaL_checkstring(L, 1), &error);
    if (!pid) {
        lua_pushstring(L, error->message);
        g_error_free(error);
    }
    
    return 1;
}

static gint lualock_lua_get_data_dir(lua_State *L) {
    lua_pushliteral(L, LUALOCK_DATA_DIR);
    return 1;
}

static gint lualock_lua_get_config_dir(lua_State *L) {
    lua_pushstring(L, g_build_filename(g_get_user_config_dir(), "lualock", NULL));
    return 1;
}

static gint lualock_lua_sleep(lua_State *L) {
    gdk_event_handler_set(do_nothing_event_handler, NULL, NULL);
    g_timeout_add(1000 * lua_tonumber(L, 1), sleep_timeout_cb, NULL);
    return 0;
}

static gint lualock_lua_get_screen_width(lua_State *L) {
    lua_pushinteger(L, gdk_screen_get_width(lualock.scr));
    return 1;
}

static gint lualock_lua_get_screen_height(lua_State *L) {
    lua_pushinteger(L, gdk_screen_get_height(lualock.scr));
    return 1;
}

static gint lualock_lua_parse_color(lua_State *L) {
    gdouble r, g, b, a;
    parse_color(luaL_checkstring(L, 1), &r, &g, &b, &a);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    lua_pushnumber(L, a);
    return 4;
}

void lualock_lua_utils_init(lua_State *L) {
    const struct luaL_Reg lualock_utils_lib[] = {
        { "spawn", lualock_lua_spawn },
        { "get_data_dir", lualock_lua_get_data_dir },
        { "get_config_dir", lualock_lua_get_config_dir },
        { "sleep", lualock_lua_sleep },
        { "screen_width", lualock_lua_get_screen_width },
        { "screen_height", lualock_lua_get_screen_height },
        { "parse_color", lualock_lua_parse_color },
        { NULL, NULL }
    };
    luaL_register(L, "utils", lualock_utils_lib);
}

