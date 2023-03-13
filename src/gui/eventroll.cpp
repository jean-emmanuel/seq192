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

#include "eventroll.h"
#include "styles.h"

EventRoll::EventRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_hscroll = 0;
    m_zoom = c_default_zoom;
    m_snap = c_default_snap;
    m_snap_active = true;
    m_snap_bypass = false;

    m_status = EVENT_NOTE_ON;

    m_adding = false;
    m_selecting = false;
    m_moving    = false;
    m_moving_init = false;
    m_growing   = false;
    m_painting  = false;
    m_paste     = false;
    m_is_drag_pasting = false;
    m_is_drag_pasting_start = false;
    m_justselected_one = false;

    m_draw_background_queued = false;

    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );
    queue_draw_background();

    add_events(Gdk::POINTER_MOTION_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::POINTER_MOTION_MASK |
                Gdk::ENTER_NOTIFY_MASK |
                Gdk::LEAVE_NOTIFY_MASK |
                Gdk::SCROLL_MASK
 );
}

EventRoll::~EventRoll()
{
}

void
EventRoll::queue_draw_background()
{
    m_draw_background_queued = true;
}

void
EventRoll::draw_background()
{

    m_draw_background_queued = false;

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(0, -1, width + 1, height + 2);
    cr->paint_with_alpha(1.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

    // Horizontal lines

    cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_separator);
    cr->set_line_width(1.0);
    cr->move_to(0, 0.5);
    cr->line_to(width, 0.5);
    cr->stroke();
    cr->move_to(0, height - 0.5);
    cr->line_to(width, height - 0.5);
    cr->stroke();

    // Vertical lines

    int measures_per_line = 1;

    int ticks_per_measure =  m_sequence->get_bpm() * (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_beat =  (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_step = 3 * m_zoom;
    int ticks_per_m_line =  ticks_per_measure * measures_per_line;
    int start_tick = m_hscroll - (m_hscroll % ticks_per_step);
    int end_tick = start_tick + width * m_zoom;
    if (m_sequence->get_length() < end_tick) end_tick = m_sequence->get_length();
    int last_snap = 0;

    for (int i=start_tick; i<=end_tick; i += ticks_per_step)
    {
        int base_line = (i - m_hscroll) / m_zoom;
        bool draw = true;

        if (i % ticks_per_m_line == 0)
        {
            cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_measure);
        }
        else if (i % ticks_per_beat == 0)
        {
            cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_beat);
        }
        else if (i % m_snap <= last_snap) {
            cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_snap);
            base_line -= (i - m_snap * (i / m_snap)) / m_zoom;
        }
        else draw = false;

        last_snap = i % m_snap;

        if (draw) {
            cr->move_to(base_line + 0.5, 0);
            cr->line_to(base_line + 0.5, height);
            cr->stroke();
        }
    }

    // Events
    long tick;
    int x;
    unsigned char d0,d1;
    bool selected;

    m_sequence->reset_draw_list();

    while (m_sequence->get_next_event(m_status, m_cc, &tick, &d0, &d1, &selected) == true)
    {
        if (tick >= start_tick && tick <= end_tick)
        {

            /* turn into screen corrids */
            x = (tick - m_hscroll) / m_zoom + 2;

           if (selected)
           {
               cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_event);
           }
           else
           {
               cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_event);
           }

           cr->rectangle(x, 2, c_event_width, c_eventroll_height - 3);
           cr->fill();
        }
    }
}

bool
EventRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        queue_draw_background();
    }

    if (m_draw_background_queued) draw_background();

    // draw background
    cr->set_source(m_surface, 0.0, 0.0);
    cr->paint();

    cr->set_line_width(1.0);

    // mouse edition
    int x;
    int w;
    int y = 1;
    int h = c_eventroll_height;
    cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_lasso_stroke);
    if (m_selecting)
    {

    	x_to_w(m_drop_x, m_current_x, &x,&w);
        if (w > 0)
        {
            x -= m_hscroll / m_zoom;


            cr->set_source_rgba(c_color_lasso.r, c_color_lasso.g, c_color_lasso.b, c_alpha_lasso_fill);
            cr->rectangle(x, y - 1, w, h + 1);
            cr->fill();
            cr->set_source_rgba(c_color_lasso.r, c_color_lasso.g, c_color_lasso.b, c_alpha_lasso_stroke);
            cr->rectangle(x + 0.5, y -0.5, w, h );
            cr->stroke();
        }

    }

    if (m_moving || m_paste)
    {

    	int delta_x = m_current_x - m_drop_x;

    	x = m_selected.x + delta_x;
        x -= m_hscroll / m_zoom;

        cr->rectangle(x + 0.5, y - 0.5, m_selected.width - 1, h);
        cr->stroke();
    }

    return true;
}

void
EventRoll::draw_update()
{
    if (m_draw_background_queued || m_selecting || m_moving || m_paste) {
        queue_draw();
    }
}

