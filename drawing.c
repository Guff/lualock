#include <pthread.h>
#include <unistd.h>

#include "lualock.h"
#include "misc.h"
#include "drawing.h"

void draw_password_field(cairo_t *cr) {
    cairo_rectangle(cr, 0, 0, lualock.style.width,
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
    cairo_move_to(cr, lualock.style.off_x, lualock.style.off_y);
    pango_layout_set_text(layout, get_password_mask(), -1);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);
    cairo_fill(cr);
    cairo_destroy(cr);
    g_object_unref(layout);
}

gboolean draw(void *data) {
    if (lualock.need_updates <= 0)
        return TRUE;
    cairo_t *cr = cairo_create(lualock.surface_buf);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    int i = 0;
    while (i < lualock.layers_alloc && lualock.layers[i] != NULL) {
        cairo_save(cr);
        cairo_rotate(cr, lualock.layers[i]->angle);
        cairo_translate(cr, lualock.layers[i]->x, lualock.layers[i]->y);
        cairo_scale(cr, lualock.layers[i]->scale_x, lualock.layers[i]->scale_y);
        cairo_set_source_surface(cr, lualock.layers[i]->surface, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
        i++;
    }
    draw_password_mask();
    cairo_translate(cr, lualock.style.x, lualock.style.y);
    cairo_set_source_surface(cr, lualock.pw_surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    
    cairo_t *crw = gdk_cairo_create(lualock.win);
    cairo_set_source_surface(crw, lualock.surface_buf, 0, 0);
    cairo_set_operator(crw, CAIRO_OPERATOR_SOURCE);
    cairo_paint(crw);
    cairo_destroy(crw);
    
    lualock.need_updates--;
    return TRUE;
}

void* draw_frames(void *data) {
	while(1) {
		draw(NULL);
		usleep(1000000 / 10);
	}
}

void start_drawing() {
    //draw_password_mask();
	pthread_t draw_thread;
	pthread_create(&draw_thread, NULL, draw_frames, NULL);
}
