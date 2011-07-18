//      lualock.h - common stuff
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
    
    GArray *timers;
    
    GHashTable *hooks;
    const char **hook_names;
    style_t style;
} lualock_t;

extern lualock_t lualock;

#endif
