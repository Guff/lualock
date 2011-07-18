PREFIX     ?= /usr/local
INSTALLDIR := $(DESTDIR)$(PREFIX)

CC	:= gcc

PKGS := clutter-gtk-1.0 lua xscrnsaver dbus-glib-1
INCS := $(shell pkg-config --cflags $(PKGS))
LIBS := $(shell pkg-config --libs $(PKGS)) -lpam

CFLAGS   := -Wall -Wextra -std=gnu99 -I. $(INCS) $(CFLAGS)
CPPFLAGS := -DLUALOCK_INSTALL_DIR=\"$(INSTALLDIR)/share/lualock\" $(CPPFLAGS)
DEBUG    := -g -DDEBUG
LDFLAGS  := $(LIBS) $(LDFLAGS)

SRCS  := $(wildcard *.c clib/*.c)
HEADS := $(wildcard *.h clib/*.h)
OBJS  := $(foreach obj,$(SRCS:.c=.o),$(obj))


all: options lualock

debug: CFLAGS += $(DEBUG)
debug: all

options:
	@echo lualock build options:
	@echo "CC               = $(CC)"
	@echo "CFLAGS           = $(CFLAGS)"
	@echo "LDFLAGS          = $(LDFLAGS)"
	@echo "INSTALLDIR       = $(INSTALLDIR)"
	@echo
	@echo "build targets:"
	@echo "SRCS = $(SRCS)"
	@echo

.c.o:
	@echo $(CC) -c $< -o $@
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

lualock: $(OBJS)
	@echo $(CC) -o $@ $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

apidoc:
	mkdir -p apidocs
	luadoc --nofiles -d apidocs lib/* luadoc/*

doc: $(HEADS) $(SRCS)
	doxygen -s lualock.doxygen

install:
	install -d $(INSTALLDIR)/share/lualock/
	cp -r lib $(INSTALLDIR)/share/lualock/
	chmod 755 $(INSTALLDIR)/share/lualock/lib/
	chmod 755 $(INSTALLDIR)/share/lualock/lib/odious/
	chmod 644 $(INSTALLDIR)/share/lualock/lib/odious/*.lua
	install -d $(INSTALLDIR)/bin/
	install lualock $(INSTALLDIR)/bin/lualock
	install -d $(DESTDIR)/etc/xdg/lualock/
	install -m644 config/rc.lua $(DESTDIR)/etc/xdg/lualock/
	install -D -m 644 lualock.pam $(DESTDIR)/etc/pam.d/lualock

clean:
	rm -rf lualock $(OBJS)
	rm -rf apidocs
