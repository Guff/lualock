#include <string.h>

#include "lualock.h"
#include "misc.h"
#include "clib/style.h"

void style_set(const char *font, int x, int y, int off_x, int off_y, int width,
			   int height, double r, double g, double b, double a) {
	if (font)
		lualock.style.font = font;
	lualock.style.x = x;
	lualock.style.y = y;
	lualock.style.off_x = off_x;
	lualock.style.off_y = off_y;
	lualock.style.width = width;
	lualock.style.height = height;
	lualock.style.r = r;
	lualock.style.g = g;
	lualock.style.b = b;
	lualock.style.a = a;
	
	clutter_actor_set_size(lualock.pw_actor, lualock.style.width, lualock.style.height);
	clutter_cairo_texture_set_surface_size(CLUTTER_CAIRO_TEXTURE(lualock.pw_actor),
										   lualock.style.width, lualock.style.height);
	clutter_actor_set_position(lualock.pw_actor, lualock.style.x, lualock.style.y);
}

int lualock_lua_style_set(lua_State *L) {
	double r, g, b, a;
	lua_getfield(L, 1, "color");
	parse_color(luaL_optstring(L, 2, "#000000"), &r, &g, &b, &a);
	lua_pop(L, 1);
	lua_getfield(L, 1, "font");
	lua_getfield(L, 1, "x");
	lua_getfield(L, 1, "y");
	lua_getfield(L, 1, "off_x");
	lua_getfield(L, 1, "off_y");
	lua_getfield(L, 1, "width");
	lua_getfield(L, 1, "height");
	
	style_set(luaL_optstring(L, 2, lualock.style.font),
			  luaL_optnumber(L, 3, lualock.style.x),
			  luaL_optnumber(L, 4, lualock.style.y),
			  luaL_optnumber(L, 5, lualock.style.off_x),
			  luaL_optnumber(L, 6, lualock.style.off_y),
			  luaL_optnumber(L, 7, lualock.style.width),
			  luaL_optnumber(L, 8, lualock.style.height),
			  r, g, b, a);
	return 0;
}

void lualock_lua_style_init(lua_State *L) {
    lua_register(L, "style", lualock_lua_style_set);
}
