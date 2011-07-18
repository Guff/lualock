//      spawn.c - API for spawning processes
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
