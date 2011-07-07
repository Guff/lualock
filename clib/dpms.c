#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

#include "misc.h"
#include "clib/dpms.h"

int lualock_lua_dpms_set(lua_State *L) {
    Display *dpy = XOpenDisplay(NULL);
    if (DPMSCapable(dpy)) {
	lua_getfield(L, 1, "standby");
	lua_getfield(L, 1, "suspend");
	lua_getfield(L, 1, "off");
	DPMSEnable(dpy);
	DPMSSetTimeouts(dpy, luaL_checkint(L, 2), luaL_checkint(L, 3),
			luaL_checkint(L, 4));
    }
    return 0;
}

void lualock_lua_dpms_init(lua_State *L) {
    lua_register(L, "dpms", lualock_lua_dpms_set);
}
