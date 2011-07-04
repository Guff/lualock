#include <pthread.h>
#include <stdbool.h>

typedef struct {
    pthread_t thread;
    bool running;
    int cycles;
    int completed_cycles;
    int int_us;
    void (*cb)(void);
} l_timer_t;

void timer_new(l_timer_t *timer, unsigned int int_us, int cycles, void (*cb)(void));
void* timer_run(void *data);

void lualock_lua_timer_init(lua_State *L);
