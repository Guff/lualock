#include <gtk/gtk.h>
#include <lualib.h>

typedef struct {
    guint val;
    GdkModifierType mod;
    
    gint r;
} keybind_t;

void lualock_lua_keybinder_init(lua_State *L);
