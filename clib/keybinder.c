#include <stdlib.h>
#include <lauxlib.h>

#include "lualock.h"
#include "clib/keybinder.h"

int lualock_lua_keybinder(lua_State *L) {
	lua_settop(L, 2);
	keybind_t *bind = malloc(sizeof(keybind_t));
	gtk_accelerator_parse(luaL_checkstring(L, 1), &bind->val, &bind->mod);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	bind->r = luaL_ref(L, LUA_REGISTRYINDEX);
	g_ptr_array_add(lualock.keybinds, bind);
	return 0;
}
void lualock_lua_keybinder_init(lua_State *L) {
	lua_register(L, "keybinder", lualock_lua_keybinder);
}
