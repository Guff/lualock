#include "lua_api.h"
#include "lualock.h"
#include "drawing.h"

#include "clib/image.h"
#include "clib/background.h"
#include "clib/text.h"
#include "clib/timer.h"
#include "clib/style.h"
#include "clib/prefs.h"
#include "clib/hook.h"
#include "clib/spawn.h"

int lualock_lua_on_error(lua_State *L) {
    printf("error: %s\n", luaL_checkstring(L, -1));
    return 1;
}

gboolean lualock_lua_loadrc(lua_State *L) {
    luaL_openlibs(lualock.L);
    
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    
    lua_pushliteral(L, ";" LUALOCK_INSTALL_DIR "/lib/?.lua");
    lua_pushliteral(L, ";" LUALOCK_INSTALL_DIR "/lib/?/init.lua");
    lua_concat(L, 2);
    lua_concat(L, 2);
    lua_setfield(L, 1, "path");
    lua_pop(L, 1);
    
    lualock_lua_image_init(lualock.L);
    
    lualock_lua_background_init(lualock.L);
    
    lualock_lua_text_init(lualock.L);
    
    lualock_lua_timer_init(lualock.L);
    
    lualock_lua_style_init(lualock.L);
    
    lualock_lua_prefs_init(lualock.L);
    
    lualock_lua_hook_init(lualock.L);
    
    lualock_lua_spawn_init(lualock.L);
    
    lua_pushcfunction(L, lualock_lua_on_error);
    
    gchar *config = g_build_filename(g_get_user_config_dir(), "lualock", "rc.lua",
                                     NULL);
    if (luaL_loadfile(L, config) || lua_pcall(L, 0, 0, -2)) {
        return false;
    }
    return true;
}
