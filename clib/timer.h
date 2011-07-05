#ifndef CLIB_TIMER_H
#define CLIB_TIMER_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    bool running;
    int cycles;
    int completed_cycles;
    int int_us;
    void (*cb)(void);
} l_timer_t;

void timer_new(l_timer_t *timer, unsigned int int_us, int cycles, void (*cb)(void));
gboolean timer_run(void *data);

void lualock_lua_timer_init(lua_State *L);

#endif
