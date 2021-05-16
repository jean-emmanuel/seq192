//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq24 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq24; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------
#include "event.h"
#include "seqdata.h"
#include "font.h"


seqdata::seqdata(sequence *a_seq, int a_zoom,  Gtk::Adjustment   *a_hadjust): DrawingArea()
{
    m_seq = a_seq;
    m_zoom = a_zoom;

    add_events( Gdk::BUTTON_PRESS_MASK |
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::POINTER_MOTION_MASK |
		Gdk::LEAVE_NOTIFY_MASK |
                Gdk::SCROLL_MASK );

    // in the construor you can only allocate colors,
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    //m_text_font_5_7 = Gdk_Font( c_font_5_7 );

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );
    m_blue  = Gdk::Color( "dark blue" );
    m_red   = Gdk::Color( "orange" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    colormap->alloc_color( m_blue );
    colormap->alloc_color( m_red );

    m_dragging = false;
    m_drag_handle = false;

    set_flags(Gtk::CAN_FOCUS );
    set_double_buffered( false );

    set_size_request( 10,  c_dataarea_y );

    m_hadjust = a_hadjust;

    m_scroll_offset_ticks = 0;
    m_scroll_offset_x = 0;
}

void
seqdata::update_sizes()
{
    if( is_realized() ) {
        /* create pixmaps with window dimentions */

        m_pixmap = Gdk::Pixmap::create( m_window,
                                        m_window_x,
                                        m_window_y, -1  );
        update_pixmap();
        queue_draw();
    }

}

void
seqdata::reset()
{
    update_sizes();
    update_pixmap();
    queue_draw();
}


void
seqdata::redraw()
{
    update_pixmap();
    queue_draw();
}


void
seqdata::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &seqdata::change_horz ));

    for( int i=0; i<c_dataarea_y; ++i ){

        m_numbers[i] = Gdk::Pixmap::create( m_window,
                                            6,
                                            30, -1  );

        m_gc->set_foreground( m_white );
        m_numbers[i]->draw_rectangle(m_gc,true,
                                     0,
                                     0,
                                     6,
                                     30 );

        char val[5];
        snprintf(val, sizeof(val), "%3d\n", i);
        char num[6];
        memset( num, 0, 6);
        num[0] = val[0];
        num[2] = val[1];
        num[4] = val[2];

        p_font_renderer->render_string_on_drawable(m_gc,
                                                   0,
                                                   0,
                                                   m_numbers[i], &num[0], font::BLACK );
        p_font_renderer->render_string_on_drawable(m_gc,
                                                   0,
                                                   8,
                                                   m_numbers[i], &num[2], font::BLACK );
        p_font_renderer->render_string_on_drawable(m_gc,
                                                   0,
                                                   16,
                                                   m_numbers[i], &num[4], font::BLACK );



    }

    update_sizes();

}

void
seqdata::set_zoom( int a_zoom )
{
    if ( m_zoom != a_zoom ){
        m_zoom = a_zoom;
        reset();
    }
}



void
seqdata::set_data_type( unsigned char a_status, unsigned char a_control = 0  )
{
    m_status = a_status;
    m_cc = a_control;

    this->redraw();
}


void
seqdata::update_pixmap()
{
    draw_events_on_pixmap();
}

