#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

#include "misc.h"
#include "clib/dpms.h"

int lualock_lua_dpms_set(lua_State *L) {
    lua_getfield(L, 1, "standby");
    lua_getfield(L, 1, "suspend");
    lua_getfield(L, 1, "off");
    lualock.dpms_cfg_standby = luaL_checkint(L, 2);
    lualock.dpms_cfg_suspend = luaL_checkint(L, 3);
    lualock.dpms_cfg_off = luaL_checkint(L, 4);
    lualock.using_dpms = True;
    return 0;
}

void lualock_lua_dpms_init(lua_State *L) {
    lua_register(L, "dpms", lualock_lua_dpms_set);
}
