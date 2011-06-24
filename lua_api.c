#include "lua_api.h"
#include "globalconf.h"

bool lualock_lua_loadrc(lua_State *L, xdgHandle *xdg) {
    char *config = xdgConfigFind("lualock/rc.lua", xdg);
    if (luaL_loadfile(L, config) || lua_pcall(L, 0, 0, 0))
        return false;
    return true;
}
