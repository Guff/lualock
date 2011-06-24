#include <stdio.h>
#include <unistd.h>
#include <cairo-xlib.h>
#include <X11/Intrinsic.h>

#include "globalconf.h"
#include "lua_api.h"

lualock_t lualock;

extern const struct luaL_reg lualock_image_lib[];

void init_display() {
    lualock.dpy = XOpenDisplay(NULL);
    lualock.scr = DefaultScreen(lualock.dpy);
}

void init_window() {
    Display *dpy = lualock.dpy;
    int scr = lualock.scr;
    
    XSetWindowAttributes attrs;
    attrs.override_redirect = 1;
    attrs.background_pixel = BlackPixel(dpy, scr);
    unsigned long attr_mask = CWOverrideRedirect | CWBackPixel;
    
    lualock.win = XCreateWindow(dpy, RootWindow(dpy, scr), 0, 0,
                                   DisplayWidth(dpy, scr),
                                   DisplayHeight(dpy, scr),
                                   0, DefaultDepth(dpy, scr),
                                   CopyFromParent, DefaultVisual(dpy, scr),
                                   attr_mask, &attrs);
    
    lualock.pmap = XCreatePixmap(dpy, lualock.win,
                                    DisplayWidth(dpy, scr),
                                    DisplayHeight(dpy, scr),
                                    DefaultDepth(dpy, scr));
        
    XSetWindowBackgroundPixmap(dpy, lualock.win, lualock.pmap);
    
}

void init_cairo() {
    Display *dpy = lualock.dpy;
    int scr = lualock.scr;
    lualock.surface = cairo_xlib_surface_create(dpy, lualock.pmap,
                                                DefaultVisual(dpy, scr),
                                                DisplayWidth(dpy, scr),
                                                DisplayHeight(dpy, scr));
    
    lualock.cr = cairo_create(lualock.surface);
    
}

void init_lua() {
    xdgHandle xdg;
    xdgInitHandle(&xdg);

    lualock.L = luaL_newstate();
    
    luaL_register(lualock.L, "image", lualock_image_lib);
    lua_pop(lualock.L, 1);

    lualock_lua_loadrc(lualock.L, &xdg);    
}



void event_handler(Display *dpy, Window win) {
    XEvent ev;
    XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    while (XNextEvent(dpy, &ev))
        if (ev.type == KeyPress)
            return;
}

int main() {    
    Display *dpy;
    Window win;
    
    cairo_surface_t *surface;
    cairo_t *cr;
    
    init_display();
    init_window();
    init_cairo();
    init_lua();
    
    dpy = lualock.dpy;
    win = lualock.win;
    cr = lualock.cr;
    
    XMapRaised(dpy, win);
    XClearWindow(dpy, win);
    event_handler(dpy, win);
    
    XCloseDisplay(dpy);
    
    return 0;
}
