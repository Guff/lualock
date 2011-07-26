//      misc.c - miscellaneous functions for lualock
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "datatypes.h"
#include "misc.h"

cairo_surface_t* create_surface(gint width, gint height) {
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
    layer->show = FALSE;
    return layer;
}

void add_layer(layer_t *layer) {
    ptr_array_add(lualock.layers, layer);
}

void remove_layer(layer_t *layer) {
    ptr_array_remove(lualock.layers, layer);
    layer_destroy(layer);
}

void replace_layer(layer_t *old_layer, layer_t *new_layer) {
    for (guint i = 0; i < lualock.layers->len; i++) {
        if (ptr_array_index(lualock.layers, i) == old_layer) {
            layer_destroy(old_layer);
            lualock.layers->data[i] = new_layer;
            return;
        }
    }
}

void layer_destroy(layer_t *layer) {
    cairo_surface_destroy(layer->surface);
    free(layer);
}

void parse_color(const gchar *color, gdouble *r, gdouble *g, gdouble *b, gdouble *a) {
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

void get_abs_pos(gdouble rel_x, gdouble rel_y, gdouble *x, gdouble *y) {
    *x = rel_x >= 1.0 ? rel_x : rel_x * gdk_screen_get_width(lualock.scr);
    *y = rel_y >= 1.0 ? rel_y : rel_y * gdk_screen_get_height(lualock.scr);
}

void get_abs_pos_for_dims(gdouble dim_w, gdouble dim_h, gdouble rel_w, gdouble rel_h,
                          gdouble *w, gdouble *h) {
    *w = rel_w >= 1.0 ? rel_w : rel_w * dim_w;
    *h = rel_h >= 1.0 ? rel_h : rel_h * dim_h;
}

void add_timer(guint id) {
    uint_array_add(lualock.timers, id);
}

void remove_timer(guint id) {
    if (!id)
        return;
    uint_array_remove(lualock.timers, id);
}

void clear_timers() {
    for (guint i = 0; i < lualock.timers->len; i++)
        g_source_remove(g_array_index(lualock.timers, guint, i));
    
    uint_array_clear(lualock.timers);
}

void register_update(gdouble x, gdouble y, gdouble w, gdouble h) {
    cairo_rectangle_int_t rect = { x, y, w, h };
    cairo_region_union_rectangle(lualock.updates_needed, &rect);
}

void register_update_for_layer(layer_t *layer) {
    gdouble x, y, width, height;
    x = layer->x, y = layer->y;
    width = layer->width * layer->scale_x;
    height = layer->height * layer->scale_y;
    
    register_update(x, y, width, height);
}

void clear_updates() {
    cairo_region_t *region = cairo_region_create();
    cairo_region_intersect(lualock.updates_needed, region);
    cairo_region_destroy(region);
}

void update_screen() {
    register_update(0, 0, gdk_screen_get_width(lualock.scr),
                    gdk_screen_get_height(lualock.scr));
}
