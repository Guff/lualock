#include "misc.h"
#include "clib/prefs.h"

int lualock_lua_prefs_set(lua_State *L) {
    lua_getfield(L, 1, "timeout");
    lualock.timeout = luaL_checkint(L, 2);
    
    return 0;
}

void lualock_lua_prefs_init(lua_State *L) {
    lua_register(L, "prefs", lualock_lua_prefs_set);
}
