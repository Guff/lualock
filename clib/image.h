#ifndef CLIB_IMAGE_H
#define CLIB_IMAGE_H

#include <stdbool.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <lualib.h>
#include <lauxlib.h>

#include "misc.h"

typedef struct {
    GdkPixbuf *pbuf;
    ClutterActor *actor;
} image_t;

int image_get_width(image_t *image);
int image_get_height(image_t *image);

bool image_new(const char *filename, image_t *image);

void lualock_lua_image_init(lua_State *L);

#endif
