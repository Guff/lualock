#include <gdk/gdk.h>
#include <math.h>

#include "drawing.h"
#include "clib/image.h"

int image_get_width(image_t *image) {
    return clutter_actor_get_width(image->actor);
}

int image_get_height(image_t *image) {
    return clutter_actor_get_height(image->actor);
}

void image_render(image_t *image, int x, int y) {
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(image->actor));
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    gdk_cairo_set_source_pixbuf(cr, image->pbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    clutter_actor_set_position(image->actor, x, y);
}

void image_rotate(image_t *image, double angle) {
    image->rotation += angle;
    clutter_actor_set_z_rotation_from_gravity(image->actor, image->rotation, CLUTTER_GRAVITY_CENTER);
}

void image_scale(image_t *image, double sx, double sy) {
    clutter_actor_set_scale(image->actor, sx, sy);
}

void image_resize(image_t *image, int width, int height) {
    image_scale(image, width / (float) clutter_actor_get_width(image->actor),
                height / (float) clutter_actor_get_height(image->actor));
}

bool image_new(const char *filename, image_t *image) {
    GError **error = NULL;
    image->pbuf = gdk_pixbuf_new_from_file(filename, error);
    
    image->actor = create_actor(gdk_pixbuf_get_width(image->pbuf),
                                gdk_pixbuf_get_height(image->pbuf));
    
    image->rotation = 0;
    
    add_actor(image->actor);

    return true;
}

static int lualock_lua_image_new(lua_State *L) {
    image_t *image = lua_newuserdata(L, sizeof(image_t));
    luaL_getmetatable(L, "lualock.image");
    lua_setmetatable(L, -2);
    
    const char *filename = luaL_checkstring(L, 1);
    bool loaded = image_new(filename, image);
    
    // keep the userdata referenced
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    
    return loaded;
}

static int lualock_lua_image_render(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_render(image, lua_tointeger(L, 2), lua_tointeger(L, 3));
    return 0;
}

static int lualock_lua_image_rotate(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_rotate(image, lua_tonumber(L, 2));
    return 0;
}

static int lualock_lua_image_scale(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_scale(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static int lualock_lua_image_resize(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_resize(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static int lualock_lua_image_get_width(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    lua_pushinteger(L, image_get_width(image));
    return 1;
}

static int lualock_lua_image_get_height(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    lua_pushinteger(L, image_get_height(image));
    return 1;
}

void lualock_lua_image_init(lua_State *L) {
    gdk_init(NULL, NULL);
    const struct luaL_reg lualock_image_lib[] =
    {
        { "render", lualock_lua_image_render },
        { "rotate", lualock_lua_image_rotate },
        { "scale", lualock_lua_image_scale },
        { "resize", lualock_lua_image_resize },
        { "get_width", lualock_lua_image_get_width },
        { "get_height", lualock_lua_image_get_height },
        { NULL, NULL }
    };
    luaL_newmetatable(L, "lualock.image");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_register(L, NULL, lualock_image_lib);
    lua_register(L, "image", lualock_lua_image_new);
    lua_pop(L, 1);
}
    
