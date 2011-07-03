#include <string.h>

#include "lualock.h"
#include "misc.h"
#include "clib/style.h"

void style_set(const char *font, int x, int y, int off_x, int off_y, int width,
			   int height, double r, double g, double b, double a) {
	if (font)
		lualock.style.font = strdup(font);
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
	
	cairo_surface_finish(lualock.pw_surface);
	cairo_surface_destroy(lualock.pw_surface);
	lualock.pw_surface = create_surface();
}

int lualock_lua_style_set(lua_State *L) {
	double r, g, b, a;
	lua_getfield(L, 1, "color");
	parse_color(luaL_checkstring(L, 2), &r, &g, &b, &a);
	lua_pop(L, 1);
	lua_getfield(L, 1, "font");
	lua_getfield(L, 1, "x");
	lua_getfield(L, 1, "y");
	lua_getfield(L, 1, "off_x");
	lua_getfield(L, 1, "off_y");
	lua_getfield(L, 1, "width");
	lua_getfield(L, 1, "height");
	style_set(lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
			  lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7),
			  lua_tonumber(L, 8), r, g, b, a);
	return 0;
}

void lualock_lua_style_init(lua_State *L) {
    const struct luaL_reg lualock_style_lib[] =
    {
        { "set", lualock_lua_style_set },
        { NULL, NULL }
    };
    luaL_register(L, "style", lualock_style_lib);
}
