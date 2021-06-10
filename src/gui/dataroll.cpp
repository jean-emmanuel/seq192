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


#include "../core/globals.h"

#include "dataroll.h"
#include "styles.h"

DataRoll::DataRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_hscroll = 0;
    m_zoom = c_default_zoom;

    m_dragging = false;
    m_drag_handle = false;

    m_status = EVENT_NOTE_ON;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &DataRoll::on_draw));

    add_events(Gdk::BUTTON_PRESS_MASK |
		       Gdk::BUTTON_RELEASE_MASK |
		       Gdk::POINTER_MOTION_MASK |
               Gdk::SCROLL_MASK
    );

}

DataRoll::~DataRoll()
{
}


bool
DataRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    Pango::FontDescription font;
    int text_width;
    int text_height;

    font.set_family(c_font);
    font.set_size(c_key_fontsize * Pango::SCALE);
    font.set_weight(Pango::WEIGHT_NORMAL);

    long tick;
    unsigned char d0,d1;
    bool selected;
    int event_x;
    int event_height;

    /*  For note ON there can be multiple events on the same vertical in which
        the selected item can be covered.  For note ON's the selected item needs
        to be drawn last so it can be seen.  So, for other events the below var
        num_selected_events will be -1 for ALL_EVENTS. For note ON's only, the
        var will be the number of selected events. If 0 then only one pass is
        needed. If > 0 then two passes are needed, one for unselected (first), and one for
        selected (last).
    */
    int num_selected_events = ALL_EVENTS;
    int selection_type = num_selected_events;

    if (m_status == EVENT_NOTE_ON)
    {
        num_selected_events = m_sequence->get_num_selected_events(m_status, m_cc);

        /* For first pass - if any selected,  selection_type = UNSELECTED_EVENTS.
           For second pass will be set to num_selected_events*/
        if (num_selected_events > 0) selection_type = UNSELECTED_EVENTS;
    }

    int start_tick = m_hscroll - c_keys_width * m_zoom;
    int end_tick = start_tick + width * m_zoom;
    if (m_sequence->get_length() < end_tick) end_tick = m_sequence->get_length();

    SECOND_PASS_NOTE_ON: // yes this is a goto... yikes!!!!

    m_sequence->reset_draw_marker();

    while (m_sequence->get_next_event(m_status, m_cc, &tick, &d0, &d1, &selected, selection_type) == true)
    {
        if (tick >= start_tick && tick <= end_tick)
        {

            if (selected) cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_event);
            else cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_event);

            /* turn into screen corrids */
            event_x = (tick - m_hscroll) / m_zoom + c_keys_width + c_event_width / 2 + 2;

            /* generate the value */
            event_height = d1;

            if (m_status == EVENT_PROGRAM_CHANGE || m_status == EVENT_CHANNEL_PRESSURE)
            {
                event_height = d0;
            }


            int y = (1 - event_height / 127.0) * (c_data_y1 - c_data_y0) + c_data_y0;


            /* draw vert lines */
            cr->move_to(event_x, y);
            cr->line_to(event_x, height - c_data_text_height - c_dataroll_padding - c_data_handle_radius - 1);
            cr->stroke();

            /* draw handle */
            cr->arc(event_x, y, 2, 0, 2 * G_PI);
            cr->fill();
            if (selected) cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_handle);
            else cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_handle);
            cr->arc(event_x, y, 6, 0, 2 * G_PI);
            cr->fill();

            /* draw numbers */

            auto t = create_pango_layout(to_string(event_height));
            t->set_font_description(font);
            t->set_justify(Pango::ALIGN_CENTER);
            t->set_width(0);
            t->set_wrap(Pango::WRAP_CHAR);
            t->get_pixel_size(text_width, text_height);

            cr->set_source_rgb(c_color_data_background.r, c_color_data_background.g, c_color_data_background.b);
            cr->rectangle(event_x - c_data_text_width / 2, height - c_data_text_height - c_dataroll_padding, c_data_text_width, c_data_text_height + c_dataroll_padding);
            cr->fill();

            if (selected) cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_event);
            else cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_event);
            cr->move_to(event_x - text_width / 2 - 0.5, height - c_data_text_height - c_dataroll_padding);
            t->show_in_cairo_context(cr);

        }

    }

    if(selection_type == UNSELECTED_EVENTS)
    {
        selection_type = num_selected_events;
        goto SECOND_PASS_NOTE_ON; // this is NOT spaghetti code... it's very clear what is going on!!!
    }

    cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_bottom_line);
    cr->set_line_width(1.0);
    cr->move_to(0, c_data_y1 + 0.5);
    cr->line_to(width, c_data_y1 + 0.5);
    cr->stroke();

    if (m_dragging)
    {
        // int x,y,w,h;
        cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_event);

        // xy_to_rect (m_drop_x, m_drop_y, m_current_x, m_current_y, &x, &y, &w, &h);
        //
        // x -= m_hscroll / m_zoom;
        int y0 = m_current_y / 127.0 * (c_data_y1 - c_data_y0) + c_data_y0;
        int y1 = m_drop_y / 127.0 * (c_data_y1 - c_data_y0) + c_data_y0;

        cr->move_to(m_current_x - m_hscroll / m_zoom, y0);
        cr->line_to(m_drop_x - m_hscroll / m_zoom, y1);
        cr->stroke();
    }

    return true;
}


