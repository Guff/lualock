#include "lua_api.h"
#include "lualock.h"
#include "drawing.h"

#include "clib/image.h"
#include "clib/background.h"
#include "clib/text.h"
#include "clib/timer.h"
#include "clib/style.h"
#include "clib/dpms.h"

bool lualock_lua_loadrc(lua_State *L, xdgHandle *xdg) {
    luaL_openlibs(lualock.L);
    
    lualock_lua_image_init(lualock.L);
    
    lualock_lua_background_init(lualock.L);
    
    lualock_lua_text_init(lualock.L);
    
    lualock_lua_timer_init(lualock.L);
    
    lualock_lua_style_init(lualock.L);
    
    lualock_lua_dpms_init(lualock.L);
    
    char *config = xdgConfigFind("lualock/rc.lua", xdg);
    if (luaL_loadfile(L, config) || lua_pcall(L, 0, 0, 0))
        return false;
    return true;
}
