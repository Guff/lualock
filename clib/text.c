//      text.c - API for drawing text
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

#include <pango/pangocairo.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "misc.h"
#include "clib/text.h"

static void get_extents_for_string(const char *text, const char *font,
                                   int *width, int *height) {
    /* This is a horribly inelegant solution, but it was the first, and so far
     * the only I could think of. Because we want the text layer to only be as
     * big as it needs to, we need to get the extents of the text. But we can't
     * do that without actually creating a layout. Which needs a cairo surface.
     * So, create a tiny dummy surface, use that, then make the real layer 
     * later. */ 
    cairo_surface_t *surface = create_surface(1, 1);
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string(font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_set_text(layout, text, strlen(text));
    
    PangoRectangle log_rect;
    pango_layout_get_pixel_extents(layout, NULL, &log_rect);
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(layout);
    
    *width = log_rect.width;
    *height = log_rect.height;
}

text_t* text_new(text_t *text_obj, char *text, double rel_x, double rel_y,
                 char *font, char *font_color,
                 char *border_color, double border_width) {
    double x, y;
    get_abs_pos(rel_x, rel_y, &x, &y);
    int width, height;
    get_extents_for_string(text, font, &width, &height);
    layer_t *layer = create_layer(width + 2 * border_width,
                                       height + 2 * border_width);
    PangoLayout *layout = NULL;

    *text_obj = (text_t) { .text = text, .x = x, .y = y, .font = font,
                           .font_color = font_color, .layout = layout,
                           .border_width = border_width,
                           .border_color = border_color, .layer = layer };
    add_layer(text_obj->layer);
    return text_obj;    
}

void text_draw(text_t *text_obj) {
    double r, g, b, a;
    double border_r, border_g, border_b, border_a;
    parse_color(text_obj->font_color, &r, &g, &b, &a);
    parse_color(text_obj->border_color, &border_r, &border_g, &border_b, &border_a);

    int width, height;    
    get_extents_for_string(text_obj->text, text_obj->font, &width, &height);
    
    layer_t *layer = create_layer(width + text_obj->border_width,
                                  height + text_obj->border_width);
    layer_t *old_layer = text_obj->layer;
    layer->x = text_obj->x - text_obj->border_width;
    layer->y = text_obj->y - text_obj->border_width;
    text_obj->layer = layer;
    replace_layer(old_layer, layer);
    
    cairo_t *cr = cairo_create(layer->surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_translate(cr, text_obj->border_width, text_obj->border_width);
    text_obj->layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string(text_obj->font);
    pango_layout_set_font_description(text_obj->layout, desc);
    pango_font_description_free(desc);
    
    pango_layout_set_text(text_obj->layout, text_obj->text, -1);
    pango_cairo_update_layout(cr, text_obj->layout);
    pango_cairo_layout_path(cr, text_obj->layout);
    
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, text_obj->border_width);
    cairo_set_source_rgba(cr, border_r, border_g, border_b, border_a);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_fill(cr);
    cairo_destroy(cr);
    
    g_object_unref(text_obj->layout);
    
    layer->show = TRUE;
    register_update_for_layer(text_obj->layer);
}    

static int lualock_lua_text_new(lua_State *L) {
    if (!lua_istable(L, 1))
        luaL_typerror(L, 1, "table");
    lua_getfield(L, 1, "text");
    lua_getfield(L, 1, "x");
    lua_getfield(L, 1, "y");
    lua_getfield(L, 1, "font");
    lua_getfield(L, 1, "color");
    lua_getfield(L, 1, "border_color");
    lua_getfield(L, 1, "border_width");
    
    text_t *text_obj = lua_newuserdata(L, sizeof(text_t));
    // keep the userdata referenced
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, "lualock.text");
    lua_setmetatable(L, -2);
    text_new(text_obj,
             strdup(luaL_optstring(L, 2, "")), // text
             lua_tonumber(L, 3), // x
             lua_tonumber(L, 4), // y
             strdup(luaL_optstring(L, 5, "Sans Bold 12")), // font
             strdup(luaL_optstring(L, 6, "#000000")), // font_color
             strdup(luaL_optstring(L, 7, "#000000")), // border_color
             lua_tonumber(L, 8)); //border_width

    return 1;
}

static int lualock_lua_text_draw(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    text_draw(text_obj);
    return 0;
}
static int lualock_lua_text_hide(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    text_obj->layer->show = FALSE;
    return 0;
}

static int lualock_lua_text_set(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    if (!lua_istable(L, 2))
        luaL_typerror(L, 2, "table");
    lua_getfield(L, 2, "text");
    lua_getfield(L, 2, "x");
    lua_getfield(L, 2, "y");
    lua_getfield(L, 2, "font");
    lua_getfield(L, 2, "color");
    lua_getfield(L, 2, "border_color");
    lua_getfield(L, 2, "border_width");

    if (lua_isstring(L, 3)) {
        free(text_obj->text);
        text_obj->text = strdup(lua_tostring(L, 3));
    }
    if (lua_isnumber(L, 4))
        text_obj->x = lua_tonumber(L, 4);
    if (lua_isnumber(L, 5))
        text_obj->y = lua_tonumber(L, 5);
    if (lua_isstring(L, 6)) {
        free(text_obj->font);
        text_obj->font = strdup(lua_tostring(L, 6));
    }
    if (lua_tostring(L, 7)) {
        free(text_obj->font_color);
        text_obj->font_color = strdup(lua_tostring(L, 7));
    }
    if (lua_isstring(L, 8)) {
        free(text_obj->border_color);
        text_obj->border_color = strdup(lua_tostring(L, 8));
    }
    if (lua_isnumber(L, 9))
        text_obj->border_width = lua_tonumber(L, 9);
    return 0;
}

void lualock_lua_text_init(lua_State *L) {
    const struct luaL_reg lualock_text_lib[] =
    {
        { "draw", lualock_lua_text_draw },
        { "hide", lualock_lua_text_hide },
        { "set", lualock_lua_text_set },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, "lualock.text");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_register(L, NULL, lualock_text_lib);
    lua_pop(L, 1);
    lua_register(L, "text", lualock_lua_text_new);
}

