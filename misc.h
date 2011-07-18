//      misc.h - miscellaneous functions for lualock
//      Copyright ©2011 Guff <cassmacguff@gmail.com>
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//

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
