#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "misc.h"

cairo_surface_t* create_surface(int width, int height) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        width ? : gdk_screen_get_width(lualock.scr),
        height ? : gdk_screen_get_height(lualock.scr));
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
    layer->scale_x = 1;
    layer->scale_y = 1;
    layer->x = 0;
    layer->y = 0;
    layer->angle = 0;
    return layer;
}

void add_layer(layer_t *layer) {
    // find the index of the layer at the end of the array
    int i = 0;
    for (; i < lualock.layers_alloc && lualock.layers[i] != NULL; i++);
    
    if (i >= lualock.layers_alloc) {
        lualock.layers_alloc += 20;
        lualock.layers = realloc(lualock.layers, lualock.layers_alloc);
    }
    
    lualock.layers[i] = layer;
    lualock.layers[i + 1] = NULL;
}

void update_layer(layer_t *layer, layer_t *new) {
    for (int i = 0; i < lualock.layers_alloc && lualock.layers[i] != NULL; i++) {
        if (lualock.layers[i] == layer)
            lualock.layers[i] = new;
    }
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

void parse_color(const char *color, double *r, double *g, double *b, double *a) {
    if (!color) {
        *r = 0, *g = 0, *b = 0, *a = 1;
        return;
    }
    GdkColor color_gdk;
    gdk_color_parse(color, &color_gdk);
    *r = color_gdk.red / (float) (1 << 16);
    *g = color_gdk.green / (float) (1 << 16);
    *b = color_gdk.blue / (float) (1 << 16);
    *a = 1;
}

char* get_password_mask() {
    char password_mask[strlen(lualock.password) + 1];
    for (unsigned int i = 0; i < strlen(lualock.password); i++)
        password_mask[i] = '*';
    password_mask[strlen(lualock.password)] = '\0';
    return strdup(password_mask);
}
