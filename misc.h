#include <cairo.h>

cairo_surface_t* create_surface();
void add_surface(cairo_surface_t *surface);
void remove_surface(cairo_surface_t *surface);

void parse_color(const char *hex, double *r, double *g, double *b, double *a);

