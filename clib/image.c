#include <gdk/gdk.h>
#include <math.h>

#include "drawing.h"
#include "clib/image.h"

gboolean image_new(image_t *image, const char *filename) {
    GError **error = NULL;
    image->pbuf = gdk_pixbuf_new_from_file(filename, error);
    if (!image->pbuf)
        return FALSE;
    image->actor = create_actor(gdk_pixbuf_get_width(image->pbuf),
                                gdk_pixbuf_get_height(image->pbuf));
    
    image->rotation = 0;
    
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(image->actor));
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    gdk_cairo_set_source_pixbuf(cr, image->pbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    add_actor(image->actor);

    return TRUE;
}

void image_new_blank(image_t *image, gdouble rel_w, gdouble rel_h) {
    double w, h;
    get_abs_pos(rel_w, rel_h, &w, &h);
    image->actor = create_actor(w, h);
    image->rotation = 0;
    
    add_actor(image->actor);
}

gdouble image_get_width(image_t *image) {
    return clutter_actor_get_width(image->actor);
}

gdouble image_get_height(image_t *image) {
    return clutter_actor_get_height(image->actor);
}

void image_set_position(image_t *image, gdouble rel_x, gdouble rel_y) {
    gdouble x, y;
    get_abs_pos(rel_x, rel_y, &x, &y);
    clutter_actor_set_position(image->actor, x, y);
}

void image_show(image_t *image) {
    clutter_actor_show(image->actor);
}

void image_hide(image_t *image) {
    clutter_actor_hide(image->actor);
}

void image_rotate(image_t *image, gdouble angle, gfloat x, gfloat y) {
    image->rotation += angle;
    clutter_actor_set_anchor_point(image->actor, x, y);
    clutter_actor_set_rotation(image->actor, CLUTTER_Z_AXIS, image->rotation,
                               0, 0, 0);
}

void image_scale(image_t *image, double sx, double sy) {
    clutter_actor_set_scale(image->actor, sx, sy);
}

void image_resize(image_t *image, gdouble width, gdouble height) {
    image_scale(image, width / (gdouble) clutter_actor_get_width(image->actor),
                height / (gdouble) clutter_actor_get_height(image->actor));
}

void image_draw_rectangle(image_t *image, gdouble rel_x, gdouble rel_y,
                          gdouble rel_w, gdouble rel_h, gboolean fill,
                          ClutterColor *color) {
    gdouble x, y, w, h;
    get_abs_pos_for_dims(image_get_width(image), image_get_height(image),
                         rel_x, rel_y, &x, &y);
    get_abs_pos_for_dims(image_get_width(image), image_get_height(image),
                         rel_w, rel_h, &w, &h);
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(image->actor));
    clutter_cairo_set_source_color(cr, color);
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, x + 0.5, y + 0.5, w, h);
    cairo_stroke_preserve(cr);
    if (fill)
        cairo_fill(cr);
    
    cairo_destroy(cr);
}

static int lualock_lua_image_new(lua_State *L) {
    lua_settop(L, 2);
    image_t *image = lua_newuserdata(L, sizeof(image_t));
    luaL_getmetatable(L, "lualock.image");
    lua_setmetatable(L, -2);
    
    if (lua_isstring(L, 1)) {    
        const char *filename = luaL_checkstring(L, 1);
        gboolean loaded = image_new(image, filename);
        if (!loaded)
            return 0;
    } else if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        image_new_blank(image, lua_tonumber(L, 1), lua_tonumber(L, 2));
    }
    // keep the userdata referenced
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    
    return 1;
}

static int lualock_lua_image_show(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_show(image);
    return 0;
}

static int lualock_lua_image_hide(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_hide(image);
    return 0;
}

static int lualock_lua_image_set_position(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_set_position(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static int lualock_lua_image_rotate(lua_State *L) {
    lua_settop(L, 4);
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_rotate(image, luaL_checknumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
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

static int lualock_lua_image_draw_rectangle(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    ClutterColor *color;
    gdouble r, g, b, a;
    parse_color(luaL_optstring(L, 7, "#000000"), &r, &g, &b, &a);
    color = clutter_color_new(r * 255, g * 255, b * 255, a * 255);
    image_draw_rectangle(image, lua_tonumber(L, 2), lua_tonumber(L, 3),
                         lua_tonumber(L, 4), lua_tonumber(L, 5),
                         lua_toboolean(L, 6), color);
    return 0;
}

void lualock_lua_image_init(lua_State *L) {
    gdk_init(NULL, NULL);
    const struct luaL_reg lualock_image_lib[] =
    {
        { "show", lualock_lua_image_show },
        { "hide", lualock_lua_image_hide },
        { "set_position", lualock_lua_image_set_position },
        { "rotate", lualock_lua_image_rotate },
        { "scale", lualock_lua_image_scale },
        { "resize", lualock_lua_image_resize },
        { "width", lualock_lua_image_get_width },
        { "height", lualock_lua_image_get_height },
        { "draw_rectangle", lualock_lua_image_draw_rectangle },
        { NULL, NULL }
    };
    luaL_newmetatable(L, "lualock.image");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_register(L, NULL, lualock_image_lib);
    lua_register(L, "image", lualock_lua_image_new);
    lua_pop(L, 1);
}
    