void
seqdata::draw_events_on(  Glib::RefPtr<Gdk::Drawable> a_draw  )
{
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

    if ( m_status == EVENT_NOTE_ON)
    {
        num_selected_events = m_seq->get_num_selected_events(m_status, m_cc);

        /* For first pass - if any selected,  selection_type = UNSELECTED_EVENTS.
           For second pass will be set to num_selected_events*/
        if(num_selected_events > 0)
            selection_type = UNSELECTED_EVENTS;
    }

    int start_tick = m_scroll_offset_ticks ;
    int end_tick = (m_window_x * m_zoom) + m_scroll_offset_ticks;

    //printf( "draw_events_on\n" );

    m_gc->set_foreground( m_white );
    a_draw->draw_rectangle(m_gc,true,
                           0,
                           0,
                           m_window_x,
                           m_window_y );

    SECOND_PASS_NOTE_ON: // yes this is a goto... yikes!!!!

    m_seq->reset_draw_marker();

    while ( m_seq->get_next_event( m_status,
                                   m_cc,
                                   &tick, &d0, &d1,
                                   &selected, selection_type ) == true )
    {
        if ( tick >= start_tick && tick <= end_tick )
        {
            if(selected)
                m_gc->set_foreground( m_red );
            else
                m_gc->set_foreground( m_blue );

            /* turn into screen corrids */
            event_x = tick / m_zoom;

            /* generate the value */
            event_height = d1;

            if ( m_status == EVENT_PROGRAM_CHANGE ||
                    m_status == EVENT_CHANNEL_PRESSURE  )
            {

                event_height = d0;
            }

            m_gc->set_line_attributes( 2,
                                       Gdk::LINE_SOLID,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );

            /* draw vert lines */
            a_draw->draw_line(m_gc,
                              event_x -  m_scroll_offset_x + 1,
                              c_dataarea_y - event_height,
                              event_x -  m_scroll_offset_x + 1,
                              c_dataarea_y );

            /* draw handle */
            a_draw->draw_rectangle(m_gc,
                              true,
                              event_x -  m_scroll_offset_x - 3,
                              c_dataarea_y - event_height ,
                              c_data_handle_x,
                              c_data_handle_y);

            /* draw numbers */
            a_draw->draw_drawable(m_gc,
                                  m_numbers[event_height],
                                  0,0,
                                  event_x + 3 - m_scroll_offset_x,
                                  c_dataarea_y - 25,
                                  6,30);
        }
    }

    if(selection_type == UNSELECTED_EVENTS)
    {
        selection_type = num_selected_events;
        goto SECOND_PASS_NOTE_ON; // this is NOT spaghetti code... it's very clear what is going on!!!
    }
}




void
seqdata::draw_events_on_pixmap()
{
    draw_events_on( m_pixmap );
}

void
seqdata::draw_pixmap_on_window()
{
    queue_draw();
}


int
seqdata::idle_redraw()
{
    /* no flicker, redraw */
    if ( !m_dragging ){
	draw_events_on( m_window );
	draw_events_on( m_pixmap );
    }
	return true;
}

bool
seqdata::on_expose_event(GdkEventExpose* a_e)
{
    m_window->draw_drawable(m_gc,
                            m_pixmap,
                            a_e->area.x,
                            a_e->area.y,
                            a_e->area.x,
                            a_e->area.y,
                            a_e->area.width,
                            a_e->area.height );
    return true;
}

/* takes screen corrdinates, give us notes and ticks */
void
seqdata::convert_x( int a_x, long *a_tick )
{
    *a_tick = a_x * m_zoom;
}

bool
seqdata::on_scroll_event( GdkEventScroll* a_ev )
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();

    /* This scroll event only handles basic scrolling without any
     * modifier keys such as GDK_CONTROL_MASK or GDK_SHIFT_MASK */
    if ((a_ev->state & modifiers) != 0)
        return false;

    if (  a_ev->direction == GDK_SCROLL_UP ){
        m_seq->increment_selected( m_status, m_cc );
    }
    if (  a_ev->direction == GDK_SCROLL_DOWN ){
        m_seq->decrement_selected( m_status, m_cc );
    }

    update_pixmap();
    queue_draw();

    return true;
}

bool
seqdata::on_button_press_event(GdkEventButton* a_p0)
{
    if ( a_p0->type == GDK_BUTTON_PRESS )
    {
        /* set values for line */
        m_drop_x = (int) a_p0->x + m_scroll_offset_x;
        m_drop_y = (int) a_p0->y;

        /* if they select the handle */
        long tick_s, tick_f;

        convert_x( m_drop_x - 3, &tick_s );
        convert_x( m_drop_x + 3, &tick_f );

        m_drag_handle = m_seq->select_event_handle(tick_s, tick_f,
                                              m_status, m_cc,
                                              c_dataarea_y - m_drop_y +3);

        if(m_drag_handle)
            if(!m_seq->get_hold_undo()) // if they used line draw but did not leave...
                m_seq->push_undo();

        /* reset box that holds dirty redraw spot */
        m_old.x = 0;
        m_old.y = 0;
        m_old.width = 0;
        m_old.height = 0;

        m_dragging = !m_drag_handle;
    }

    return true;
}

