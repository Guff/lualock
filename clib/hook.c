#include <lauxlib.h>
#include <string.h>

#include "lualock.h"
#include "lua_api.h"
#include "clib/hook.h"

typedef struct {
    lua_State *L;
    int r;
} hook_data_t;

void hook_call_lua_function(gpointer data) {
    hook_data_t *hook_data = data;
    lua_rawgeti(hook_data->L, LUA_REGISTRYINDEX, hook_data->r);
    lualock_lua_do_function(hook_data->L);
}

int lualock_lua_hook_connect(lua_State *L) {
    lua_settop(L, 2);
    const char *hook_str = luaL_checkstring(L, 1);
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
    const struct luaL_reg lualock_hook_lib[] = {
        { "connect", lualock_lua_hook_connect },
        { NULL, NULL }
    };
    luaL_register(L, "hook", lualock_hook_lib);
}
