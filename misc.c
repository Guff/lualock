#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>

#include "misc.h"

cairo_surface_t* create_surface(gint width, gint height) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        width ? : gdk_screen_get_width(lualock.scr),
        height ? : gdk_screen_get_height(lualock.scr));
    return surface;
}

ClutterActor* create_actor(gint width, gint height) {
	ClutterActor *actor = clutter_cairo_texture_new(
        width ? : gdk_screen_get_width(lualock.scr),
        height ? : gdk_screen_get_height(lualock.scr));
    return actor;
}

void add_actor(ClutterActor *actor) {
    clutter_container_add_actor(CLUTTER_CONTAINER(lualock.stage), actor);
    clutter_actor_hide(actor);
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
    g_array_append_val(lualock.timers, id);
}

void clear_timers() {
    for (guint i = 0; i < lualock.timers->len; i++)
        g_source_remove(g_array_index(lualock.timers, guint, i));
    
    if (lualock.timers->len)
        g_array_remove_range(lualock.timers, 0, lualock.timers->len);
}
