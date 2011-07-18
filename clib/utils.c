#include <glib.h>
#include <lauxlib.h>
#include <unistd.h>

#include "clib/spawn.h"

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

static int lualock_lua_spawn(lua_State *L) {
	lua_settop(L, 1);
	GError *error = NULL;
	GPid pid = spawn_command(luaL_checkstring(L, 1), &error);
	if (!pid) {
		lua_pushstring(L, error->message);
		g_error_free(error);
	}
	
	return 1;
}

static int lualock_lua_get_data_dir(lua_State *L) {
	lua_pushliteral(L, LUALOCK_DATA_DIR);
	return 1;
}

static int lualock_lua_get_config_dir(lua_State *L) {
	lua_pushstring(L, g_build_filename(g_get_user_config_dir(), "lualock", NULL));
	return 1;
}

static int lualock_lua_sleep(lua_State *L) {
	usleep(1000000 * lua_tonumber(L, 1));
	return 0;
}

void lualock_lua_utils_init(lua_State *L) {
	const struct luaL_reg lualock_utils_lib[] = {
		{ "spawn", lualock_lua_spawn },
		{ "get_data_dir", lualock_lua_get_data_dir },
		{ "get_config_dir", lualock_lua_get_config_dir },
		{ "sleep", lualock_lua_sleep },
		{ NULL, NULL }
	};
	luaL_register(L, "utils", lualock_utils_lib);
}

