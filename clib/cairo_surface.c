#include <cairo.h>
#include <oocairo.h>

#include "lualock.h"
#include "misc.h"
#include "clib/cairo_surface.h"

static int lualock_lua_cairo_surface_create(lua_State *L) {
    lua_settop(L, 4);
    lualock_surface_t *lsurface = lua_newuserdata(L, sizeof(lualock_surface_t));
    luaL_getmetatable(L, "lualock.cairo_surface");
    lua_setmetatable(L, -2);
    
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    
    lsurface->layer = create_layer(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    lsurface->layer->x = luaL_checknumber(L, 1);
    lsurface->layer->y = luaL_checknumber(L, 2);
    
    add_layer(lsurface->layer);
    
    return 1;
}

static int lualock_lua_cairo_surface_show(lua_State *L) {
    lualock_surface_t *lsurface = luaL_checkudata(L, 1, "lualock.cairo_surface");
    lsurface->layer->show = TRUE;
    register_update_for_layer(lsurface->layer);
    return 0;
}

static int lualock_lua_cairo_surface_get_surface(lua_State *L) {
    lualock_surface_t *lsurface = luaL_checkudata(L, 1, "lualock.cairo_surface");
    oocairo_surface_push(L, lsurface->layer->surface);
    return 1;
}

static int lualock_lua_cairo_surface_set_position(lua_State *L) {
    lualock_surface_t *lsurface = luaL_checkudata(L, 1, "lualock.cairo_surface");
    lsurface->layer->x = luaL_checknumber(L, 2);
    lsurface->layer->y = luaL_checknumber(L, 3);
    return 0;
}

void lualock_lua_cairo_surface_init(lua_State *L) {
    const struct luaL_reg lualock_cairo_surface_lib[] =
    {
        { "show", lualock_lua_cairo_surface_show },
        { "get_surface", lualock_lua_cairo_surface_get_surface },
        { "set_position", lualock_lua_cairo_surface_set_position },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, "lualock.cairo_surface");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_register(L, NULL, lualock_cairo_surface_lib);
    lua_register(L, "cairo_surface", lualock_lua_cairo_surface_create);
}
