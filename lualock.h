#include <lua.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

typedef struct {
    char *font;
    
    int x;
    int y;
    
    int off_x;
    int off_y;
    
    int width;
    int height;
    
    double r;
    double g;
    double b;
    double a;
} style_t;
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
    
    cairo_surface_t *surface_buf;
    cairo_surface_t *surface;
    cairo_t *cr;
    
    cairo_surface_t *pw_surface;
    cairo_t *pw_cr;
    
    cairo_surface_t **surfaces;
    int surfaces_alloc;

    style_t style;
} lualock_t;

extern lualock_t lualock;

char* get_password_mask();