void
DataRoll::set_zoom(int zoom)
{
    if (zoom < c_min_zoom) zoom = c_min_zoom;
    else if (zoom > c_max_zoom) zoom = c_max_zoom;
    m_zoom = zoom;
    queue_draw();
}

/* takes screen corrdinates, give us notes and ticks */
void
DataRoll::convert_x(int a_x, long *a_tick)
{
    *a_tick = (a_x - c_keys_width - (2 + c_event_width / 2)) * m_zoom;
}


// Takes two points, returns a Xwin rectangle
void
DataRoll::xy_to_rect(int a_x1, int a_y1, int a_x2, int a_y2, int *a_x, int *a_y, int *a_w, int *a_h )
{
    /* checks mins / maxes..  the fills in x,y
       and width and height */

    if (a_x1 < a_x2) {
    	*a_x = a_x1;
    	*a_w = a_x2 - a_x1;
    } else {
    	*a_x = a_x2;
    	*a_w = a_x1 - a_x2;
    }

    if (a_y1 < a_y2) {
    	*a_y = a_y1;
    	*a_h = a_y2 - a_y1;
    } else {
    	*a_y = a_y2;
    	*a_h = a_y1 - a_y2;
    }
}


bool
DataRoll::on_button_press_event(GdkEventButton* event)
{

    /* set values for line */
    m_current_x = m_drop_x = (int) event->x + m_hscroll / m_zoom + 0;
    m_current_y = m_drop_y = (int) 1.0 * (event->y - c_data_y0) / (c_data_y1 - c_data_y0) * 127;

    /* if they select the handle */
    long tick_s, tick_f;

    convert_x(m_drop_x - c_data_handle_radius, &tick_s);
    convert_x(m_drop_x + c_data_handle_radius, &tick_f);

    m_drag_handle = m_sequence->select_event_handle(tick_s, tick_f, m_status, m_cc, 127 - m_drop_y, c_data_handle_radius);

    if (m_drag_handle && !m_sequence->get_hold_undo()) m_sequence->push_undo(); // if they used line draw but did not leave...

    m_dragging = !m_drag_handle;

    return true;
}

bool
DataRoll::on_motion_notify_event(GdkEventMotion* event)
{

    if (m_drag_handle)
    {
        m_current_y = (int) 1.0 * (event->y - c_data_y0) / (c_data_y1 - c_data_y0) * 127;

        if (m_current_y < 0) m_current_y = 0;
        if (m_current_y > 127 ) m_current_y = 127;

        m_sequence->adjust_data_handle(m_status, 127 - m_current_y);
    }

    if (m_dragging)
    {
        m_current_x = (int) event->x + m_hscroll / m_zoom;
        m_current_y = (int) 1.0 * (event->y - c_data_y0) / (c_data_y1 - c_data_y0) * 127;

        long tick_s, tick_f;

        int adj_x_min, adj_x_max,
            adj_y_min, adj_y_max;

        if (m_current_x < m_drop_x)
        {
            adj_x_min = m_current_x;
            adj_y_min = m_current_y;
            adj_x_max = m_drop_x;
            adj_y_max = m_drop_y;
        }
        else
        {
            adj_x_max = m_current_x;
            adj_y_max = m_current_y;
            adj_x_min = m_drop_x;
            adj_y_min = m_drop_y;
        }

        convert_x(adj_x_min, &tick_s);
        convert_x(adj_x_max, &tick_f);

        m_sequence->change_event_data_range(tick_s, tick_f, m_status, m_cc, 127 - adj_y_min, 127 - adj_y_max );

    }

    return false;
}






bool
DataRoll::on_button_release_event(GdkEventButton* event)
{
    m_current_x = (int) event->x + m_hscroll / m_zoom;
    m_current_y = (int) 1.0 * (event->y - c_data_y0) / (c_data_y1 - c_data_y0) * 127;

    if (m_dragging)
    {
        long tick_s, tick_f;

        if (m_current_x < m_drop_x)
        {
            swap(m_current_x, m_drop_x);
            swap(m_current_y, m_drop_y);
        }

        convert_x(m_drop_x, &tick_s);
        convert_x(m_current_x, &tick_f);

        m_sequence->change_event_data_range(tick_s, tick_f,m_status, m_cc, 127 - m_drop_y, 127 - m_current_y);

        /* convert x,y to ticks, then set events in range */
        m_dragging = false;
    }

    if (m_drag_handle)
    {
        m_drag_handle = false;
        m_sequence->unselect();
        m_sequence->set_dirty();
    }

    if (m_sequence->get_hold_undo())
    {
        m_sequence->push_undo(true);
        m_sequence->set_hold_undo(false);
    }


    return true;
}


bool
DataRoll::on_scroll_event(GdkEventScroll* event)
{

    if (signal_scroll.emit(event)) return true;

    if (event->direction == GDK_SCROLL_UP) m_sequence->increment_selected(m_status, m_cc);
    if (event->direction == GDK_SCROLL_DOWN) m_sequence->decrement_selected(m_status, m_cc);

    return false;
}


void
DataRoll::set_data_type(unsigned char status, unsigned char control = 0)
{
    m_status = status;
    m_cc = control;
}
