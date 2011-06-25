#include <lua.h>
#include <cairo-xlib.h>

typedef struct {
	lua_State *L;
	
	Display *dpy;
	int scr;
	Window win;
	Pixmap pmap;
	
	char *password;
	int pw_length;
	int pw_alloc;
	
	struct pam_handle *pam_handle;
	
	cairo_surface_t *surface;
	cairo_t *cr;
} lualock_t;

extern lualock_t lualock;

#define PW_BUFF_SIZE 32
