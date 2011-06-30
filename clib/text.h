#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
    const char *text;
    
    int x;
    int y;
    
    const char *font;
    
    double r;
    double g;
    double b;
    double a;
    
    cairo_surface_t *surface;
    PangoLayout *layout;
} text_t;

text_t text_new(const char *text, int x, int y, const char *font,
                double r, double g, double b, double a);

void lualock_lua_text_init(lua_State *L);
