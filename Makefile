PREFIX     ?= /usr/local
INSTALLDIR := $(DESTDIR)$(PREFIX)

CC	:= gcc

PKGS := cairo-xlib gdk-2.0 gdk-pixbuf-2.0 lua libxdg-basedir pangocairo
INCS := $(shell pkg-config --cflags $(PKGS))
LIBS := $(shell pkg-config --libs $(PKGS)) -lpam

CFLAGS	:= -g -Wall -Wextra -std=gnu99 -I. $(INCS) $(CFLAGS)
LDFLAGS	:= $(LIBS) $(LDFLAGS)

SRCS := $(wildcard *.c clib/*.c)
OBJS := $(foreach obj,$(SRCS:.c=.o),$(obj))


all: options lualock

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

clean:
	rm -rf lualock $(OBJS)
