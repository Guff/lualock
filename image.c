#include <lualib.h>
#include <lauxlib.h>
#include <Imlib2.h>

#include "globalconf.h"

int lualock_lua_set_background_image(lua_State *L) {
	unsigned char *image_data;
	int width, height;
	const char *filename = luaL_checkstring(L, 1);
	Imlib_Image image_file = imlib_load_image(filename);
	imlib_context_set_image(image_file);
	width = imlib_image_get_width();
	height = imlib_image_get_height();
	image_data = (unsigned char *) imlib_image_get_data_for_reading_only();
	cairo_surface_t *image =
		cairo_image_surface_create_for_data(image_data, CAIRO_FORMAT_ARGB32,
											width, height,
											cairo_format_stride_for_width(
												CAIRO_FORMAT_ARGB32,
												width));
	cairo_set_source_surface(lualock.cr, image, 0, 0);
	cairo_paint(lualock.cr);
	XClearWindow(lualock.dpy, lualock.win);
	return 0;
}

const struct luaL_reg lualock_image_lib[] =
{
	{ "set_background", lualock_lua_set_background_image },
	{ NULL, NULL }
};
