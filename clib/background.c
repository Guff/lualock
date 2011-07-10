#include <string.h>
#include <cairo-xlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

#include "misc.h"
#include "clib/background.h"

void background_set_color(const char *hex) {
    ClutterActor *actor = create_actor(0, 0);
    add_actor(actor);
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(actor));
    
    double r, g, b, a;
    parse_color(hex, &r, &g, &b, &a);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_paint(cr);
    cairo_destroy(cr);
}

static int lualock_lua_background_set(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    if (!strcmp(filename, "color")) {
        background_set_color(luaL_checkstring(L, 2));
        return 0;
    }
    GError **error = NULL;
    GdkPixbuf *pbuf = gdk_pixbuf_new_from_file(filename, error);
    
    const char *style = lua_tostring(L, 2);
    double width, height, win_width, win_height;
    int off_x = 0;
    int off_y = 0;
    double scale_x = 1;
    double scale_y = 1;
    
    width = gdk_pixbuf_get_width(pbuf);
    height = gdk_pixbuf_get_height(pbuf);
    win_width = gdk_screen_get_width(lualock.scr);
    win_height = gdk_screen_get_height(lualock.scr);

    ClutterActor *actor = create_actor(0, 0);
    add_actor(actor);
    
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(actor));

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
    
    return 0;
}

void lualock_lua_background_init(lua_State *L) {
    lua_register(L, "background", lualock_lua_background_set);
}
