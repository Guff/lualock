//      lua_api.c - sets up lualock's lua API
//      Copyright ©2011 Guff <cassmacguff@gmail.com>
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//

#include <oocairo.h>

#include "lua_api.h"
#include "lualock.h"
#include "drawing.h"

#include "clib/image.h"
#include "clib/background.h"
#include "clib/timer.h"
#include "clib/style.h"
#include "clib/prefs.h"
#include "clib/hook.h"
#include "clib/utils.h"
#include "clib/keybinder.h"
#include "clib/cairo_surface.h"

int lualock_lua_on_error(lua_State *L) {
    printf("error: %s\n", luaL_checkstring(L, -1));
    return 1;
}

void lualock_lua_do_function(lua_State *L) {
    if (lua_pcall(L, 0, 0, 0))
        lualock_lua_on_error(L);
}

gboolean lualock_lua_loadrc(lua_State *L) {
    luaL_openlibs(L);
    
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    
#ifdef DEBUG
    lua_pushliteral(L, ";./lib/?.lua");
    lua_pushliteral(L, ";./lib/?/init.lua");
    lua_concat(L, 2);
    lua_concat(L, 2);
#endif

    lua_pushliteral(L, ";" LUALOCK_INSTALL_DIR "/lib/?.lua");
    lua_pushliteral(L, ";" LUALOCK_INSTALL_DIR "/lib/?/init.lua");
    lua_concat(L, 2);
    lua_concat(L, 2);
    lua_setfield(L, 1, "path");
    lua_pop(L, 1);

    luaopen_oocairo(L);
    
    lualock_lua_image_init(L);
    
    lualock_lua_background_init(L);
    
    lualock_lua_timer_init(L);
    
    lualock_lua_style_init(L);
    
    lualock_lua_prefs_init(L);
    
    lualock_lua_hook_init(L);
    
    lualock_lua_utils_init(L);
    
    lualock_lua_keybinder_init(L);
    
    lualock_lua_cairo_surface_init(L);
        
    GPtrArray *configs = g_ptr_array_new();
    g_ptr_array_add(configs, g_build_filename(g_get_user_config_dir(), "lualock",
                                              "rc.lua", NULL));
    const gchar * const *system_config_dirs = g_get_system_config_dirs();
    for (guint i = 0; system_config_dirs[i]; i++)
        g_ptr_array_add(configs, g_build_filename(system_config_dirs[i],
                                                  "lualock", "rc.lua", NULL));
    for (guint i = 0; i < configs->len; i++) {
        if (!luaL_loadfile(L, g_ptr_array_index(configs, i))) {
            lualock_lua_do_function(L);
            return TRUE;
        }
    }
    
    g_ptr_array_free(configs, TRUE);
    return FALSE;
}
