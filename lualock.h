#ifndef LUALOCK_H
#define LUALOCK_H

#include <lua.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <clutter/clutter.h>

typedef struct {
    const char *font;
    
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
    lua_State *L;
    
    GdkScreen *scr;
    GtkWidget *win;
    
    ClutterActor *stage;

    char *password;
    int pw_length;
    int pw_alloc;
    
    char *password_font;
    
    struct pam_handle *pam_handle;
    
    ClutterActor *pw_actor;
    
    ClutterActor **actors;
    int actors_alloc;
    
    BOOL dpms_enabled, using_dpms;
    CARD16 dpms_standby, dpms_suspend, dpms_off;
    CARD16 dpms_cfg_standby, dpms_cfg_suspend, dpms_cfg_off;
    
    style_t style;
} lualock_t;

extern lualock_t lualock;

#endif
