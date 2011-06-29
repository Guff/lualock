#include <pango/pangocairo.h>
#include <string.h>
#include <unistd.h>

#include "lualock.h"
#include "text.h"

void draw_text(const char *text, int x, int y, const char *font,
			   double r, double g, double b, double a) {
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		cairo_xlib_surface_get_width(lualock.surface),
		cairo_xlib_surface_get_height(lualock.surface));
	cairo_t *cr = cairo_create(surface);
	
	add_surface(surface);
	
	PangoLayout *layout;
	PangoFontDescription *desc;
	
	cairo_translate(cr, x, y);
	layout = pango_cairo_create_layout(cr);
	
	pango_layout_set_text(layout, text, strlen(text));
	desc = pango_font_description_from_string(font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	
	cairo_set_source_rgba(cr, r, g, b, a);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);
	
	g_object_unref(layout);
}

int lualock_lua_text_draw(lua_State *L) {
	const char *text = luaL_checkstring(L, 1);
	int x = lua_tonumber(L, 2);
	int y = lua_tonumber(L, 3);
	const char *font = lua_tostring(L, 4);
	const char *hex = lua_tostring(L, 5);
	
	if (!font)
		font = "Sans Bold 12";
	if (!hex)
		hex = "#000000";
	
	unsigned long packed_rgb;
	// hex + 1 skips over the pound sign, which we don't need
	sscanf(hex + 1, "%lx", &packed_rgb);
	double r = (packed_rgb >> 16) / 256.0;
	double g = (packed_rgb >> 8 & 0xff) / 256.0;
	double b = (packed_rgb & 0xff) / 256.0;
	
	draw_text(text, x, y, font, r, g, b, 1);
	return 0;
}

void lualock_lua_text_init(lua_State *L) {
	const struct luaL_reg lualock_text_lib[] =
	{
		{ "draw", lualock_lua_text_draw },
		{ NULL, NULL }
	};
	luaL_register(L, "text", lualock_text_lib);
}

