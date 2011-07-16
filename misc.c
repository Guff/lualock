#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>

#include "misc.h"

cairo_surface_t* create_surface(int width, int height) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        width ? : gdk_screen_get_width(lualock.scr),
        height ? : gdk_screen_get_height(lualock.scr));
    return surface;
}

ClutterActor* create_actor(int width, int height) {
	ClutterActor *actor = clutter_cairo_texture_new(
        width ? : gdk_screen_get_width(lualock.scr),
        height ? : gdk_screen_get_height(lualock.scr));
    return actor;
}

void add_actor(ClutterActor *actor) {
    clutter_container_add_actor(CLUTTER_CONTAINER(lualock.stage), actor);
    clutter_actor_hide(actor);
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

void get_abs_pos(double rel_x, double rel_y, double *x, double *y) {
    if (rel_x >= 1.0)
        *x = rel_x;
    else
        *x = rel_x * gdk_screen_get_width(lualock.scr);
    
    if (rel_y >= 1.0)
        *y = rel_y;
    else
        *y = rel_y * gdk_screen_get_height(lualock.scr);
}

void add_timer(guint id) {
    g_array_append_val(lualock.timers, id);
}

void clear_timers() {
    for (guint i = 0; i < lualock.timers->len; i++)
        g_source_remove(g_array_index(lualock.timers, guint, i));
    
    g_array_remove_range(lualock.timers, 0, lualock.timers->len);
}
