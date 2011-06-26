#include <lualib.h>
#include <lauxlib.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

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

void image_render(image_t *image, int x, int y) {
	cairo_surface_t *surface = cairo_image_surface_create_for_data(image->data,
		CAIRO_FORMAT_ARGB32, image_get_width(image), image_get_height(image),
		cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, image_get_width(image)));
	cairo_translate(lualock.cr, x, y);
	cairo_set_source_surface(lualock.cr, surface, 0, 0);
	cairo_paint(lualock.cr);
}

void image_resize(image_t *image, int w, int h) {
	imlib_context_set_image(image->image);
	image->image = imlib_create_cropped_scaled_image(0, 0,
													 image_get_width(image),
													 image_get_height(image),
													 w, h);
}

bool image_new(const char *filename, image_t *image) {
	GError **error = NULL;
	image->image = imlib_load_image(filename);
	// if loading the image didn't work, maybe it's an svg
	if (!image->image) {
		RsvgDimensionData dims;
		unsigned int width, height;

		rsvg_init();
		RsvgHandle *rsvg_handle = rsvg_handle_new_from_file(filename, error);
		rsvg_handle_get_dimensions(rsvg_handle, &dims);
		width = dims.width;
		height = dims.height;
		cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
															  width, height);
		cairo_t *cr = cairo_create(surface);
		rsvg_handle_render_cairo(rsvg_handle, cr);
		image->data = cairo_image_surface_get_data(surface);
		image->image = imlib_create_image_using_copied_data(width, height, (DATA32 *) image->data);
		imlib_context_set_image(image->image);
		cairo_destroy(cr);
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

static int lualock_lua_image_render(lua_State *L) {
	image_t *image = lua_touserdata(L, 1);
	/* Neat thing is, lua_tointeger returns 0 if it fails. So if the user
	 * doesn't supply these two arguments, two nils are on the stack, which
	 * means two zeros are returned. So, the image is drawn at (0, 0), a
	 * convenient default. */
	image_render(image, lua_tointeger(L, 2), lua_tointeger(L, 3));
	return 0;
}

void lualock_lua_image_init(lua_State *L) {
	const struct luaL_reg lualock_image_lib[] =
	{
		{ "new", lualock_lua_image_new },
		{ "render", lualock_lua_image_render },
		{ NULL, NULL }
	};
	luaL_register(L, "image", lualock_image_lib);
}
	
