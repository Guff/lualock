//      timer.c - timer API for running lua code periodically
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

#include <lauxlib.h>
#include <stdlib.h>
#include <unistd.h>

#include "lualock.h"
#include "lua_api.h"
#include "misc.h"
#include "timer.h"

void timer_new(l_timer_t *timer, unsigned int int_us, int cycles,
               void (*cb)(void*)) {
    timer->cycles = cycles;
    timer->completed_cycles = 0,
    timer->running = TRUE;
    timer->int_us = int_us;
    timer->cb = cb;
    add_timer(timer->id = g_timeout_add(int_us, timer_run, timer));
}
    
gboolean timer_run(void *data) {
    l_timer_t *timer = (l_timer_t *)data;
    if (timer->cycles != 0 && timer->completed_cycles >= timer->cycles)
        return FALSE;
    else
        timer->cb(timer);
    timer->completed_cycles++;
    return TRUE;
}

static void timer_run_lua_function(void *data) {
    l_timer_t *timer = data;
    lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->r);
    lualock_lua_do_function(timer->L);
}

static int lualock_lua_timer_new(lua_State *L) {
    int interval = luaL_checknumber(L, 2) * 1000;
    int run_times = lua_tonumber(L, 3);
    lua_settop(L, 3);
    
    l_timer_t *timer = lua_newuserdata(L, sizeof(l_timer_t));
    timer->L = L;
    lua_pushvalue(L, 1);
    timer->r = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, "lualock.timer");
    timer_new(timer, interval, run_times, timer_run_lua_function);
    lua_setmetatable(L, -2);
    
    // keep the userdata referenced
    lua_pushvalue(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    
    return 1;
}

void lualock_lua_timer_init(lua_State *L) {
    luaL_newmetatable(L, "lualock.timer");
    
    lua_register(L, "timer", lualock_lua_timer_new);
}
