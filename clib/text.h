#ifndef CLIB_TEXT_H
#define CLIB_TEXT_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <clutter/clutter.h>

typedef struct {
    const char *text;
    
    double x;
    double y;
    
    const char *font;
    const char *font_color;
    
    double border_width;
    const char *border_color;
    
    ClutterActor *actor;
    PangoLayout *layout;
} text_t;

text_t* text_new(text_t *text_obj, const char *text, int x, int y,
                 const char *font, const char *font_color,
                 const char *border_color, double border_width);

void lualock_lua_text_init(lua_State *L);

#endif
