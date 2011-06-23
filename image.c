#include <lualib.h>
#include <lauxlib.h>

#include "globalconf.h"

int luaH_set_background_image(lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);
	cairo_surface_t *image = cairo_image_surface_create_from_png(filename);
	cairo_set_source_surface(lualock.cr, image, 0, 0);
	cairo_paint(lualock.cr);
	XClearWindow(lualock.dpy, lualock.win);
	return 0;
}

const struct luaL_reg lualock_image_lib[] =
{
	{ "set_background", luaH_set_background_image },
	{ NULL, NULL }
};
