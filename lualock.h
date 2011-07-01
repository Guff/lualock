#include <lua.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

typedef struct {
    char *font;
    
    int x;
    int y;
    
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
    Pixmap bg;
    
    char *password;
    int pw_length;
    int pw_alloc;
    
    char *password_font;
    
    struct pam_handle *pam_handle;
    
    cairo_surface_t *surface_buf;
    cairo_surface_t *surface;
    cairo_t *cr;
    
    cairo_surface_t *pw_surface;
    
    cairo_surface_t **surfaces;
    int surfaces_alloc;

    style_t style;
} lualock_t;

extern lualock_t lualock;

cairo_surface_t *create_surface();
void add_surface(cairo_surface_t *surface);
void remove_surface(cairo_surface_t *surface);
void parse_color(const char *hex, double *r, double *g, double *b, double *a);
