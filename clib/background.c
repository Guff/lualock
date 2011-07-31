//      background.c - API for setting the window's background
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
#include <cairo-xlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

#include "misc.h"
#include "clib/background.h"

void background_set_color(const gchar *hex) {
    cairo_t *cr = cairo_create(lualock.bg_surface);
    
    gdouble r, g, b, a;
    parse_color(hex, &r, &g, &b, &a);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    update_screen();
}

static gint lualock_lua_background_set(lua_State *L) {
    const gchar *filename = luaL_checkstring(L, 1);
    if (!strcmp(filename, "color")) {
        background_set_color(luaL_checkstring(L, 2));
        return 0;
    }
    GError **error = NULL;
    GdkPixbuf *pbuf = gdk_pixbuf_new_from_file(filename, error);
    
    const gchar *style = lua_tostring(L, 2);
    gdouble width, height, win_width, win_height;
    gint off_x = 0;
    gint off_y = 0;
    gdouble scale_x = 1;
    gdouble scale_y = 1;
    
    width = gdk_pixbuf_get_width(pbuf);
    height = gdk_pixbuf_get_height(pbuf);
    win_width = gdk_screen_get_width(lualock.scr);
    win_height = gdk_screen_get_height(lualock.scr);

    cairo_t *cr = cairo_create(lualock.bg_surface);

    if (style) {
        if (!strcmp(style, "stretch")) {
            scale_x = win_width / width;
            scale_y = win_height / height;
        } else if (!strcmp(style, "zoom")) {
            if ((win_width / win_height) >= (width / height)) {
                scale_x = scale_y = win_width / width;
            }
            else {
                scale_x = scale_y = win_height / height;
            }
        } else if (!strcmp(style, "center")) {
            off_x = (win_width - width) / 2;
            off_y = (win_height - height) / 2;
        }
    }
    cairo_translate(cr, off_x, off_y);
    cairo_scale(cr, scale_x, scale_y);
    gdk_cairo_set_source_pixbuf(cr, pbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    g_object_unref(pbuf);
    
    update_screen();
    
    return 0;
}

void lualock_lua_background_init(lua_State *L) {
    lua_register(L, "background", lualock_lua_background_set);
}
