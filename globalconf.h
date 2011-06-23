#include <lua.h>
#include <cairo-xlib.h>

typedef struct {
	lua_State *L;
	
	Display *dpy;
	int scr;
	Window win;
	Pixmap pmap;
	
	cairo_surface_t *surface;
	cairo_t *cr;
} lualock_t;

extern lualock_t lualock;
