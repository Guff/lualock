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
    cairo_t *cr = clutter_cairo_texture_create(CLUTTER_CAIRO_TEXTURE(lualock.pw_actor));
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
    gchar *password_mask = get_password_mask();
    pango_layout_set_text(layout, password_mask, -1);
    free(password_mask);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);
    cairo_fill(cr);
    g_object_unref(layout);
    cairo_destroy(cr);
}
