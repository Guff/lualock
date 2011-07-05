#include <pango/pangocairo.h>
#include <string.h>
#include <unistd.h>

#include "misc.h"
#include "clib/text.h"

text_t text_new(const char *text, int x, int y, const char *font,
                double r, double g, double b, double a) {
    layer_t *layer = create_layer(gdk_screen_get_width(lualock.scr),
                                 gdk_screen_get_height(lualock.scr));
    cairo_t *cr = cairo_create(layer->surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string(font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_set_text(layout, text, strlen(text));

    text_t text_obj = { .text = text, .x = x, .y = y, .font = font, .r = r,
                        .g = g, .b = b, .a = a, .layer = layer,
                        .layout = layout };
    cairo_destroy(cr);
    return text_obj;    
}

void text_draw(text_t text_obj) {
    cairo_t *cr = cairo_create(text_obj.layer->surface);
    
    add_layer(text_obj.layer);
    
    cairo_translate(cr, text_obj.x, text_obj.y);
    
    cairo_set_source_rgba(cr, text_obj.r, text_obj.g, text_obj.b, text_obj.a);
    pango_cairo_update_layout(cr, text_obj.layout);
    pango_cairo_show_layout(cr, text_obj.layout);
    cairo_destroy(cr);
}

int lualock_lua_text_new(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    const char *font = lua_tostring(L, 4);
    const char *hex = lua_tostring(L, 5);
    
    if (!font)
        font = "Sans Bold 12";
    if (!hex)
        hex = "#000000";
    
    double r, g, b, a;
    parse_color(hex, &r, &g, &b, &a);
    
    text_t *text_obj = lua_newuserdata(L, sizeof(text_t));
    *text_obj = text_new(text, x, y, font, r, g, b, a);
    return 1;
}

int lualock_lua_text_draw(lua_State *L) {
    text_t *text_obj = lua_touserdata(L, 1);
    text_draw(*text_obj);
    return 0;
}

void lualock_lua_text_init(lua_State *L) {
    const struct luaL_reg lualock_text_lib[] =
    {
        { "new", lualock_lua_text_new },
        { "draw", lualock_lua_text_draw },
        { NULL, NULL }
    };
    luaL_register(L, "text", lualock_text_lib);
}

