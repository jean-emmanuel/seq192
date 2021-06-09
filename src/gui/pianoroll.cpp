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


#include "pianoroll.h"
#include "../core/globals.h"
#include "styles.h"

PianoRoll::PianoRoll(perform * p, sequence * seq, PianoKeys * pianokeys)
{
    m_perform = p;
    m_sequence = seq;
    m_pianokeys = pianokeys;

    m_hscroll = 0;
    m_zoom = c_default_zoom;
    m_snap = 192;
    m_snap_active = true;
    m_note_length = 192;

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

    add_events( Gdk::POINTER_MOTION_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::POINTER_MOTION_MASK |
                Gdk::ENTER_NOTIFY_MASK |
                Gdk::LEAVE_NOTIFY_MASK |
                Gdk::SCROLL_MASK
    );


}

PianoRoll::~PianoRoll()
{
}


bool
PianoRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // Horizontal lines

    cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_key);
    cr->set_line_width(1.0);
    int i;
    for (i = 0; i < c_num_keys; i++) {
        cr->move_to(0, i * c_key_height - 0.5);
        cr->line_to(width, i * c_key_height - 0.5);
        cr->stroke();
    }

    // Vertical lines

    int measures_per_line = 1;

    int ticks_per_measure =  m_sequence->get_bpm() * (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_beat =  (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_step = 3 * m_zoom;
    int ticks_per_m_line =  ticks_per_measure * measures_per_line;
    int start_tick = m_hscroll - (m_hscroll % ticks_per_step);
    int end_tick = m_sequence->get_length();
    int last_snap = 0;

    for (int i=start_tick; i<=end_tick; i += ticks_per_step)
    {
        int base_line = (i - m_hscroll) / m_zoom;

        if ( i % ticks_per_m_line == 0 )
        {
            cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_measure);
        }
        else if (i % ticks_per_beat == 0 )
        {
            cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_beat);
        }
        else
        {
            if (i % m_snap <= last_snap) {
                cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, c_alpha_grid_snap);
                base_line -= (i - m_snap * (i / m_snap)) / m_zoom;
            } else {
                cr->set_source_rgba(c_color_grid.r, c_color_grid.g, c_color_grid.b, 0);

            }
        }

        last_snap = i % m_snap;

        cr->move_to(base_line + 0.5, 0);
        cr->line_to(base_line + 0.5, height);
        cr->stroke();
    }



    // Events
    draw_type dt;
    long tick_s;
    long tick_f;
    int note;
    bool selected;
    int velocity;
    int note_x;
    int note_width;
    int note_y;
    int note_height = c_key_height - 3;

    m_sequence->reset_draw_marker();

    while ((dt = m_sequence->get_next_note_event( &tick_s, &tick_f, &note, &selected, &velocity )) != DRAW_FIN)
    {
        if ((tick_s >= start_tick && tick_s <= end_tick) || ((dt == DRAW_NORMAL_LINKED) && (tick_f >= start_tick && tick_f <= end_tick)))
        {

            if (selected)
            {
                cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_event);
            }
            else
            {
                cr->set_source_rgba(c_color_event.r, c_color_event.g, c_color_event.b, c_alpha_event);
            }

            note_x = tick_s / m_zoom;
            note_y = height - c_key_height * (note + 1) + 1;

            if (dt == DRAW_NORMAL_LINKED)
            {
                if (tick_f >= tick_s)
                {
                    note_width = (tick_f - tick_s) / m_zoom;
                    if ( note_width < 4 ) note_width = 4;
                }
                else
                {
                    note_width = (m_sequence->get_length() - tick_s) / m_zoom;
                }
            }
            else
            {
                note_width = 16 / m_zoom;
            }

            note_x -= m_hscroll / m_zoom;

            cr->rectangle(note_x + 2, note_y, note_width - 2, note_height + (note == 0 ? 1 : 0));

            if (tick_f < tick_s)
            {
                cr->rectangle(0 - m_hscroll / m_zoom, note_y, tick_f / m_zoom - 2, note_height + (note == 0 ? 1 : 0));
            }

            cr->fill();
        }
    }

    // mouse edition
    if (m_selecting || m_moving || m_paste ||  m_growing)
    {
        int x,y,w,h;
        cr->set_source_rgba(c_color_event_selected.r, c_color_event_selected.g, c_color_event_selected.b, c_alpha_lasso_stroke);
        if (m_selecting)
        {

            xy_to_rect(m_drop_x,  m_drop_y, m_current_x, m_current_y, &x, &y, &w, &h);

            if (w > 0)
            {
                x -= m_hscroll / m_zoom;

                cr->set_source_rgba(c_color_lasso.r, c_color_lasso.g, c_color_lasso.b, c_alpha_lasso_fill);
                cr->rectangle(x, y - 1, w, h + 1);
                cr->fill();
                cr->set_source_rgba(c_color_lasso.r, c_color_lasso.g, c_color_lasso.b, c_alpha_lasso_stroke);
                cr->rectangle(x + 0.5, y - 0.5, w, h);
                cr->stroke();
            }

        }
        if (m_moving || m_paste)
        {

            int delta_x = m_current_x - m_drop_x;
            int delta_y = m_current_y - m_drop_y;

            x = m_selected.x + delta_x;
            y = m_selected.y + delta_y;

            snap_y(&y);

            x -= m_hscroll / m_zoom;

            cr->rectangle(x + 0.5, y - 0.5, m_selected.width + 1, m_selected.height);
            cr->stroke();
        }

        if (m_growing){

            int delta_x = m_current_x - m_drop_x;
            int width = delta_x + m_selected.width;

            if (width < 1) width = 1;

            x = m_selected.x;
            y = m_selected.y;

            x -= m_hscroll / m_zoom;

            cr->rectangle(x + 0.5, y + 0.5, width + 1, m_selected.height);
            cr->stroke();

        }
    }


    // progress marker
    long tick = (m_sequence->get_last_tick() - m_hscroll) / m_zoom;
    if (tick != 0)
    {
        cr->set_line_width(1.0);
        cr->set_source_rgba(c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue(), 0.75);
        cr->move_to(tick + 0.5, 0);
        cr->line_to(tick + 0.5, height);
        cr->stroke();
    }

    return true;
}

