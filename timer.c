#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <unistd.h>

#include "globalconf.h"
#include "timer.h"

typedef struct {
    l_timer_t timer;
    lua_State *L;
} timer_lua;

l_timer_t timer_new(unsigned int int_us, int cycles, void (*cb)(void *), lua_State *L) {
        l_timer_t timer = { .cycles = cycles, .completed_cycles = 0,
                            .running = false, .int_us = int_us, .cb = cb };
	timer_lua data = { .timer = timer, .L = L };
        if (!pthread_create(&timer.thread, NULL, timer_run, &data)){
        }
        
        return timer;
}

void *timer_run(void *data) {
        l_timer_t timer = ((timer_lua *)data)->timer;
        lua_State *L = ((timer_lua *)data)->L;
        while(timer.cycles == 0 || timer.completed_cycles < timer.cycles) {
            printf("hi from timer\n");
            timer.cb(L);
            usleep(timer.int_us);
            timer.completed_cycles++;
        }
        return NULL;
}

void timer_run_lua_function(void *L) {
    lua_getglobal(L, "cb");
    lua_pcall((lua_State *)L, 0, 0, 0);
}

static int lualock_lua_timer_new(lua_State *L) {
    l_timer_t *timer = lua_newuserdata(L, sizeof(l_timer_t));
    
    *timer = timer_new(luaL_checknumber(L, 1) * 1000000, lua_tonumber(L, 2),
                       timer_run_lua_function, L);
    return 1;
}

void lualock_lua_timer_init(lua_State *L) {
        const struct luaL_reg lualock_timer_lib[] =
        {
                { "new", lualock_lua_timer_new },
                { NULL, NULL }
        };
        luaL_register(L, "timer", lualock_timer_lib);
}
