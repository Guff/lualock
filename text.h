#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void draw_text(const char *text, int x, int y, const char *font,
			   double r, double g, double b, double a);

void lualock_lua_text_init(lua_State *L);