void
PianoRoll::set_zoom(int zoom)
{
    if (zoom < c_min_zoom) zoom = c_min_zoom;
    else if (zoom > c_max_zoom) zoom = c_max_zoom;
    m_zoom = zoom;
    queue_draw();
}

void
PianoRoll::set_snap(int snap)
{
    m_snap = snap;
}

void
PianoRoll::set_note_length(int note_length)
{
    m_note_length = note_length;
}

void
PianoRoll::set_adding(bool adding)
{
    m_adding = adding;
    get_window()->set_cursor(Gdk::Cursor::create(get_window()->get_display(), adding ? "pencil" : "normal"));
}

void
PianoRoll::convert_xy(int x, int y, long *tick, int *note)
{
    *tick = x * m_zoom;
    *note = (c_keys_height - y - 2) / c_key_height;
}

void
PianoRoll::convert_tn( long ticks, int note, int *x, int *y)
{
    *x = ticks /  m_zoom;
    *y = c_keys_height - ((note + 1) * c_key_height) - 1;
}


/* checks mins / maxes..  the fills in x,y
   and width and height */
void
PianoRoll::xy_to_rect(int x1,  int y1, int x2,  int y2,  int *x,  int *y,  int *w,  int *h)
{
    if (x1 < x2){
    	*x = x1;
    	*w = x2 - x1;
    } else {
    	*x = x2;
    	*w = x1 - x2;
    }

    if (y1 < y2){
    	*y = y1;
    	*h = y2 - y1;
    } else {
    	*y = y2;
    	*h = y1 - y2;
    }
}


void
PianoRoll::convert_tn_box_to_rect(long tick_s, long tick_f, int note_h, int note_l, int *x, int *y,  int *w, int *h )
{
    int x1, y1, x2, y2;

    /* convert box to X,Y values */
    convert_tn(tick_s, note_h, &x1, &y1);
    convert_tn(tick_f, note_l, &x2, &y2);

    xy_to_rect(x1, y1, x2, y2, x, y, w, h);

    *h += c_key_height;
}

/* performs a 'snap' on y */
void
PianoRoll::snap_y(int *y)
{
    *y = *y - (*y % c_key_height);
}

