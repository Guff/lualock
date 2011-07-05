#include <lualib.h>
#include <lauxlib.h>
#include <librsvg/rsvg.h>
#include <gdk/gdk.h>
#include <math.h>

#include "drawing.h"
#include "clib/image.h"

int image_get_width(image_t *image) {
    return image->width;
}

int image_get_height(image_t *image) {
    return image->height;
}

void image_render(image_t *image, int x, int y) {
    cairo_surface_t *surface = create_surface(image->width, image->height);
    cairo_t *cr = cairo_create(surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    gdk_cairo_set_source_pixbuf(cr, image->pbuf, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    cairo_t *crb = cairo_create(image->layer->surface);
    cairo_set_source_surface(crb, surface, 0, 0);
    cairo_set_operator(crb, CAIRO_OPERATOR_SOURCE);
    cairo_paint(crb);
    cairo_destroy(crb);
    cairo_surface_destroy(surface);
    image->layer->x = x;
    image->layer->y = y;
}

void image_rotate(image_t *image, double angle) {
    image->layer->angle += 2 * M_PI * angle / 360;
}

bool image_new(const char *filename, image_t *image) {
    GError **error = NULL;
    image->pbuf = gdk_pixbuf_new_from_file(filename, error);
    // if loading the image didn't work, maybe it's an svg
    if (!(image->pbuf)) {
        image->pbuf = rsvg_pixbuf_from_file(filename, error);
    }
    
    image->width = gdk_pixbuf_get_width(image->pbuf);
    image->height = gdk_pixbuf_get_height(image->pbuf);
    image->layer = create_layer(image->width, image->height);

    add_layer(image->layer);

    return true;
}

static int lualock_lua_image_new(lua_State *L) {
    image_t *image = lua_newuserdata(L, sizeof(image_t));
    
    const char *filename = luaL_checkstring(L, 1);
    bool loaded = image_new(filename, image);
    
    return loaded;
}

static int lualock_lua_image_render(lua_State *L) {
    image_t *image = lua_touserdata(L, 1);
    image_render(image, lua_tointeger(L, 2), lua_tointeger(L, 3));
    return 0;
}

static int lualock_lua_image_rotate(lua_State *L) {
    image_t *image = lua_touserdata(L, 1);
    image_rotate(image, lua_tonumber(L, 2));
    return 0;
}

void lualock_lua_image_init(lua_State *L) {
    gdk_init(NULL, NULL);
    const struct luaL_reg lualock_image_lib[] =
    {
        { "new", lualock_lua_image_new },
        { "render", lualock_lua_image_render },
        { "rotate", lualock_lua_image_rotate },
        { NULL, NULL }
    };
    luaL_register(L, "image", lualock_image_lib);
}
    
