#ifndef MISC_H
#define MISC_H

#include <clutter/clutter.h>

#include "lualock.h"

cairo_surface_t* create_surface(gint width, gint height);

ClutterActor* create_actor(gint width, gint height);
void add_actor(ClutterActor *actor);
void update_actor(ClutterActor *actor, ClutterActor *new);

void parse_color(const gchar *hex, gdouble *r, gdouble *g, gdouble *b, gdouble *a);

char* get_password_mask();

void get_abs_pos(double rel_x, double rel_y, double *x, double *y);
void get_abs_pos_for_dims(double dim_w, double dim_h, double rel_w, double rel_h,
                          double *w, double *h);

void add_timer(guint id);
void clear_timers();

#endif