/* performs a 'snap' on x */
void
PianoRoll::snap_x(int *x)
{
    //snap = number pulses to snap to
    //m_zoom = number of pulses per pixel
    //so snap / m_zoom  = number pixels to snap to
    int snap = m_snap_active ? m_snap : c_disabled_snap;
    int mod = (snap / m_zoom);
    if (mod <= 0) mod = 1;
    *x = *x - (*x % mod);
}


void
PianoRoll::start_paste()
{
     long tick_s;
     long tick_f;
     int note_h;
     int note_l;

     snap_x(&m_current_x);
     snap_y(&m_current_y);

     m_drop_x = m_current_x;
     m_drop_y = m_current_y;

     m_paste = true;

     /* get the box that selected elements are in */
     m_sequence->get_clipboard_box(&tick_s, &note_h,  &tick_f, &note_l);

     convert_tn_box_to_rect(tick_s, tick_f, note_h, note_l, &m_selected.x, &m_selected.y, &m_selected.width,  &m_selected.height);

     /* adjust for clipboard being shifted to tick 0 */
     m_selected.x += m_drop_x;
     m_selected.y += (m_drop_y - m_selected.y);
}




bool
PianoRoll::on_leave_notify_event(GdkEventCrossing* event)
{
    m_pianokeys->on_leave_notify_event(event);
    return true;
}

bool
PianoRoll::on_enter_notify_event(GdkEventCrossing* event)
{
    return true;
}

bool
PianoRoll::on_expose_event(GdkEventExpose* event)
{
    return true;
}
bool
PianoRoll::on_button_press_event(GdkEventButton* event)
{

    signal_focus.emit((string)"pianoroll");

    bool snap = !(event->state & GDK_MOD1_MASK);
    int numsel;
    long tick_s;
    long tick_f;
    int note_h;
    int note_l;
    int norm_x, norm_y, snapped_x, snapped_y;

    snapped_x = norm_x = (int) (event->x + m_hscroll / m_zoom) - 1;
    snapped_y = norm_y = (int) event->y;

    if (snap) snap_x( &snapped_x );
    snap_y( &snapped_y );

    m_current_y = m_drop_y = snapped_y;

    if (event->button == 3) {
        set_adding(true);
    }

    if (m_paste)
    {
        convert_xy(snapped_x, snapped_y, &tick_s, &note_h);
        m_paste = false;
        m_sequence->push_undo();
        m_sequence->paste_selected(tick_s, note_h);
    }

    else if (event->button == 1 || event->button == 2)
    {

        m_current_x = m_drop_x = norm_x;

        convert_xy(m_drop_x, m_drop_y, &tick_s, &note_h);

        if (m_adding)
        {
            /* start the paint job */
            m_painting = true;

            /* adding, snapped x */
            if (snap) m_current_x = m_drop_x = snapped_x;
            convert_xy(m_drop_x, m_drop_y, &tick_s, &note_h);

            // test if a note is already there
            // fake select, if so, no add
            if (!m_sequence->select_note_events(tick_s, note_h, tick_s, note_h, sequence::e_would_select))
            {
                /* add note, length = little less than snap */
                m_sequence->push_undo();
                m_sequence->add_note(tick_s, m_note_length - 1, note_h, true);
            }
        }
        else
        {

            if (!m_sequence->select_note_events(tick_s, note_h, tick_s, note_h, sequence::e_is_selected))
            {
                if (!(event->state & GDK_CONTROL_MASK))
                {
                    m_sequence->unselect();
                }

                /* on direct click select only one event */
                numsel = m_sequence->select_note_events(tick_s,note_h,tick_s,note_h, sequence::e_select_one);

                /* none selected, start selection box */
                if (numsel == 0)
                {
                    if (event->button == 1) m_selecting = true;
                }
            }
            if (m_sequence->select_note_events(tick_s, note_h, tick_s, note_h, sequence::e_is_selected))
            {
                // moving - left click only
                if (event->button == 1 && !(event->state & GDK_CONTROL_MASK))
                {
                    m_moving_init = true;

                    /* get the box that selected elements are in */
                    m_sequence->get_selected_box(&tick_s, &note_h, &tick_f, &note_l);

                    convert_tn_box_to_rect(tick_s, tick_f, note_h, note_l, &m_selected.x, &m_selected.y, &m_selected.width, &m_selected.height );

                    /* save offset that we get from the snap above */
                    int adjusted_selected_x = m_selected.x;
                    snap_x(&adjusted_selected_x );
                    m_move_snap_offset_x = m_selected.x - adjusted_selected_x;

                    /* align selection for drawing */
                    if (snap) snap_x(&m_selected.x);

                    m_current_x = m_drop_x = snapped_x;
                }
                // middle mouse button, or left-ctrl click (for 2button mice)
                else if (event->button == 2 || (event->button == 1 && (event->state & GDK_CONTROL_MASK)))
                {

                    m_growing = true;

                    // get the box that selected elements are in
                    m_sequence->get_selected_box(&tick_s, &note_h, &tick_f, &note_l);

                    convert_tn_box_to_rect(tick_s, tick_f, note_h, note_l, &m_selected.x, &m_selected.y, &m_selected.width, &m_selected.height);

                }
            }
        }

    }

    return true;
}

