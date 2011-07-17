#include <dbus/dbus-glib.h>

#include "dbus.h"

void init_dbus() {
	GError *error = NULL;
	DBusGConnection *gbus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	DBusConnection *bus = dbus_g_connection_get_connection(gbus);
	dbus_bus_add_match(bus, "type='signal',interface='org.guff.lualock.Signal'");
	dbus_connection_add_filter(bus, signal_filter, NULL, NULL);
}
	
static DBusHandlerResult signal_filter(DBusConnection *bus, DBusMessage *message,
									   void *data) {
	if (dbus_message_is_signal(message, "org.guff.lualock.Signal", "Lock")) {
		show_lock();
		return DBUS_HANDLER_RESULT_HANDLED;
	} else return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
