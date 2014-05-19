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

#define luaL_register(L,n,l)    (luaL_openlib(L,(n),(l),0))
#include <lauxlib.h>
#include <stdlib.h>
#include <unistd.h>

#include "lualock.h"
#include "lua_api.h"
#include "misc.h"
#include "timer.h"

void timer_new(l_timer_t *timer, guint int_us, guint cycles,
               void (*cb)(gpointer)) {
    timer->cycles = cycles;
    timer->completed_cycles = 0;
    timer->running = TRUE;
    timer->int_us = int_us;
    timer->cb = cb;
    timer->id = 0;
}

void timer_start(l_timer_t *timer) {
    add_timer(timer->id = g_timeout_add(timer->int_us, timer_run, timer));
}    
    
gboolean timer_run(gpointer data) {
    l_timer_t *timer = (l_timer_t *)data;
    if (timer->cycles != 0 && timer->completed_cycles >= timer->cycles)
        return FALSE;
    else
        timer->cb(timer);
    timer->completed_cycles++;
    return TRUE;
}

static void timer_run_lua_function(gpointer data) {
    l_timer_t *timer = data;
    lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->r);
    lualock_lua_do_function(timer->L);
}

static gint lualock_lua_timer_new(lua_State *L) {
    guint interval = luaL_checknumber(L, 2) * 1000;
    guint run_times = lua_tonumber(L, 3);
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

static gint lualock_lua_timer_start(lua_State *L) {
    l_timer_t *timer = luaL_checkudata(L, 1, "lualock.timer");
    timer_start(timer);
    return 0;
}
static gint lualock_lua_timer_stop(lua_State *L) {
    l_timer_t *timer = luaL_checkudata(L, 1, "lualock.timer");
    remove_timer(timer->id);
    timer->id = 0;
    return 0;
}

void lualock_lua_timer_init(lua_State *L) {
    const struct luaL_Reg lualock_timer_lib[] = {
        { "start", lualock_lua_timer_start },
        { "stop", lualock_lua_timer_stop },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, "lualock.timer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    luaL_register(L, NULL, lualock_timer_lib);
    lua_pop(L, 1);
    lua_register(L, "timer", lualock_lua_timer_new);
}
