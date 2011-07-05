#ifndef DRAWING_H
#define DRAWING_H

#include <cairo.h>

void draw_password_field(cairo_t *cr);
void draw_password_mask();
void start_drawing();
gboolean draw(void *data);

#endif
