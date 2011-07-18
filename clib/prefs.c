//      prefs.c - API for setting lualock's preferences
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

#include "misc.h"
#include "clib/prefs.h"

int lualock_lua_prefs_set(lua_State *L) {
    lua_getfield(L, 1, "timeout");
    lualock.timeout = luaL_checkint(L, 2);
    
    return 0;
}

void lualock_lua_prefs_init(lua_State *L) {
    lua_register(L, "prefs", lualock_lua_prefs_set);
}
