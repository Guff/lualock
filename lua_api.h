#include <lua.h>
#include <lauxlib.h>
#include <basedir.h>
#include <basedir_fs.h>
#include <stdbool.h>

#include "clib/image.h"
#include "clib/background.h"
#include "clib/text.h"
#include "clib/timer.h"
#include "clib/style.h"

bool lualock_lua_loadrc(lua_State *L, xdgHandle *xdg);
