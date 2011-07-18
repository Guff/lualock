#ifndef LUALOCK_CLIB_BACKGROUND_H
#define LUALOCK_CLIB_BACKGROUND_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void background_set_color(const char *hex);

void lualock_lua_background_init(lua_State *L);

#endif
