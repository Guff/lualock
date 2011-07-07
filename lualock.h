#ifndef LUALOCK_H
#define LUALOCK_H

#include <lua.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
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
    double scale_x;
    double scale_y;
	double angle;
} layer_t;

typedef struct {
    lua_State *L;
    
    Display *dpy;
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
    
    layer_t **layers;
    int layers_alloc;
    gboolean need_updates;
    
    BOOL dpms_enabled;
    CARD16 dpms_standby, dpms_suspend, dpms_off;
    
    GMainLoop *loop;
    
    style_t style;
} lualock_t;

extern lualock_t lualock;

#endif
