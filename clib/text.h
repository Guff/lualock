//      text.h - API for drawing text
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

#ifndef LUALOCK_CLIB_TEXT_H
#define LUALOCK_CLIB_TEXT_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
    char *text;
    
    double x;
    double y;
    
    char *font;
    char *font_color;
    
    double border_width;
    char *border_color;
    
    layer_t *layer;
    PangoLayout *layout;
} text_t;

text_t* text_new(text_t *text_obj, char *text, double x, double y,
                 char *font, char *font_color,
                 char *border_color, double border_width);

void lualock_lua_text_init(lua_State *L);

#endif
