#ifndef MISC_H
#define MISC_H

#include <clutter/clutter.h>

#include "lualock.h"

cairo_surface_t* create_surface(int width, int height);

ClutterActor* create_actor(int width, int height);
void add_actor(ClutterActor *actor);
void update_actor(ClutterActor *actor, ClutterActor *new);

void parse_color(const char *hex, double *r, double *g, double *b, double *a);

char* get_password_mask();

void get_abs_pos(double rel_x, double rel_y, double *x, double *y);

#endif