bool
seqdata::on_button_release_event(GdkEventButton* a_p0)
{
    m_current_x = (int) a_p0->x + m_scroll_offset_x;
    m_current_y = (int) a_p0->y;

    if ( m_dragging )
    {
        long tick_s, tick_f;

        if ( m_current_x < m_drop_x )
        {
            swap( m_current_x, m_drop_x );
            swap( m_current_y, m_drop_y );
        }

        convert_x( m_drop_x, &tick_s );
        convert_x( m_current_x, &tick_f );

        m_seq->change_event_data_range( tick_s, tick_f,
                                        m_status,
                                        m_cc,
                                        c_dataarea_y - m_drop_y -1,
                                        c_dataarea_y - m_current_y-1 );

        /* convert x,y to ticks, then set events in range */
        m_dragging = false;
    }

    if(m_drag_handle)
    {
        m_drag_handle = false;
        m_seq->unselect();
        m_seq->set_dirty();
    }

    update_pixmap();
    queue_draw();
    return true;
}



// Takes two points, returns a Xwin rectangle
void
seqdata::xy_to_rect(  int a_x1,  int a_y1,
		      int a_x2,  int a_y2,
		      int *a_x,  int *a_y,
		      int *a_w,  int *a_h )
{
    /* checks mins / maxes..  the fills in x,y
       and width and height */

    if ( a_x1 < a_x2 ){
	*a_x = a_x1;
	*a_w = a_x2 - a_x1;
    } else {
	*a_x = a_x2;
	*a_w = a_x1 - a_x2;
    }

    if ( a_y1 < a_y2 ){
	*a_y = a_y1;
	*a_h = a_y2 - a_y1;
    } else {
	*a_y = a_y2;
	*a_h = a_y1 - a_y2;
    }
}

bool
seqdata::on_motion_notify_event(GdkEventMotion* a_p0)
{
    if(m_drag_handle)
    {
        m_current_y = (int) a_p0->y - 3;

        m_current_y = c_dataarea_y - m_current_y;

        if(m_current_y < 0 )
            m_current_y = 0;

        if(m_current_y > 127 )
            m_current_y = 127;

        m_seq->adjust_data_handle(m_status, m_current_y );

        update_pixmap();
        draw_events_on( m_window );
    }

    if ( m_dragging )
    {
        m_current_x = (int) a_p0->x + m_scroll_offset_x;
        m_current_y = (int) a_p0->y;

        long tick_s, tick_f;

        int adj_x_min, adj_x_max,
            adj_y_min, adj_y_max;

        if ( m_current_x < m_drop_x )
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

        convert_x( adj_x_min, &tick_s );
        convert_x( adj_x_max, &tick_f );

        m_seq->change_event_data_range( tick_s, tick_f,
                                        m_status,
                                        m_cc,
                                        c_dataarea_y - adj_y_min -1,
                                        c_dataarea_y - adj_y_max -1 );

        /* convert x,y to ticks, then set events in range */
        update_pixmap();

        draw_events_on( m_window );

        draw_line_on_window();
    }

    return true;
}


bool
seqdata::on_leave_notify_event(GdkEventCrossing* p0)
{
    // m_dragging = false;
    update_pixmap();
    queue_draw();
    return true;
}




void
seqdata::draw_line_on_window( void )
{
    int x,y,w,h;
    m_gc->set_foreground( m_black );
    m_gc->set_line_attributes( 1,
                               Gdk::LINE_SOLID,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );

   /* replace old */
    m_window->draw_drawable(m_gc,
                            m_pixmap,
                            m_old.x,
                            m_old.y,
                            m_old.x,
                            m_old.y,
                            m_old.width + 1,
                            m_old.height + 1 );

    xy_to_rect ( m_drop_x,
		 m_drop_y,
		 m_current_x,
		 m_current_y,
		 &x, &y,
		 &w, &h );

    x -= m_scroll_offset_x;

    m_old.x = x;
    m_old.y = y;
    m_old.width = w;
    m_old.height = h;

    m_gc->set_foreground(m_black);
    m_window->draw_line(m_gc,
                        m_current_x - m_scroll_offset_x,
                        m_current_y,
                        m_drop_x - m_scroll_offset_x,
                        m_drop_y );

}



void
seqdata::change_horz( )
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;

    update_pixmap();
    force_draw();
}


void
seqdata::on_size_allocate(Gtk::Allocation& a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();

    update_sizes();

}


void
seqdata::force_draw(void )
{
    m_window->draw_drawable(m_gc,
                            m_pixmap,
                            0,
                            0,
                            0,
                            0,
                            m_window_x,
                            m_window_y );
}
