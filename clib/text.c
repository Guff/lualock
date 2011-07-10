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

text_t* text_new(text_t *text_obj, const char *text, int x, int y,
                 const char *font, const char *font_color,
                 const char *border_color, double border_width) {
    
    int width, height;
    get_extents_for_string(text, font, &width, &height);
    layer_t *layer = create_layer(width, height);
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

    // Double buffering
    int width, height;    
    get_extents_for_string(text_obj->text, text_obj->font, &width, &height);
    cairo_surface_t *surface = create_surface(width + 2 * text_obj->border_width,
                                              height + 2 * text_obj->border_width);
    cairo_t *cr = cairo_create(surface);
    cairo_translate(cr, text_obj->border_width, text_obj->border_width);
    text_obj-> layout = pango_cairo_create_layout(cr);
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
    
    // Update the layer
    layer_t *new_layer = create_layer(width + 2 * text_obj->border_width,
                                      height + 2 * text_obj->border_width);
    cairo_surface_destroy(text_obj->layer->surface);
    free(text_obj->layer);
    update_layer(text_obj->layer, new_layer);
    text_obj->layer = new_layer;
    text_obj->layer->x = text_obj->x - text_obj->border_width;
    text_obj->layer->y = text_obj->y - text_obj->border_width;
    
    // Now we draw to the actual surface
    cairo_t *crl = cairo_create(text_obj->layer->surface);
    cairo_set_operator(crl, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(crl, surface, 0, 0);
    cairo_paint(crl);
    cairo_destroy(crl);
    cairo_surface_destroy(surface);
}

int lualock_lua_text_new(lua_State *L) {
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
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, "lualock.text");
    lua_setmetatable(L, -2);
    text_new(text_obj, luaL_optstring(L, 2, ""), lua_tonumber(L, 3),
             lua_tonumber(L,4), luaL_optstring(L, 5, "Sans Bold 12"),
             luaL_optstring(L, 6, "#000000"), luaL_optstring(L, 7, "#000000"),
             lua_tonumber(L, 8));

    return 1;
}

int lualock_lua_text_draw(lua_State *L) {
    text_t *text_obj = luaL_checkudata(L, 1, "lualock.text");
    text_draw(text_obj);
    return 0;
}

int lualock_lua_text_set(lua_State *L) {
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
    if (lua_isstring(L, 3))
        text_obj->text = lua_tostring(L, 3);
    if (lua_isnumber(L, 4))
        text_obj->x = lua_tonumber(L, 4);
    if (lua_isnumber(L, 5))
        text_obj->y = lua_tonumber(L, 5);
    if (lua_isstring(L, 6))
        text_obj->font = lua_tostring(L, 6);
    if (lua_tostring(L, 7))
        text_obj->font_color = lua_tostring(L, 7);
    if (lua_isstring(L, 8))
        text_obj->border_color = lua_tostring(L, 8);
    if (lua_isnumber(L, 9))
        text_obj->border_width = lua_tonumber(L, 9);
    return 0;
}

void lualock_lua_text_init(lua_State *L) {
    const struct luaL_reg lualock_text_lib[] =
    {
        { "draw", lualock_lua_text_draw },
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

