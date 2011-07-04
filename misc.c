#include <stdlib.h>
#include <string.h>

#include "lualock.h"
#include "misc.h"

cairo_surface_t* create_surface() {
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		gdk_screen_get_width(lualock.scr),
		gdk_screen_get_height(lualock.scr));
    return surface;
}

void add_surface(cairo_surface_t *surface) {
    int i = 0;
    while (i < lualock.surfaces_alloc && lualock.surfaces[i] != NULL)
        i++;
    
    if (i >= lualock.surfaces_alloc) {
        lualock.surfaces_alloc += 20;
        lualock.surfaces = realloc(lualock.surfaces, lualock.surfaces_alloc);
    }
    
    lualock.surfaces[i] = surface;
    lualock.surfaces[i + 1] = NULL;
}

void remove_surface(cairo_surface_t *surface) {
    int i = 0;
    while (lualock.surfaces[i] != surface)
        i++;
    
    int j = 0;
    while (lualock.surfaces[i + j] != NULL) {
        lualock.surfaces[i + j] = lualock.surfaces[i + j + 1];
        j++;
    }
}

void parse_color(const char *hex, double *r, double *g, double *b, double *a) {
    unsigned long packed_rgb;
    // hex + 1 skips over the pound sign, which we don't need
    sscanf(hex + 1, "%lx", &packed_rgb);
    *r = (packed_rgb >> 16) / 256.0;
    *g = (packed_rgb >> 8 & 0xff) / 256.0;
    *b = (packed_rgb & 0xff) / 256.0;
    *a = 1;
}

char* get_password_mask() {
    char password_mask[strlen(lualock.password) + 1];
    for (unsigned int i = 0; i < strlen(lualock.password); i++)
        password_mask[i] = '*';
    password_mask[strlen(lualock.password)] = '\0';
    return strdup(password_mask);
}
