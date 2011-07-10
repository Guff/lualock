#ifndef LUA_API_H
#define LUA_API_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <basedir.h>
#include <basedir_fs.h>
#include <stdbool.h>

bool lualock_lua_loadrc(lua_State *L, xdgHandle *xdg);

#endif
