#ifndef LUA_API_H
#define LUA_API_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>

int lualock_lua_on_error(lua_State *L);

gboolean lualock_lua_loadrc(lua_State *L);

#endif
