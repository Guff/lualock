#include <glib.h>
#include <lauxlib.h>
#include <unistd.h>

#include "clib/spawn.h"

static void spawn_child_callback(gpointer user_data) {
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

int lualock_lua_spawn(lua_State *L) {
	lua_settop(L, 1);
	GError *error = NULL;
	GPid pid = spawn_command(luaL_checkstring(L, 1), &error);
	if (!pid) {
		lua_pushstring(L, error->message);
		g_error_free(error);
	}
	
	return 1;
}

void lualock_lua_spawn_init(lua_State *L) {
	lua_register(L, "spawn", lualock_lua_spawn);
}
