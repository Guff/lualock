#include "lua_api.h"
#include "lualock.h"
#include "drawing.h"

#include "clib/image.h"
#include "clib/background.h"
#include "clib/text.h"
#include "clib/timer.h"
#include "clib/style.h"
#include "clib/prefs.h"
#include "clib/dpms.h"

gboolean lualock_lua_loadrc(lua_State *L) {
    luaL_openlibs(lualock.L);
    
    lualock_lua_image_init(lualock.L);
    
    lualock_lua_background_init(lualock.L);
    
    lualock_lua_text_init(lualock.L);
    
    lualock_lua_timer_init(lualock.L);
    
    lualock_lua_style_init(lualock.L);
    
    lualock_lua_prefs_init(lualock.L);
    
    lualock_lua_dpms_init(lualock.L);
    
    gchar *config = g_build_filename(g_get_user_config_dir(), "lualock", "rc.lua",
                                    NULL);
    if (luaL_loadfile(L, config) || lua_pcall(L, 0, 0, 0))
        return false;
    return true;
}
