//      timer.h - timer API for running lua code periodically
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

#ifndef LUALOCK_CLIB_TIMER_H
#define LUALOCK_CLIB_TIMER_H

#include <lualib.h>

typedef struct {
    int id;
    lua_State *L;
    gboolean running;
    int cycles;
    int completed_cycles;
    int int_us;
    int r;
    void (*cb)(void*);
} l_timer_t;

void timer_new(l_timer_t *timer, unsigned int int_us, int cycles,
               void (*cb)(void*));
gboolean timer_run(void *data);

void lualock_lua_timer_init(lua_State *L);

#endif
