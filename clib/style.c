//      style.c - API for setting the style of the password field
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

#include <string.h>

#include "lualock.h"
#include "misc.h"
#include "clib/style.h"

void style_set(const gchar *font, gint x, gint y, gint off_x, gint off_y,
               gint width, gint height, gdouble r, gdouble g, gdouble b,
               gdouble a) {
    if (font) {
        pango_font_description_free(lualock.style.font_desc);
        lualock.style.font_desc = pango_font_description_from_string(font);
    }
    
    lualock.style.x = x;
    lualock.style.y = y;
    lualock.style.off_x = off_x;
    lualock.style.off_y = off_y;
    lualock.style.width = width;
    lualock.style.height = height;
    lualock.style.r = r;
    lualock.style.g = g;
    lualock.style.b = b;
    lualock.style.a = a;
    
    cairo_surface_t *old_pw_surface = lualock.pw_surface;
    lualock.pw_surface = create_surface(width, height);
    cairo_surface_destroy(old_pw_surface);
}

int lualock_lua_style_set(lua_State *L) {
    gdouble r, g, b, a;
    lua_getfield(L, 1, "color");
    parse_color(luaL_optstring(L, 2, "#000000"), &r, &g, &b, &a);
    lua_pop(L, 1);
    lua_getfield(L, 1, "font");
    lua_getfield(L, 1, "x");
    lua_getfield(L, 1, "y");
    lua_getfield(L, 1, "off_x");
    lua_getfield(L, 1, "off_y");
    lua_getfield(L, 1, "width");
    lua_getfield(L, 1, "height");
    
    style_set(luaL_optstring(L, 2, DEFAULT_FONT),
              luaL_optnumber(L, 3, lualock.style.x),
              luaL_optnumber(L, 4, lualock.style.y),
              luaL_optnumber(L, 5, lualock.style.off_x),
              luaL_optnumber(L, 6, lualock.style.off_y),
              luaL_optnumber(L, 7, lualock.style.width),
              luaL_optnumber(L, 8, lualock.style.height),
              r, g, b, a);
    return 0;
}

void lualock_lua_style_init(lua_State *L) {
    lua_register(L, "style", lualock_lua_style_set);
}
