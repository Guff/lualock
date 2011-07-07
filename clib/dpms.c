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
	int err = DPMSSetTimeouts(dpy, 5, 100, 200);
	DPMSCapable(dpy);
	printf("%i %i %i %i\n", lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4), err);
    }
    return 0;
}

void lualock_lua_dpms_init(lua_State *L) {
    lua_register(L, "dpms", lualock_lua_dpms_set);
}
