#include <lua.h>
#include <cairo-xlib.h>

typedef struct {
    lua_State *L;
    
    Display *dpy;
    int scr;
    Window win;
    
    char *password;
    int pw_length;
    int pw_alloc;
    
    char *password_font;
    
    struct pam_handle *pam_handle;
    
    cairo_surface_t *surface;
    
    cairo_surface_t **surfaces;
    int surfaces_alloc;

    cairo_t *cr;
} lualock_t;

extern lualock_t lualock;

cairo_surface_t *create_surface();
void add_surface(cairo_surface_t *surface);
void remove_surface(cairo_surface_t *surface);
