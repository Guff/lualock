//      image.c - API for working with images
//      Copyright ©2011 Guff <cassmacguff@gmail.com>
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//

#include <gdk/gdk.h>
#include <math.h>
#include <oocairo.h>

#include "drawing.h"
#include "clib/image.h"

gboolean image_new(image_t *image, const char *filename) {
    GError **error = NULL;
    image->pbuf = gdk_pixbuf_new_from_file(filename, error);
    if (!image->pbuf)
        return FALSE;
    image->layer = create_layer(gdk_pixbuf_get_width(image->pbuf),
                                gdk_pixbuf_get_height(image->pbuf));
    
    image->rotation = 0;
    
    image->surface = create_surface(image->layer->width, image->layer->height);
    cairo_t *cr = cairo_create(image->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    gdk_cairo_set_source_pixbuf(cr, image->pbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    add_layer(image->layer);

    return TRUE;
}

void image_new_blank(image_t *image, gdouble rel_w, gdouble rel_h) {
    gdouble w, h;
    get_abs_pos(rel_w, rel_h, &w, &h);
    image->layer = create_layer(w, h);
    image->rotation = 0;
    image->surface = create_surface(w, h);
    image->pbuf = NULL;
    
    add_layer(image->layer);
}

void image_render(image_t *image) {
    cairo_t *cr = cairo_create(image->layer->surface);
    cairo_set_source_surface(cr, image->surface, 0, 0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
}

void image_clear(image_t *image) {
    cairo_t *cr = cairo_create(image->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    if (image->pbuf)
        gdk_cairo_set_source_pixbuf(cr, image->pbuf, 0, 0);
    else
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
}    

gdouble image_get_width(image_t *image) {
    return image->layer->width;
}

gdouble image_get_height(image_t *image) {
    return image->layer->height;
}

void image_set_position(image_t *image, gdouble rel_x, gdouble rel_y) {
    gdouble x, y;
    get_abs_pos(rel_x, rel_y, &x, &y);
    image->layer->x = x;
    image->layer->y = y;
}

void image_show(image_t *image) {
    image_render(image);
    image->layer->show = TRUE;
    register_update_for_layer(image->layer);
}

void image_hide(image_t *image) {
    image->layer->show = FALSE;
}

void image_rotate(image_t *image, gdouble angle, gdouble x, gdouble y) {
    image->rotation += angle;
    image->layer->angle = image->rotation;
}

void image_scale(image_t *image, gdouble sx, gdouble sy) {
    image->layer->scale_x = sx;
    image->layer->scale_y = sy;
}

void image_resize(image_t *image, gdouble width, gdouble height) {
    image_scale(image, width / (gdouble) image->layer->width,
                height / (gdouble) image->layer->height);
}

void image_draw_rectangle(image_t *image, gdouble x, gdouble y,
                          gdouble w, gdouble h, gboolean fill,
                          GdkRGBA *color) {
    cairo_t *cr = cairo_create(image->surface);
    gdk_cairo_set_source_rgba(cr, color);
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, x, y, w, h);
    if (fill)
        cairo_fill(cr);
    else
        cairo_stroke(cr);
    
    cairo_destroy(cr);
}

void image_draw_line(image_t *image, gdouble x1, gdouble y1,
                     gdouble x2, gdouble y2, gdouble width,
                     GdkRGBA *color) {
    cairo_t *cr = cairo_create(image->surface);
    gdk_cairo_set_source_rgba(cr, color);
    cairo_set_line_width(cr, width);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
    
    cairo_destroy(cr);
}

void image_draw_circle(image_t *image, gdouble x, gdouble y,
                       gdouble radius, gboolean fill, GdkRGBA *color) {
    cairo_t *cr = cairo_create(image->surface);
    gdk_cairo_set_source_rgba(cr, color);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_set_line_width(cr, 1);
    if (fill)
        cairo_fill(cr);
    else
        cairo_stroke(cr);
    
    cairo_destroy(cr);
}

static gint lualock_lua_image_new(lua_State *L) {
    lua_settop(L, 2);
    image_t *image = lua_newuserdata(L, sizeof(image_t));
    luaL_getmetatable(L, "lualock.image");
    lua_setmetatable(L, -2);
    
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        image_new_blank(image, lua_tonumber(L, 1), lua_tonumber(L, 2));
    } else if (lua_isstring(L, 1)) {   
        const gchar *filename = luaL_checkstring(L, 1);
        gboolean loaded = image_new(image, filename);
        if (!loaded)
            return 0;
    }
    // keep the userdata referenced
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    
    return 1;
}

static gint lualock_lua_image_show(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_show(image);
    return 0;
}

static gint lualock_lua_image_hide(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_hide(image);
    return 0;
}

static gint lualock_lua_image_set_position(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_set_position(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static gint lualock_lua_image_rotate(lua_State *L) {
    lua_settop(L, 4);
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_rotate(image, luaL_checknumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}

static gint lualock_lua_image_scale(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_scale(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static gint lualock_lua_image_resize(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_resize(image, lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}

static gint lualock_lua_image_get_width(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    lua_pushinteger(L, image_get_width(image));
    return 1;
}

static gint lualock_lua_image_get_height(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    lua_pushinteger(L, image_get_height(image));
    return 1;
}

static gint lualock_lua_image_clear(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    image_clear(image);
    return 0;
}

static gint lualock_lua_image_get_surface(lua_State *L) {
    image_t *image = luaL_checkudata(L, 1, "lualock.image");
    oocairo_surface_push(L, image->surface);
    return 1;
}

void lualock_lua_image_init(lua_State *L) {
    const struct luaL_Reg lualock_image_lib[] =
    {
        { "show", lualock_lua_image_show },
        { "hide", lualock_lua_image_hide },
        { "set_position", lualock_lua_image_set_position },
        { "rotate", lualock_lua_image_rotate },
        { "scale", lualock_lua_image_scale },
        { "resize", lualock_lua_image_resize },
        { "width", lualock_lua_image_get_width },
        { "height", lualock_lua_image_get_height },
        { "clear", lualock_lua_image_clear },
        { "get_surface", lualock_lua_image_get_surface },
        { NULL, NULL }
    };
    luaL_newmetatable(L, "lualock.image");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_setfuncs(L, lualock_image_lib, 0);
    lua_register(L, "image", lualock_lua_image_new);
    lua_pop(L, 1);
}
    
