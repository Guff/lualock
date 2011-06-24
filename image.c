#include <lualib.h>
#include <lauxlib.h>
#include <Imlib2.h>

#include "globalconf.h"
#include "image.h"

static int lualock_lua_image_new(lua_State *L) {
	image_t *image = lua_newuserdata(L, sizeof(image_t));
	
	image->image = imlib_load_image(luaL_checkstring(L, 1));
	imlib_context_set_image(image->image);
	image->data = (unsigned char *) imlib_image_get_data_for_reading_only();
	
	return 1;
}

static int lualock_lua_image_set_background(lua_State *L) {
	image_t *image = (image_t *) lua_touserdata(L, 1);
	int width, height;
	
	imlib_context_set_image(image->image);
	width = imlib_image_get_width();
	height = imlib_image_get_height();
	cairo_surface_t *surface =
		cairo_image_surface_create_for_data(image->data, CAIRO_FORMAT_ARGB32,
											width, height,
											cairo_format_stride_for_width(
												CAIRO_FORMAT_ARGB32,
												width));
	cairo_set_source_rgba(lualock.cr, 0, 0, 0, 1);
	cairo_paint(lualock.cr);
	cairo_scale(lualock.cr, 0.5, 0.5);
	cairo_set_source_surface(lualock.cr, surface, 0, 0);
	cairo_paint(lualock.cr);
	XClearWindow(lualock.dpy, lualock.win);
	return 0;
}

const struct luaL_reg lualock_image_lib[] =
{
	{ "image_new", lualock_lua_image_new },
	{ "set_background", lualock_lua_image_set_background },
	{ NULL, NULL }
};
