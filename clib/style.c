#include "lualock.h"
#include "style.h"

void style_set(char *font, int x, int y, int width, int height, double r,
			   double g, double b, double a) {
	lualock.style.font = font;
	lualock.style.x = x;
	lualock.style.y = y;
	lualock.style.width = width;
	lualock.style.height = height;
	lualock.style.r = r;
	lualock.style.g = g;
	lualock.style.b = b;
	lualock.style.a = a;
}

int lualock_lua_style_set(lua_State *L) {
	
	return 0;
}