void
EventRoll::set_zoom(double zoom)
{
    if (zoom < c_min_zoom) zoom = c_min_zoom;
    else if (zoom > c_max_zoom) zoom = c_max_zoom;
    m_zoom = zoom;
    queue_draw_background();
}

void
EventRoll::set_snap(int snap)
{
    m_snap = snap;
}

void
EventRoll::set_snap_bypass(bool bypass)
{
    m_snap_bypass = bypass;
    m_current_x = m_last_x;
    if (m_moving || m_paste) snap_x(&m_current_x);
}

void
EventRoll::set_adding(bool adding)
{
    m_adding = adding;
}

/* takes screen corrdinates, give us notes and ticks */
void
EventRoll::convert_x(int a_x, long *a_tick)
{
    *a_tick = a_x * m_zoom;
}


/* notes and ticks to screen corridinates */
void
EventRoll::convert_t(long a_ticks, int *a_x)
{
    *a_x = a_ticks / m_zoom;
}

/* performs a 'snap' on y */
void
EventRoll::snap_y(int *a_y)
{
    *a_y = *a_y - (*a_y % c_key_height);
}

/* performs a 'snap' on x */
void
EventRoll::snap_x(int *a_x)
{
    //snap = number pulses to snap to
    //m_zoom = number of pulses per pixel
    //so snap / m_zoom  = number pixels to snap to
    int snap = m_snap_active && !m_snap_bypass ? m_snap : c_disabled_snap;
    int mod = (snap / m_zoom);
    if (mod <= 0) mod = 1;
    *a_x = *a_x - (*a_x % mod);
}

/* checks mins / maxes..  the fills in x,y
   and width and height */
void
EventRoll::x_to_w(int a_x1, int a_x2, int *a_x, int *a_w)
{
    if (a_x1 < a_x2){
    	*a_x = a_x1;
    	*a_w = a_x2 - a_x1;
    } else {
    	*a_x = a_x2;
    	*a_w = a_x1 - a_x2;
    }
}

void
EventRoll::drop_event(long a_tick)
{

    unsigned char status = m_status;
    unsigned char d0 = m_cc;
    unsigned char d1 = 0x40;

    if (m_status == EVENT_PROGRAM_CHANGE) d0 = 0; /* d0 == new patch */
    if (m_status == EVENT_CHANNEL_PRESSURE) d0 = 0x40; /* d0 == pressure */
    if (m_status == EVENT_PITCH_WHEEL) d0 = 0;

    m_sequence->add_event(a_tick, status, d0, d1, true);
}


void
EventRoll::start_paste()
{
     long tick_s;
     long tick_f;
     int note_h;
     int note_l;
     int x, w;

     snap_x(&m_current_x);
     snap_y(&m_current_x);

     m_drop_x = m_current_x;
     m_drop_y = m_current_y;

     m_paste = true;

     /* get the box that selected elements are in */
     m_sequence->get_clipboard_box(&tick_s, &note_h, &tick_f, &note_l);

     /* convert box to X,Y values */
     convert_t(tick_s, &x);
     convert_t(tick_f, &w);

     /* w is actually corrids now, so we have to change */
     w = w-x;

     /* set the m_selected rectangle to hold the
	x,y,w,h of our selected events */

     m_selected.x = x;
     m_selected.width = w;

     /* adjust for clipboard being shifted to tick 0 */
     m_selected.x  += m_drop_x;
}


bool
EventRoll::on_enter_notify_event(GdkEventCrossing* event)
{
    signal_hover.emit((string)"eventroll");

    return true;
}

bool
EventRoll::on_leave_notify_event(GdkEventCrossing* event)
{
    signal_hover.emit((string)"");

    return true;
}

