#include <string.h>
#include <cairo-xlib.h>

#include "globalconf.h"
#include "background.h"
#include "image.h"

void background_set_color(const char *hex) {
	unsigned long packed_rgb;
	// hex + 1 skips over the pound sign, which we don't need
	sscanf(hex + 1, "%lx", &packed_rgb);
	double r = (packed_rgb >> 16) / 256.0;
	double g = (packed_rgb >> 8 & 0xff) / 256.0;
	double b = (packed_rgb & 0xff) / 256.0;
	
	cairo_set_source_rgb(lualock.cr, r, g, b);
	cairo_paint(lualock.cr);
}

//void background_stretch(image_t *image, double width, double height,
						//double win_width, double win_height) {
	
//}

static int lualock_lua_background_set(lua_State *L) {
	image_t *image = lua_touserdata(L, 1);
	
	// if parameter is not userdata, maybe the user wants a solid color
	if (!image) {
		background_set_color(luaL_checkstring(L, 1));
		return 0;
	}
	
	const char *style = lua_tostring(L, 2);
	double width, height, win_width, win_height;
	int off_x = 0;
	int off_y = 0;
	double scale_x = 1;
	double scale_y = 1;
	
	width = image_get_width(image);
	height = image_get_height(image);
	win_width = cairo_xlib_surface_get_width(lualock.surface);
	win_height = cairo_xlib_surface_get_height(lualock.surface);

	cairo_set_source_rgba(lualock.cr, 0, 0, 0, 1);
	cairo_paint(lualock.cr);

	if (style) {
		if (!strcmp(style, "stretch")) {
			scale_x = win_width / width;
			scale_y = win_height / height;
		} else if (!strcmp(style, "zoom")) {
			if ((win_width / win_height) >= (width / height)) {
				scale_x = scale_y = win_width / width;
			}
			else {
				scale_x = scale_y = win_height / height;
			}
		} else if (!strcmp(style, "center")) {
			off_x = (win_width - width) / 2;
			off_y = (win_height - height) / 2;
		}
	}
	cairo_translate(lualock.cr, off_x, off_y);
	cairo_scale(lualock.cr, scale_x, scale_y);
	cairo_surface_t *surface =
		cairo_image_surface_create_for_data(image->data, CAIRO_FORMAT_ARGB32,
											width, height,
											cairo_format_stride_for_width(
												CAIRO_FORMAT_ARGB32,
												width));
	cairo_set_source_surface(lualock.cr, surface, 0, 0);
	cairo_paint(lualock.cr);
	cairo_scale(lualock.cr, 1 / scale_x, 1 / scale_y);
	cairo_translate(lualock.cr, -off_x, -off_y);
	XClearWindow(lualock.dpy, lualock.win);
	return 0;
}

void lualock_lua_background_init(lua_State *L) {
	const struct luaL_reg lualock_background_lib[] =
	{
		{ "set", lualock_lua_background_set },
		{ NULL, NULL }
	};
	luaL_register(L, "background", lualock_background_lib);
}
