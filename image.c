#include <string.h>
#include <lualib.h>
#include <lauxlib.h>
#include <Imlib2.h>
#include <svg-cairo.h>
#include <cairo-svg.h>
#include <stdbool.h>

#include "globalconf.h"

typedef struct {
	Imlib_Image image;
	
	unsigned char *data;
} image_t;

int image_get_width(image_t *image) {
	imlib_context_set_image(image->image);
	return imlib_image_get_width();
}

int image_get_height(image_t *image) {
	imlib_context_set_image(image->image);
	return imlib_image_get_height();
}

bool image_new(const char *filename, image_t *image) {
	image->image = imlib_load_image(filename);
	// if loading the image didn't work, maybe it's an svg
	if (!image->image) {
		svg_cairo_t *svg_image = NULL;
		unsigned int width, height;
		svg_cairo_create(&svg_image);
		if (svg_cairo_parse(svg_image, filename) != SVG_CAIRO_STATUS_SUCCESS)
			return false;
		svg_cairo_get_size(svg_image, &width, &height);
		
		cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
															  width, height);
		cairo_t *cr = cairo_create(surface);
		svg_cairo_render(svg_image, cr);
		image->data = cairo_image_surface_get_data(surface);
		image->image = imlib_create_image_using_copied_data(width, height, (DATA32 *) image->data);
		imlib_context_set_image(image->image);
	} else {
		imlib_context_set_image(image->image);
		image->data = (unsigned char *) imlib_image_get_data_for_reading_only();
	}
	return true;
}

static int lualock_lua_image_new(lua_State *L) {
	image_t *image = lua_newuserdata(L, sizeof(image_t));
	
	const char *filename = luaL_checkstring(L, 1);
	bool loaded = image_new(filename, image);
	
	return loaded;
}

static int lualock_lua_image_set_background(lua_State *L) {
	image_t *image = lua_touserdata(L, 1);
	
	// if not userdata, maybe the user wants a solid color
	if (!image) {
		const char *hex = luaL_checkstring(L, 1);
		unsigned long packed_rgb;
		sscanf(hex + 1, "%lx", &packed_rgb);
		unsigned long r = packed_rgb >> 16;
		unsigned long g = packed_rgb >> 8 & 0xff;
		unsigned long b = packed_rgb & 0xff;
		
		cairo_set_source_rgb(lualock.cr, r, g, b);
		cairo_paint(lualock.cr);
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
	XClearWindow(lualock.dpy, lualock.win);
	return 0;
}

void lualock_lua_image_init(lua_State *L) {
	const struct luaL_reg lualock_image_lib[] =
	{
		{ "new", lualock_lua_image_new },
		{ "set_background", lualock_lua_image_set_background },
		{ NULL, NULL }
	};
	luaL_register(L, "image", lualock_image_lib);
}
	