bool
EventRoll::on_button_press_event(GdkEventButton* event)
{
    signal_click.emit((string)"eventroll");

    int x,w,numsel;

    long tick_s;
    long tick_w;
    long tick_f;

    convert_x(c_event_width + 4, &tick_w);

    /* set values for dragging */
    m_drop_x = m_current_x = (int) event->x + m_hscroll / m_zoom - 1;

    if (m_paste){

        snap_x(&m_current_x);
        convert_x(m_current_x, &tick_s);
        m_paste = false;
        m_sequence->push_undo();
        m_sequence->paste_selected(tick_s, 0);

    }
    else
    {
        /*      left mouse button     */
        if (event->button == 1)
        {

            /* turn x,y in to tick/note */
            convert_x(m_drop_x, &tick_s);

            if (m_adding)
            {
                m_painting = true;

                snap_x(&m_drop_x);
                /* turn x,y in to tick/note */
                convert_x(m_drop_x, &tick_s);
                /* add note, length = little less than snap */

                if (!m_sequence->select_events(tick_s, tick_s, tick_w, m_status, m_cc, sequence::e_would_select))
                {
                    m_sequence->push_undo();
                    drop_event(tick_s);
                }

            }
            else /* selecting */
            {
                if (!m_sequence->select_events(tick_s, tick_s, tick_w, m_status, m_cc, sequence::e_is_selected))
                {
                    if (!(event->state & GDK_CONTROL_MASK) && !(event->state & GDK_SHIFT_MASK))
                    {
                        m_sequence->unselect();
                    }

                    numsel = m_sequence->select_events(tick_s, tick_s, tick_w, m_status, m_cc, sequence::e_select_one);

                    /* if we didnt select anyhing (user clicked empty space)
                       unselect all notes, and start selecting */

                    /* none selected, start selection box */
                    if (numsel == 0)
                    {
                        m_selecting = true;
                    }
                    else
                    {
                        /// needs update
                    }
                }

                if (m_sequence->select_events(tick_s, tick_s, tick_w, m_status, m_cc, sequence::e_is_selected))
                {

                    m_moving_init = true;
                    int note;

                    /* get the box that selected elements are in */
                    m_sequence->get_selected_box(&tick_s, &note, &tick_f, &note);

                    if (event->state & GDK_SHIFT_MASK) {
                        m_sequence->select_events(tick_s, tick_f, tick_w, m_status, m_cc, sequence::e_select);
                    }

                    tick_f += tick_w;

                    /* convert box to X,Y values */
                    convert_t(tick_s, &x);
                    /* convert box to X,Y values */
                    convert_t(tick_s, &x);
                    convert_t(tick_f, &w);

                    /* w is actually corrids now, so we have to change */
                    w = w-x;

                    /* set the m_selected rectangle to hold the
                       x,y,w,h of our selected events */

                    m_selected.x = x;
                    m_selected.width = w;

                    /* save offset that we get from the snap above */
                    int adjusted_selected_x = m_selected.x;
                    snap_x(&adjusted_selected_x);
                    m_move_snap_offset_x = (m_selected.x - adjusted_selected_x);

                    /* align selection for drawing */
                    snap_x(&m_selected.x);
                    snap_x(&m_current_x);
                    snap_x(&m_drop_x);

                }

            }

        }

        if (event->button == 3)
        {
            signal_adding.emit(true);
        }
    }


    return true;
}

bool
EventRoll::on_motion_notify_event(GdkEventMotion* event)
{

    long tick = 0;

    m_current_x = m_last_x = (int) event->x  + m_hscroll / m_zoom - 1;

    if (m_moving_init)
    {
        m_moving_init = false;
        m_moving = true;
    }

    if (m_selecting || m_moving || m_paste)
    {

        if (m_moving || m_paste) snap_x(&m_current_x);
    }


    if (m_painting)
    {
        long tick_w;

        snap_x(&m_current_x);
        convert_x(m_current_x, &tick);
        convert_x(c_event_width + 4, &tick_w);

        if (!m_sequence->select_events(tick, tick, tick_w, m_status, m_cc, sequence::e_would_select))
        {
            m_sequence->push_undo();
            drop_event(tick);
        }
    }

    return false;
}






bool
EventRoll::on_button_release_event(GdkEventButton* event)
{

    long tick_s;
    long tick_f;

    int x,w;

    m_current_x = (int) event->x + m_hscroll / m_zoom - 1;

    if (m_moving) snap_x(&m_current_x);

    int delta_x = m_current_x - m_drop_x;

    long delta_tick;

    if (event->button == 1)
    {

        if (m_selecting)
        {
            x_to_w(m_drop_x, m_current_x, &x, &w);

            convert_x(x,   &tick_s);
            convert_x(x+w, &tick_f);

            m_sequence->select_events(tick_s, tick_f, 0, m_status, m_cc, sequence::e_select);
        }

        if (m_moving)
        {

            /* adjust for snap */
            delta_x -= m_move_snap_offset_x;

            /* convert deltas into screen corridinates */
            convert_x(delta_x, &delta_tick);

            /* not really notes, but still moves events */
            m_sequence->push_undo();
            m_sequence->move_selected_notes(delta_tick, 0);
        }

    }

    if (event->button == 3)
    {

        signal_adding.emit(false);

    }

    /* turn off */
    m_selecting = false;
    m_moving = false;
    m_growing = false;
    m_moving_init = false;
    m_painting = false;

    m_sequence->unpaint_all();

    return true;
}

bool
EventRoll::on_scroll_event(GdkEventScroll* event)
{

    if (!(event->state & GDK_CONTROL_MASK)) event->state = GDK_SHIFT_MASK;

    if (signal_scroll.emit(event)) return true;

    return false;
}

void
EventRoll::set_hscroll(int s) {
    if (s != m_hscroll) {
        m_hscroll = s;
        queue_draw_background();
    }
}

void
EventRoll::set_data_type(unsigned char status, unsigned char control = 0)
{
    m_status = status;
    m_cc = control;
    queue_draw_background();
}
