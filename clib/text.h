#ifndef CLIB_TEXT_H
#define CLIB_TEXT_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <clutter/clutter.h>

typedef struct {
    char *text;
    
    double x;
    double y;
    
    char *font;
    char *font_color;
    
    double border_width;
    char *border_color;
    
    ClutterActor *actor;
    PangoLayout *layout;
} text_t;

text_t* text_new(text_t *text_obj, char *text, double x, double y,
                 char *font, char *font_color,
                 char *border_color, double border_width);

void lualock_lua_text_init(lua_State *L);

#endif
