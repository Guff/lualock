#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "misc.h"

cairo_surface_t* create_surface() {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        gdk_screen_get_width(lualock.scr),
        gdk_screen_get_height(lualock.scr));
    return surface;
}

layer_t* create_layer(int width, int height) {
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		width,
		height);
    
    layer_t *layer = malloc(sizeof(layer_t));
    layer->surface = surface;
    layer->width = width;
    layer->height = height;
    layer->x = 0;
    layer->y = 0;
    layer->angle = 0;
    return layer;
}

void add_layer(layer_t *layer) {
    int i = 0;
    while (i < lualock.layers_alloc && lualock.layers[i] != NULL)
        i++;
    
    if (i >= lualock.layers_alloc) {
        lualock.layers_alloc += 20;
        lualock.layers = realloc(lualock.layers, lualock.layers_alloc);
    }
    
    lualock.layers[i] = layer;
    lualock.layers[i + 1] = NULL;
}

void remove_layer(layer_t *layer) {
    int i = 0;
    while (lualock.layers[i] != layer)
        i++;
    
    int j = 0;
    while (lualock.layers[i + j] != NULL) {
        lualock.layers[i + j] = lualock.layers[i + j + 1];
        j++;
    }
}

void parse_color(const char *hex, double *r, double *g, double *b, double *a) {
    unsigned long packed_rgb;
    // hex + 1 skips over the pound sign, which we don't need
    sscanf(hex + 1, "%lx", &packed_rgb);
    *r = (packed_rgb >> 16) / 256.0;
    *g = (packed_rgb >> 8 & 0xff) / 256.0;
    *b = (packed_rgb & 0xff) / 256.0;
    *a = 1;
}

char* get_password_mask() {
    char password_mask[strlen(lualock.password) + 1];
    for (unsigned int i = 0; i < strlen(lualock.password); i++)
        password_mask[i] = '*';
    password_mask[strlen(lualock.password)] = '\0';
    return strdup(password_mask);
}
