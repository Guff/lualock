#include <pango/pangocairo.h>
#include <string.h>
#include <unistd.h>

#include "lualock.h"
#include "text.h"

text_t text_new(const char *text, int x, int y, const char *font,
                double r, double g, double b, double a) {
    cairo_surface_t *surface = create_surface();
    cairo_t *cr = cairo_create(surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string(font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_set_text(layout, text, strlen(text));

    text_t text_obj = { .text = text, .x = x, .y = y, .font = font, .r = r,
                        .g = g, .b = b, .a = a, .surface = surface,
                        .layout = layout };
    return text_obj;
    
}

void text_draw(text_t text_obj) {
    cairo_t *cr = cairo_create(text_obj.surface);
    
    add_surface(text_obj.surface);
    
    cairo_translate(cr, text_obj.x, text_obj.y);
    
    cairo_set_source_rgba(cr, text_obj.r, text_obj.g, text_obj.b, text_obj.a);
    pango_cairo_update_layout(cr, text_obj.layout);
    pango_cairo_show_layout(cr, text_obj.layout);
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
    
    unsigned long packed_rgb;
    // hex + 1 skips over the pound sign, which we don't need
    sscanf(hex + 1, "%lx", &packed_rgb);
    double r = (packed_rgb >> 16) / 256.0;
    double g = (packed_rgb >> 8 & 0xff) / 256.0;
    double b = (packed_rgb & 0xff) / 256.0;
    
    text_t *text_obj = lua_newuserdata(L, sizeof(text_t));
    *text_obj = text_new(text, x, y, font, r, g, b, 1);
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
