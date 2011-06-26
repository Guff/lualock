#include <lua.h>
#include <lauxlib.h>
#include <basedir.h>
#include <basedir_fs.h>
#include <stdbool.h>

#include "image.h"
#include "background.h"

bool lualock_lua_loadrc(lua_State *L, xdgHandle *xdg);
