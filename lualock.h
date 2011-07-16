#ifndef LUALOCK_H
#define LUALOCK_H

#include <lua.h>
#include <gtk/gtk.h>
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
    
    int timeout;
    
    guint *timers;
    guint timers_alloc;
    
    GHashTable *hooks;
    const char **hook_names;
    style_t style;
} lualock_t;

extern lualock_t lualock;

#endif
