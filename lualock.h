#ifndef LUALOCK_H
#define LUALOCK_H

#include <lua.h>
#include <gdk/gdk.h>
#include <pango/pangocairo.h>

typedef struct {
    char *font;
    
    int x;
    int y;
    
    int off_x;
    int off_y;
    
    int width;
    int height;
    
    double r;
    double g;
    double b;
    double a;
} style_t;

typedef struct {
	cairo_surface_t *surface;
	int x;
	int y;
	int width;
	int height;
	double angle;
} layer_t;

typedef struct {
    lua_State *L;
    
    GdkDisplay *dpy;
    GdkScreen *scr;
    GdkWindow *win;
    
    char *password;
    int pw_length;
    int pw_alloc;
    
    char *password_font;
    
    struct pam_handle *pam_handle;
    
    cairo_surface_t *surface_buf;
    cairo_surface_t *surface;
    
    cairo_surface_t *pw_surface;
    cairo_t *pw_cr;
    
    layer_t **layers;
    int layers_alloc;

    style_t style;
} lualock_t;

extern lualock_t lualock;

#endif
