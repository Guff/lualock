#ifndef CLIB_IMAGE_H
#define CLIB_IMAGE_H

#include <stdbool.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

#include "misc.h"

typedef struct {
    GdkPixbuf *pbuf;
    layer_t *layer;
    cairo_t *cr;
    
    int width;
    int height;
    double angle;
} image_t;

int image_get_width(image_t *image);
int image_get_height(image_t *image);

bool image_new(const char *filename, image_t *image);

void lualock_lua_image_init(lua_State *L);

#endif
