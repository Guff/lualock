#include <lualib.h>

typedef struct {
    layer_t *layer;
} lualock_surface_t;

void lualock_lua_cairo_surface_init(lua_State *L);
