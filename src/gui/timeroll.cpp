// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "timeroll.h"
#include "../core/globals.h"
#include "styles.h"

TimeRoll::TimeRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_hscroll = 0;
    m_zoom = c_default_zoom;

    add_events(Gdk::SCROLL_MASK);

}

TimeRoll::~TimeRoll()
{
}


bool
TimeRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // Horizontal lines

    cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_separator);
    cr->set_line_width(1.0);

    cr->move_to(0, height - 0.5);
    cr->line_to(width, height -0.5);
    cr->stroke();


    // Vertical lines

    Pango::FontDescription font;
    int text_width;
    int text_height;

    font.set_family(c_font);
    font.set_size(c_key_fontsize * Pango::SCALE);
    font.set_weight(Pango::WEIGHT_BOLD);

    int ticks_per_measure =  m_sequence->get_bpm() * (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_step = 3 * m_zoom;
    int start_tick = m_hscroll - (m_hscroll % ticks_per_step);
    int end_tick = start_tick + width * m_zoom;
    if (m_sequence->get_length() < end_tick) end_tick = m_sequence->get_length();
    int m = ceil(1.0 * start_tick / ticks_per_measure);
    int last_measure = 0;
    bool preroll = m_hscroll > 0;

    for (int i=start_tick; i<=end_tick+ticks_per_step; i+=ticks_per_step)
    {
        int base_line = (i - m_hscroll) / m_zoom + c_keys_width;

        if ( i % ticks_per_measure <= last_measure  || preroll)
        {
            if ( i % ticks_per_measure == last_measure) preroll = false;

            if (preroll) {
                m -= 1;
                base_line = max((int)((ticks_per_measure * m - m_hscroll) / m_zoom  + c_keys_width), -1);
                cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_beat);
            } else {
                cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_measure);
            }

            cr->move_to(base_line + 0.5, 0);
            cr->line_to(base_line + 0.5, height);
            cr->stroke();


            m++;
            string measure = i >= end_tick ? (string) "END" : to_string(m);
            auto t = create_pango_layout(measure);
            t->set_font_description(font);
            t->get_pixel_size(text_width, text_height);
            cr->move_to(base_line + 4, height / 2 - text_height / 2);
            t->show_in_cairo_context(cr);
        }

        preroll = false;
        last_measure = i % ticks_per_measure;

    }

    return true;
}

void
TimeRoll::set_zoom(double zoom)
{
    if (zoom < c_min_zoom) zoom = c_min_zoom;
    else if (zoom > c_max_zoom) zoom = c_max_zoom;
    m_zoom = zoom;
    queue_draw();
}


bool
TimeRoll::on_scroll_event(GdkEventScroll* event)
{

    if (!(event->state & GDK_CONTROL_MASK)) event->state = GDK_SHIFT_MASK;

    if (signal_scroll.emit(event)) return true;

    return false;
}
