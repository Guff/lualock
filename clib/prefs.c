#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

#include "misc.h"
#include "clib/prefs.h"

int lualock_lua_prefs_set(lua_State *L) {
    lua_getfield(L, 1, "timeout");
    lua_getfield(L, 1, "dpms");
    lua_getfield(L, 3, "standby");
    lua_getfield(L, 3, "suspend");
    lua_getfield(L, 3, "off");
    lualock.timeout = luaL_checkint(L, 2);
    lualock.dpms_cfg_standby = luaL_checkint(L, 4);
    lualock.dpms_cfg_suspend = luaL_checkint(L, 5);
    lualock.dpms_cfg_off = luaL_checkint(L, 6);
    lualock.using_dpms = True;
    return 0;
}

void lualock_lua_prefs_init(lua_State *L) {
    lua_register(L, "prefs", lualock_lua_prefs_set);
}
