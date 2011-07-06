#ifndef MISC_H
#define MISC_H

#include <cairo.h>

#include "lualock.h"

cairo_surface_t* create_surface(int width, int height);

layer_t* create_layer(int width, int height);
void add_layer(layer_t *layer);
void remove_layer(layer_t *layer);
void update_layer(layer_t *layer, layer_t *new);

void parse_color(const char *hex, double *r, double *g, double *b, double *a);

char* get_password_mask();

#endif
