//      drawing.c - functions related to drawing
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

#include <unistd.h>
#include <stdlib.h>

#include "lualock.h"
#include "misc.h"
#include "drawing.h"

void draw_password_field(cairo_t *cr) {
    register_update(lualock.style.x, lualock.style.y,
                    lualock.style.width, lualock.style.height);
    cairo_rectangle(cr, 0, 0, lualock.style.width,
                    lualock.style.height);
    cairo_clip_preserve(cr);
    gdk_cairo_set_source_rgba(cr, &lualock.style.bg_color);
    cairo_fill_preserve(cr);
    gdk_cairo_set_source_rgba(cr, &lualock.style.border_color);
    cairo_set_line_width(cr, lualock.style.border_width);
    cairo_stroke(cr);
}

void draw_password_mask(void) {
    cairo_t *cr = cairo_create(lualock.pw_surface);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    draw_password_field(cr);
    
    PangoLayout *layout = pango_cairo_create_layout(cr);
    
    pango_layout_set_font_description(layout, lualock.style.font_desc);
    cairo_set_source_rgba(cr, lualock.style.r, lualock.style.g,
                          lualock.style.b, lualock.style.a);
    cairo_move_to(cr, lualock.style.off_x, lualock.style.off_y);
    gchar *password_mask = get_password_mask();
    pango_layout_set_text(layout, password_mask, -1);
    free(password_mask);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);
    cairo_fill(cr);
    g_object_unref(layout);
    cairo_destroy(cr);
}

gboolean draw(gpointer data) {
    if (cairo_region_is_empty(lualock.updates_needed))
        return TRUE;
    cairo_rectangle_int_t extents;
    cairo_region_get_extents(lualock.updates_needed, &extents);
    cairo_t *cr = cairo_create(lualock.surface_buf);
    cairo_rectangle(cr, extents.x, extents.y, extents.width, extents.height);
    cairo_clip(cr);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_set_source_surface(cr, lualock.bg_surface, 0, 0);
    cairo_paint(cr);

    for (guint i = 0; i < lualock.layers->len; i++) {
        layer_t *layer = g_ptr_array_index(lualock.layers, i);
        if (!layer->show)
            continue;
        cairo_save(cr);
        cairo_translate(cr, layer->x, layer->y);
        cairo_scale(cr, layer->scale_x, layer->scale_y);
        cairo_rotate(cr, layer->angle);
        cairo_set_source_surface(cr, layer->surface, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
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
    
    clear_updates();
    return TRUE;
}

