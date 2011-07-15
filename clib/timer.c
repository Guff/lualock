#include <lua.h>
#include <lualib.h>
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
    timer_run(timer);
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
    lua_pushcfunction(timer->L, lualock_lua_on_error);
    lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->r);
    lua_pcall(timer->L, 0, 0, -2);
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
