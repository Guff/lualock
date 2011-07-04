#include <pthread.h>
#include <unistd.h>

#include "lualock.h"
#include "misc.h"
#include "drawing.h"

void draw_password_field(cairo_t *cr) {
    cairo_rectangle(cr, lualock.style.x, lualock.style.y, lualock.style.width,
					lualock.style.height);
	cairo_clip_preserve(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 0, 0, 0, .6);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);
}

void draw_password_mask() {
    cairo_t *cr = cairo_create(lualock.pw_surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    draw_password_field(cr);
    
    PangoLayout *layout = pango_cairo_create_layout(cr);
    
    PangoFontDescription *desc =
        pango_font_description_from_string(lualock.style.font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    cairo_set_source_rgba(cr, lualock.style.r, lualock.style.g,
                          lualock.style.b, lualock.style.a);
    cairo_move_to(cr, lualock.style.x + lualock.style.off_x,
				  lualock.style.y + lualock.style.off_y);
    pango_layout_set_text(layout, get_password_mask(), -1);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);
    cairo_fill(cr);
    cairo_destroy(cr);
}

void draw() {
    cairo_t *cr = cairo_create(lualock.surface_buf);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    int i = 0;
    while (i < lualock.surfaces_alloc && lualock.surfaces[i] != NULL) {
        cairo_set_source_surface(cr, lualock.surfaces[i], 0, 0);
        cairo_paint(cr);
        i++;
    }
    draw_password_mask();
    cairo_set_source_surface(cr, lualock.pw_surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    cairo_t *crw = cairo_create(lualock.surface);
    cairo_set_source_surface(crw, lualock.surface_buf, 0, 0);
    cairo_set_operator(crw, CAIRO_OPERATOR_SOURCE);
    cairo_paint(crw);
    cairo_destroy(crw);
}

void* draw_frames(void *data) {
	while(1) {
		draw();
		usleep(1000000 / 10);
	}
}

void start_drawing() {
    //draw_password_mask();
	pthread_t draw_thread;
	pthread_create(&draw_thread, NULL, draw_frames, NULL);
}
