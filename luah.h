#include <lua.h>
#include <lauxlib.h>
#include <basedir.h>
#include <basedir_fs.h>
#include <stdbool.h>

bool luaH_loadrc(lua_State *L, xdgHandle *xdg);
