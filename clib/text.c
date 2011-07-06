#include <pango/pangocairo.h>
#include <string.h>
#include <unistd.h>

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
    
    *width = log_rect.width;
    *height = log_rect.height;
}

text_t* text_new(text_t *text_obj, const char *text, int x, int y,
                 const char *font, double r, double g, double b, double a) {
    
    int width, height;
    get_extents_for_string(text, font, &width, &height);
    layer_t *layer = create_layer(width, height);
    cairo_t *cr = cairo_create(layer->surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);

    *text_obj = (text_t) { .text = text, .x = x, .y = y, .font = font, .r = r,
                        .g = g, .b = b, .a = a, .layer = layer,
                        .layout = layout };
    add_layer(text_obj->layer);
    return text_obj;    
}

void text_draw(text_t *text_obj) {
    // Double buffering
    int width, height;
    get_extents_for_string(text_obj->text, text_obj->font, &width, &height);
    cairo_surface_t *surface = create_surface(width, height);
    cairo_t *cr = cairo_create(surface);
    PangoFontDescription *desc = pango_font_description_from_string(text_obj->font);
    pango_layout_set_font_description(text_obj->layout, desc);
    pango_font_description_free(desc);
    
    pango_layout_set_text(text_obj->layout, text_obj->text, -1);
    
    cairo_set_source_rgba(cr, text_obj->r, text_obj->g, text_obj->b, text_obj->a);
    pango_cairo_update_layout(cr, text_obj->layout);
    pango_cairo_show_layout(cr, text_obj->layout);
    cairo_destroy(cr);
    
    // Update the layer
    layer_t *new_layer = create_layer(width, height);
    update_layer(text_obj->layer, new_layer);
    text_obj->layer = new_layer;
    text_obj->layer->x = text_obj->x;
    text_obj->layer->y = text_obj->y;
    
    // Now we draw to the actual surface
    cairo_t *crl = cairo_create(text_obj->layer->surface);
    cairo_set_operator(crl, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(crl, surface, 0, 0);
    cairo_paint(crl);
    cairo_destroy(crl);
    cairo_surface_destroy(surface);
    
    lualock.need_updates = TRUE;
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
    luaL_getmetatable(L, "lualock.text");
    lua_setmetatable(L, -2);
    text_new(text_obj, text, x, y, font, r, g, b, a);
    return 1;
}

int lualock_lua_text_draw(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    text_draw(text_obj);
    return 0;
}

int lualock_lua_text_set(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    text_obj->text = luaL_checkstring(L, 2);
    lua_pushlightuserdata(L, text_obj);
    luaL_getmetatable(L, "lualock.text");
    lua_setmetatable(L, -2);
    return 1;
}

void lualock_lua_text_init(lua_State *L) {
    const struct luaL_reg lualock_text_lib[] =
    {
        { "new", lualock_lua_text_new },
        { "draw", lualock_lua_text_draw },
        { "set", lualock_lua_text_set },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, "lualock.text");
    luaL_register(L, "text", lualock_text_lib);
}

