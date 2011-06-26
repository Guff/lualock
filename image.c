#include <lualib.h>
#include <lauxlib.h>
#include <svg-cairo.h>
#include <cairo-svg.h>

#include "globalconf.h"
#include "image.h"

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

void lualock_lua_image_init(lua_State *L) {
	const struct luaL_reg lualock_image_lib[] =
	{
		{ "new", lualock_lua_image_new },
		{ NULL, NULL }
	};
	luaL_register(L, "image", lualock_image_lib);
}
	