bool
PianoRoll::on_motion_notify_event(GdkEventMotion* event)
{

    m_current_x = (int) (event->x + m_hscroll / m_zoom) - 1;
    m_current_y = (int) event->y;

    bool snap = !(event->state & GDK_MOD1_MASK);
    int note;
    long tick;

    if (m_moving_init){
        m_moving_init = false;
        m_moving = true;
    }


    // snap_y(&m_current_y);
    convert_xy(0, m_current_y, &tick, &note);

    m_pianokeys->hint_key(note);

    if (m_selecting || m_moving || m_growing || m_paste){

        if (m_moving || m_paste){
            if (snap) snap_x(&m_current_x);
        }
        return true;

    }

    if (m_painting)
    {
        if (snap) snap_x(&m_current_x);
        convert_xy(m_current_x, m_current_y, &tick, &note);

        m_sequence->add_note(tick, m_note_length - 2, note, true);
        return true;
    }


    return false;
}






bool
PianoRoll::on_button_release_event(GdkEventButton* event)
{
    if (event->button == 3) {
        set_adding(false);
    }

    bool snap = !(event->state & GDK_MOD1_MASK);
    long tick_s;
    long tick_f;
    int note_h;
    int note_l;
    int x,y,w,h;

    m_current_x = (int) (event->x + m_hscroll / m_zoom) - 1;
    m_current_y = (int) event->y;

    snap_y (&m_current_y);

    if (snap && m_moving) snap_x(&m_current_x);

    int delta_x = m_current_x - m_drop_x;
    int delta_y = m_current_y - m_drop_y;

    long delta_tick;
    int delta_note;

    if (event->button == 1)
    {

        if (m_selecting)
        {
            xy_to_rect (m_drop_x, m_drop_y, m_current_x, m_current_y, &x, &y, &w, &h);

            convert_xy(x,     y, &tick_s, &note_h);
            convert_xy(x+w, y+h, &tick_f, &note_l);

            m_sequence->select_note_events(tick_s, note_h, tick_f, note_l, sequence::e_select);
        }

        if (m_moving)
        {
            /* adjust for snap */
            if (snap) delta_x -= m_move_snap_offset_x;

            /* convert deltas into screen corridinates */
            convert_xy(delta_x, delta_y, &delta_tick, &delta_note);

            /* since delta_note was from delta_y, it will be filpped
               (delta_y[0] = note[127], etc.,so we have to adjust */
            delta_note = delta_note - (c_num_keys-1);

            m_sequence->push_undo();
            m_sequence->move_selected_notes(delta_tick, delta_note);
        }

    }

    if (event->button == 2 || event->button == 1)
    {

        if (m_growing){

            /* convert deltas into screen corridinates */
            convert_xy(delta_x, delta_y, &delta_tick, &delta_note);
            m_sequence->push_undo();

            if (event->state & GDK_SHIFT_MASK)
            {
                m_sequence->stretch_selected(delta_tick);
            }
            else
            {
                m_sequence->grow_selected(delta_tick);
            }
        }
    }

    m_selecting = false;
    m_moving = false;
    m_growing = false;
    m_paste = false;
    m_moving_init = false;
    m_painting = false;

    m_sequence->unpaint_all();

    return true;
}

bool
PianoRoll::on_scroll_event(GdkEventScroll* event)
{

    if (signal_scroll.emit(event)) return true;

    m_pianokeys->hint_key(-1);

    return false;
}
