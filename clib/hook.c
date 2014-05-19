//      hook.c - API for registering hooks within lualock
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

#define luaL_register(L,n,l)    (luaL_openlib(L,(n),(l),0))
#include <lauxlib.h>
#include <string.h>

#include "lualock.h"
#include "lua_api.h"
#include "clib/hook.h"

typedef struct {
    lua_State *L;
    gint r;
} hook_data_t;

static void hook_call_lua_function(gpointer data) {
    hook_data_t *hook_data = data;
    lua_rawgeti(hook_data->L, LUA_REGISTRYINDEX, hook_data->r);
    lualock_lua_do_function(hook_data->L);
}

static gint lualock_lua_hook_connect(lua_State *L) {
    lua_settop(L, 2);
    const gchar *hook_str = luaL_checkstring(L, 1);
    GHookList *list = g_hash_table_lookup(lualock.hooks, hook_str);
    luaL_argcheck(L, list, 1, "not a valid hook");
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    GHook *hook = g_hook_alloc(list);
    hook_data_t *data = g_malloc(sizeof(hook_data_t));
    data->L = L;
    data->r = luaL_ref(L, LUA_REGISTRYINDEX);
    hook->data = data;
    hook->func = hook_call_lua_function;
    
    g_hook_append(list, hook);
    
    return 0;
}

void lualock_lua_hook_init(lua_State *L) {
    const struct luaL_Reg lualock_hook_lib[] = {
        { "connect", lualock_lua_hook_connect },
        { NULL, NULL }
    };
    luaL_register(L, "hook", lualock_hook_lib);
}
