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
#include "seqevent.h"


seqevent::seqevent(sequence *a_seq,
                   int a_zoom,
                   int a_snap,
                   seqdata *a_seqdata_wid,
                   Gtk::Adjustment   *a_hadjust): DrawingArea() 
{

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );
    m_red  = Gdk::Color( "orange" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    colormap->alloc_color( m_red );

    
    m_seq =   a_seq;
    m_zoom = a_zoom;
    m_snap =  a_snap;
    m_seqdata_wid = a_seqdata_wid;

    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::POINTER_MOTION_MASK |
		Gdk::KEY_PRESS_MASK |
		Gdk::KEY_RELEASE_MASK |
		Gdk::FOCUS_CHANGE_MASK );

    m_selecting = false;
    m_moving    = false;
    m_moving_init = false;
    m_growing   = false;
    m_paste     = false;
    m_painting  = false;
    
    m_status = EVENT_NOTE_ON;

    set_size_request( 10, c_eventarea_y );


    m_hadjust = a_hadjust;
    
    m_scroll_offset_ticks = 0;
    m_scroll_offset_x = 0;

    set_double_buffered( false );

} 





void 
seqevent::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_flags( Gtk::CAN_FOCUS );

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &seqevent::change_horz ));

    update_sizes();
}



void
seqevent::change_horz( )
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;

    update_pixmap();
    force_draw();    
}


void
seqevent::on_size_allocate(Gtk::Allocation& a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );
    
    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
  
    update_sizes(); 
 
}



int 
seqevent::idle_redraw()
{
    draw_events_on( m_window );
    draw_events_on( m_pixmap );
    return true;
}

void 
seqevent::update_sizes()
{
    
    if( is_realized() ) {
        /* create pixmaps with window dimentions */

        //printf( "update_sizes() m_window_x[%d] m_window_y[%d]\n",
        //       m_window_x, m_window_y );
        
        m_pixmap = Gdk::Pixmap::create( m_window,
                                        m_window_x,
                                        m_window_y, -1 );
        
        /* and fill the background ( dotted lines n' such ) */
        update_pixmap();
        queue_draw();
    }
}

/* basically resets the whole widget as if it was realized again */
void 
seqevent::reset()
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;
    
    update_sizes();
    update_pixmap();
    draw_pixmap_on_window();
}


void 
seqevent::redraw()
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;
    
    update_pixmap();
    draw_pixmap_on_window();
}


/* updates background */
void 
seqevent::draw_background()
{
    //printf ("draw_background()\n" );
    
    /* clear background */
    m_gc->set_foreground(m_white);
    m_pixmap->draw_rectangle(m_gc,true,
                             0,
                             0, 
                             m_window_x, 
                             m_window_y );


   /*int measure_length_64ths =  m_seq->get_bpm() * 64 /
        m_seq->get_bw();*/
    
    //printf ( "measure_length_64ths[%d]\n", measure_length_64ths );
    
    //int measures_per_line = (256 / measure_length_64ths) / (32 / m_zoom);
    //if ( measures_per_line <= 0
   int measures_per_line = 1;
    
    //printf( "measures_per_line[%d]\n", measures_per_line );

    int ticks_per_measure =  m_seq->get_bpm() * (4 * c_ppqn) / m_seq->get_bw();
    int ticks_per_beat =  (4 * c_ppqn) / m_seq->get_bw();
    int ticks_per_step = 6 * m_zoom;
    int ticks_per_m_line =  ticks_per_measure * measures_per_line;
    int start_tick = m_scroll_offset_ticks - (m_scroll_offset_ticks % ticks_per_step );
    int end_tick = (m_window_x * m_zoom) + m_scroll_offset_ticks;

    //printf ( "ticks_per_step[%d] start_tick[%d] end_tick[%d]\n",
    //         ticks_per_step, start_tick, end_tick );

    m_gc->set_foreground(m_grey);
    
    for ( int i=start_tick; i<end_tick; i += ticks_per_step )
    {
        int base_line = i / m_zoom;

        if ( i % ticks_per_m_line == 0 ){
            
            /* solid line on every beat */
            m_gc->set_foreground(m_black);
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_SOLID,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
            
        } else if (i % ticks_per_beat == 0 ){
            
            m_gc->set_foreground(m_grey);
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_SOLID,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
            
        }
        
        
        else {
            
            m_gc->set_foreground(m_grey);
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_ON_OFF_DASH,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
            gint8 dash = 1;
            m_gc->set_dashes( 0, &dash, 1 );
        }

        m_pixmap->draw_line(m_gc,
                            base_line - m_scroll_offset_x,
                            0,
                            base_line - m_scroll_offset_x,
                            m_window_y);
    }
    

    /* reset line style */
    m_gc->set_line_attributes( 1,
                               Gdk::LINE_SOLID,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );

    m_gc->set_foreground(m_black);
    m_pixmap->draw_rectangle(m_gc,false,
                             -1,
                             0, 
                             m_window_x + 1, 
                             m_window_y - 1 );
   
}

/* sets zoom, resets */
void 
seqevent::set_zoom( int a_zoom )
{
    if ( m_zoom != a_zoom ){
        
        m_zoom = a_zoom;
        reset();
    }
}

/* simply sets the snap member */
void 
seqevent::set_snap( int a_snap )
{
    m_snap = a_snap;
}


void 
seqevent::set_data_type( unsigned char a_status, unsigned char a_control = 0 )
{
    m_status = a_status;
    m_cc = a_control;
    
    this->redraw();
}
   
 


/* draws background pixmap on main pixmap,
   then puts the events on */
void 
seqevent::update_pixmap()
{

    draw_background();
    draw_events_on_pixmap();

    m_seqdata_wid->update_pixmap();
    m_seqdata_wid->draw_pixmap_on_window();
    
}


void
seqevent::draw_events_on( Glib::RefPtr<Gdk::Drawable> a_draw )
{
    long tick;

    int x;

    unsigned char d0,d1;

    bool selected;


    /* draw boxes from sequence */
    m_gc->set_foreground( m_black );

    int start_tick = m_scroll_offset_ticks ;
    int end_tick = (m_window_x * m_zoom) + m_scroll_offset_ticks;
    
    m_seq->reset_draw_marker();
    while ( m_seq->get_next_event( m_status,
                                   m_cc,
                                   &tick, &d0, &d1, 
                                   &selected ) == true ){
        if ( (tick >= start_tick && tick <= end_tick )){
            
            
            /* turn into screen corrids */
            x = tick / m_zoom;
            
            m_gc->set_foreground(m_black);
            
            a_draw->draw_rectangle(m_gc,true,
                                   x -  m_scroll_offset_x,
                                   (c_eventarea_y - c_eventevent_y)/2, 
                                   c_eventevent_x, 
                                   c_eventevent_y );
            
            
            
            if ( selected )
                m_gc->set_foreground(m_red);
            else
                m_gc->set_foreground(m_white);
            
            a_draw->draw_rectangle(m_gc,true,
                                   x -  m_scroll_offset_x + 1,
                                   (c_eventarea_y - c_eventevent_y)/2 + 1, 
                                   c_eventevent_x - 3, 
                                   c_eventevent_y - 3 );
        }
    }
    
}

/* fills main pixmap with events */
void 
seqevent::draw_events_on_pixmap()
{
    draw_events_on( m_pixmap );
}

/* draws pixmap, tells event to do the same */
void 
seqevent::draw_pixmap_on_window()
{
    /* we changed something on this window, and chances are we
       need to update the event widget as well */
   
    queue_draw(); 

    // and update our velocity window
    //m_seqdata_wid->update_pixmap();
    //m_seqdata_wid->draw_pixmap_on_window();
    // RCB ??
}

/* checks mins / maxes..  the fills in x,y
   and width and height */
void 
seqevent::x_to_w( int a_x1, int a_x2,
		int *a_x, int *a_w  )
{
    if ( a_x1 < a_x2 ){
	*a_x = a_x1; 
	*a_w = a_x2 - a_x1;
    } else {
	*a_x = a_x2; 
	*a_w = a_x1 - a_x2;
    }
}

void 
seqevent::draw_selection_on_window()
{
    int x,w;

    int y = (c_eventarea_y - c_eventevent_y)/2;
    int h =  c_eventevent_y;  

    m_gc->set_line_attributes( 1,
                               Gdk::LINE_SOLID,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );
    
    /* replace old */
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            m_old.x,
                            y,
                            m_old.x,
                            y,
                            m_old.width + 1,
                            h + 1 );

    if ( m_selecting ){
	
	x_to_w( m_drop_x, m_current_x, &x,&w );

    x -= m_scroll_offset_x;
    
	m_old.x = x;
	m_old.width = w;

	m_gc->set_foreground(m_black);
	m_window->draw_rectangle(m_gc,false,
                                 x,
                                 y, 
                                 w, 
                                 h );
    }
    
    if ( m_moving || m_paste ){

	int delta_x = m_current_x - m_drop_x;

	x = m_selected.x + delta_x;
    x -= m_scroll_offset_x;

	m_gc->set_foreground(m_black);
	m_window->draw_rectangle(m_gc,false,
                                 x,
                                 y, 
                                 m_selected.width, 
                                 h );
	m_old.x = x;
	m_old.width = m_selected.width;
    }
}

bool 
seqevent::on_expose_event(GdkEventExpose* e)
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            e->area.x,
                            e->area.y,
                            e->area.x,
                            e->area.y,
                            e->area.width,
                            e->area.height );

    draw_selection_on_window();
    return true;
}

void
seqevent::force_draw(void )
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            0,
                            0,
                            0,
                            0,
                            m_window_x,
                            m_window_y );

    draw_selection_on_window();
}


void
seqevent::start_paste( )
{
     long tick_s;
     long tick_f;
     int note_h;
     int note_l;
     int x, w;

     snap_x( &m_current_x );
     snap_y( &m_current_x );

     m_drop_x = m_current_x;
     m_drop_y = m_current_y;

     m_paste = true;

     /* get the box that selected elements are in */
     m_seq->get_clipboard_box( &tick_s, &note_h, 
			       &tick_f, &note_l );

     /* convert box to X,Y values */
     convert_t( tick_s, &x );
     convert_t( tick_f, &w );

     /* w is actually corrids now, so we have to change */
     w = w-x; 

     /* set the m_selected rectangle to hold the
	x,y,w,h of our selected events */

     m_selected.x = x;                  
     m_selected.width=w;
     m_selected.y = (c_eventarea_y - c_eventevent_y)/2;
     m_selected.height = c_eventevent_y;  

     /* adjust for clipboard being shifted to tick 0 */
     m_selected.x  += m_drop_x;
}





/* takes screen corrdinates, give us notes and ticks */
void 
seqevent::convert_x( int a_x, long *a_tick )
{
    *a_tick = a_x * m_zoom; 
}


/* notes and ticks to screen corridinates */
void 
seqevent::convert_t( long a_ticks, int *a_x )
{
    *a_x = a_ticks /  m_zoom;
}





bool 
seqevent::on_button_press_event(GdkEventButton* a_ev)
{
    bool result;

    switch (global_interactionmethod)
    {
        case e_fruity_interaction:
             result = m_fruity_interaction.on_button_press_event(a_ev, *this);
        case e_seq24_interaction:
             result = m_seq24_interaction.on_button_press_event(a_ev, *this);
        default:
             result = false;
    }
    return result;
}

void
seqevent::drop_event( long a_tick )
{

    unsigned char status = m_status;
    unsigned char d0 = m_cc;
    unsigned char d1 = 0x40;

    if ( m_status == EVENT_AFTERTOUCH )
        d0 = 0;

    if ( m_status == EVENT_PROGRAM_CHANGE )
        d0 = 0; /* d0 == new patch */

    if ( m_status == EVENT_CHANNEL_PRESSURE )
        d0 = 0x40; /* d0 == pressure */

    if ( m_status == EVENT_PITCH_WHEEL )
        d0 = 0;

    m_seq->add_event( a_tick, 
            status,
            d0,
            d1, true );
}


bool 
seqevent::on_button_release_event(GdkEventButton* a_ev)
{
    bool result;

    switch (global_interactionmethod)
    {
        case e_fruity_interaction:
             result = m_fruity_interaction.on_button_release_event(a_ev, *this);
        case e_seq24_interaction:
             result = m_seq24_interaction.on_button_release_event(a_ev, *this);
        default:
             result = false;
    }
    return result;
}



bool
seqevent::on_motion_notify_event(GdkEventMotion* a_ev)
{
    bool result;

    switch (global_interactionmethod)
    {
        case e_fruity_interaction:
             result = m_fruity_interaction.on_motion_notify_event(a_ev, *this);
        case e_seq24_interaction:
             result = m_seq24_interaction.on_motion_notify_event(a_ev, *this);
        default:
             result = false;
    }
    return result;
}



/* performs a 'snap' on y */
void 
seqevent::snap_y( int *a_y )
{
    *a_y = *a_y - (*a_y % c_key_y);
}

/* performs a 'snap' on x */
void 
seqevent::snap_x( int *a_x )
{
    //snap = number pulses to snap to
    //m_zoom = number of pulses per pixel
    //so snap / m_zoom  = number pixels to snap to
    int mod = (m_snap / m_zoom);
    if ( mod <= 0 )
        mod = 1;
    
    *a_x = *a_x - (*a_x % mod );
  
}


bool 
seqevent::on_focus_in_event(GdkEventFocus*)
{
    set_flags(Gtk::HAS_FOCUS);
    return false;
}

bool 
seqevent::on_focus_out_event(GdkEventFocus*)
{
    unset_flags(Gtk::HAS_FOCUS);
    return false;
}

bool
seqevent::on_key_press_event(GdkEventKey* a_p0)
{
    bool ret = false;

    if ( a_p0->type == GDK_KEY_PRESS ){

        if ( a_p0->keyval ==  GDK_Delete || a_p0->keyval == GDK_BackSpace ){

            m_seq->push_undo();
            m_seq->mark_selected();
            m_seq->remove_marked();
            ret = true;
        }

        if ( a_p0->state & GDK_CONTROL_MASK ){

            /* cut */
            if ( a_p0->keyval == GDK_x || a_p0->keyval == GDK_X ){

                m_seq->copy_selected();
                m_seq->mark_selected();
                m_seq->remove_marked();

                ret = true;
            }
            /* copy */
            if ( a_p0->keyval == GDK_c || a_p0->keyval == GDK_C ){

                m_seq->copy_selected();
                ret = true;
            }
            /* paste */
            if ( a_p0->keyval == GDK_v || a_p0->keyval == GDK_V ){

                start_paste();
                ret = true;
            }
            /* Undo */
            if ( a_p0->keyval == GDK_z || a_p0->keyval == GDK_Z ){

                m_seq->pop_undo();
                ret = true;
            }
         }
    }

    if ( ret == true ){

        redraw();
        m_seq->set_dirty();
        return true;
    }
    else

        return false;
}





//////////////////////////
// interaction methods
//////////////////////////


void FruitySeqEventInput::updateMousePtr(seqevent& ths)
{
    // context sensitive mouse
    long tick_s, tick_w, tick_f, pos;
    ths.convert_x( ths.m_current_x, &tick_s );
    ths.convert_x( c_eventevent_x, &tick_w  );
    tick_f = tick_s + tick_w;
    if ( tick_s < 0 )
        tick_s = 0; // clamp to 0


    if (m_is_drag_pasting || ths.m_selecting || ths.m_moving || ths.m_paste)
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ));
    }
    else if (ths.m_seq->intersectEvents( tick_s, tick_f, ths.m_status, pos ))
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::CENTER_PTR ));
    }
    else
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::PENCIL ));
    }
}


bool FruitySeqEventInput::on_button_press_event(GdkEventButton* a_ev, seqevent& ths)
{
     int x,w,numsel;

    long tick_s;
    long tick_f;
    long tick_w;

    ths.convert_x( c_eventevent_x, &tick_w  );

    /* if it was a button press */

    /* set values for dragging */
    ths.m_drop_x = ths.m_current_x = (int) a_ev->x + ths.m_scroll_offset_x;

    /* reset box that holds dirty redraw spot */
    ths.m_old.x = 0;
    ths.m_old.y = 0;
    ths.m_old.width = 0;
    ths.m_old.height = 0;

    if ( ths.m_paste ){

        ths.snap_x( &ths.m_current_x );
        ths.convert_x( ths.m_current_x, &tick_s );
        ths.m_paste = false;
        ths.m_seq->push_undo();
        ths.m_seq->paste_selected( tick_s, 0 );

    } else {

        /*      left mouse button     */
        if ( a_ev->button == 1 )
        {
            /* turn x,y in to tick/note */
            ths.convert_x( ths.m_drop_x, &tick_s );

            /* shift back a few ticks */
            tick_f = tick_s + (ths.m_zoom);
            tick_s -= (tick_w);

            if ( tick_s < 0 )
                tick_s = 0;

            if ( ! ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_would_select ) &&
                 !(a_ev->state & GDK_CONTROL_MASK) )
            {
                ths.m_painting = true;

                ths.snap_x( &ths.m_drop_x );
                /* turn x,y in to tick/note */
                ths.convert_x( ths.m_drop_x, &tick_s );

                /* add note, length = little less than snap */
                if ( ! ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_would_select ))
                {
                    ths.m_seq->push_undo();
                    ths.drop_event( tick_s );
                }

            }
            else /* selecting */
            {
                if ( ! ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_is_selected ))
                {
                    // if clicking event...
                    if (ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_would_select ) )
                    {
                        if ( ! (a_ev->state & GDK_CONTROL_MASK) )
                           ths.m_seq->unselect();
                    }
                    // if clicking empty space ...
                    else
                    {
                        // ... unselect all if ctrl-shift not held
                        if (! ((a_ev->state & GDK_CONTROL_MASK) &&
                               (a_ev->state & GDK_SHIFT_MASK)) )
                           ths.m_seq->unselect();
                    }

                    /* on direct click select only one event */
                    numsel = ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status,
                            ths.m_cc, sequence::e_select_one );

                    // prevent deselect in button_release()
                    if (numsel)
                       m_justselected_one = true;

                    // if nothing selected, start the selection box
                    if (numsel == 0 && (a_ev->state & GDK_CONTROL_MASK))
                        ths.m_selecting = true;
                }

                // if event under cursor is selected
                if ( ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_is_selected ))
                {

                    // grab/move the note
                    if ( !(a_ev->state & GDK_CONTROL_MASK) )
                    {
                        ths.m_moving_init = true;
                        int note;

                        /* get the box that selected elements are in */
                        ths.m_seq->get_selected_box( &tick_s, &note,
                                &tick_f, &note );

                        tick_f += tick_w;

                        /* convert box to X,Y values */
                        ths.convert_t( tick_s, &x );
                        ths.convert_t( tick_f, &w );

                        /* w is actually corrids now, so we have to change */
                        w = w-x;

                        /* set the m_selected rectangle to hold the
                           x,y,w,h of our selected events */

                        ths.m_selected.x = x;
                        ths.m_selected.width=w;

                        ths.m_selected.y = (c_eventarea_y - c_eventevent_y)/2;
                        ths.m_selected.height = c_eventevent_y;


                        /* save offset that we get from the snap above */
                        int adjusted_selected_x = ths.m_selected.x;
                        ths.snap_x( &adjusted_selected_x );
                        ths.m_move_snap_offset_x = ( ths.m_selected.x - adjusted_selected_x);

                        /* align selection for drawing */
                        ths.snap_x( &ths.m_selected.x );
                        ths.snap_x( &ths.m_current_x );
                        ths.snap_x( &ths.m_drop_x );
                    }
                    // ctrl left click when stuff is already selected
                    else if ((a_ev->state & GDK_CONTROL_MASK) &&
                             ths.m_seq->select_events( tick_s, tick_f,
                                                  ths. m_status, ths.m_cc, sequence::e_is_selected ))
                    {
                        m_is_drag_pasting_start = true;
                    }

                }
            }

        } /* end if button == 1 */

        if ( a_ev->button == 3 )
        {
            /* turn x,y in to tick/note */
            ths.convert_x( ths.m_drop_x, &tick_s );

            /* shift back a few ticks */
            tick_f = tick_s + (ths.m_zoom);
            tick_s -= (tick_w);

            if ( tick_s < 0 )
                tick_s = 0;

            //erase event under cursor if there is one
            if (ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_would_select ))
            {
                /* remove only the note under the cursor,
                   leave the selection intact */
                ths.m_seq->push_undo();
                ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_remove_one );
                ths.redraw();
                ths.m_seq->set_dirty();
            }
            else /* selecting */
            {
                if  ( ! (a_ev->state & GDK_CONTROL_MASK) )
                    ths.m_seq->unselect();

                // nothing selected, start the selection box
                ths.m_selecting = true;
            }
        }
    }

    /* if they clicked, something changed */
    ths.update_pixmap();
    ths.draw_pixmap_on_window();

    updateMousePtr( ths );

    return true;
}

bool FruitySeqEventInput::on_button_release_event(GdkEventButton* a_ev, seqevent& ths)
{
    long tick_s;
    long tick_f;

    int x,w;
    int numsel;

    ths.grab_focus( );

    ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;;

    if ( ths.m_moving || m_is_drag_pasting )
        ths.snap_x( &ths.m_current_x );

    int delta_x = ths.m_current_x - ths.m_drop_x;

    long delta_tick;

    if ( a_ev->button == 1 ){

        int current_x = ths.m_current_x;
        long t_s, t_f;
        ths.snap_x( &current_x );
        ths.convert_x( current_x, &t_s );

        /* shift back a few ticks */
        t_f = t_s + (ths.m_zoom);
        if ( t_s < 0 )
            t_s = 0;

        // ctrl-left click button up for select/drag copy/paste
        // left click button up for ending a move of selected notes
        if ( m_is_drag_pasting )
        {
            m_is_drag_pasting = false;
            m_is_drag_pasting_start = false;

            /* convert deltas into screen corridinates */
            ths.m_paste = false;
            ths.m_seq->push_undo();
            ths.m_seq->paste_selected( t_s, 0 );

            //m_seq->unselect();
        }
        // ctrl-left click but without movement - select a note
        if (m_is_drag_pasting_start)
        {
            m_is_drag_pasting_start = false;

            // if ctrl-left click without movement and
            // if note under cursor is selected, and ctrl is held
            // and buttondown didn't just select one
            if (!m_justselected_one &&
                ths.m_seq->select_events( t_s, t_f,
                                          ths.m_status, ths.m_cc,
                                          sequence::e_is_selected ) &&
                (a_ev->state & GDK_CONTROL_MASK))
            {
                // deselect the event
                numsel = ths.m_seq->select_events( t_s, t_f,
                                                  ths.m_status, ths.m_cc,
                                                  sequence::e_deselect );
            }
        }
        m_justselected_one = false; // clear flag on left button up

        if ( ths.m_moving ){

            /* adjust for snap */
            delta_x -= ths.m_move_snap_offset_x;

            /* convert deltas into screen corridinates */
            ths.convert_x( delta_x, &delta_tick );

            /* not really notes, but still moves events */
            ths.m_seq->push_undo();
            ths.m_seq->move_selected_notes( delta_tick, 0 );
        }
    }

    if ( a_ev->button == 3 || a_ev->button == 1 ){

       if ( ths.m_selecting ){

            ths.x_to_w( ths.m_drop_x, ths.m_current_x, &x, &w );

            ths.convert_x( x,   &tick_s );
            ths.convert_x( x+w, &tick_f );

            numsel = ths.m_seq->select_events( tick_s, tick_f,
                                            ths.m_status,
                                            ths.m_cc, sequence::e_toggle_selection );
        }
    }

    /* turn off */
    ths.m_selecting = false;
    ths.m_moving = false;
    ths.m_growing = false;
    ths.m_moving_init = false;
    ths.m_painting = false;

    ths.m_seq->unpaint_all();

    /* if they clicked, something changed */
    ths.update_pixmap();
    ths.draw_pixmap_on_window();

    updateMousePtr(ths);

    return true;
}

bool FruitySeqEventInput::on_motion_notify_event(GdkEventMotion* a_ev, seqevent& ths)
{
    long tick = 0;
    ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;


    if ( ths.m_moving_init )
    {
        ths.m_moving_init = false;
        ths.m_moving = true;
    }

    // context sensitive mouse pointer...
    updateMousePtr(ths);

    // ctrl-left click drag on selected note(s) starts a copy/unselect/paste
    if ( m_is_drag_pasting_start )
    {
        ths.m_seq->copy_selected();
        ths.m_seq->unselect();
        ths.start_paste();

        m_is_drag_pasting_start = false;
        m_is_drag_pasting = true;
    }

    if ( ths.m_selecting || ths.m_moving || ths.m_paste  ){

        if ( ths.m_moving || ths.m_paste )
            ths.snap_x( &ths.m_current_x );

        ths.draw_selection_on_window();
    }


    if ( ths.m_painting )
    {
        ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;;
        ths.snap_x( &ths.m_current_x );
        ths.convert_x( ths.m_current_x, &tick );
        ths.drop_event( tick );
    }

    return true;
}


void
Seq24SeqEventInput::set_adding( bool a_adding, seqevent& ths )
{
    if ( a_adding )
    {
    	ths.get_window()->set_cursor(  Gdk::Cursor( Gdk::PENCIL ) );
    	m_adding = true;
    }
    else
    {
    	ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ) );
    	m_adding = false;
    }
}


bool Seq24SeqEventInput::on_button_press_event(GdkEventButton* a_ev, seqevent& ths)
{
    int x,w,numsel;

    long tick_s;
    long tick_f;
    long tick_w;

    ths.convert_x( c_eventevent_x, &tick_w  );

    /* if it was a button press */

    /* set values for dragging */
    ths.m_drop_x = ths.m_current_x = (int) a_ev->x + ths.m_scroll_offset_x;

    /* reset box that holds dirty redraw spot */
    ths.m_old.x = 0;
    ths.m_old.y = 0;
    ths.m_old.width = 0;
    ths.m_old.height = 0;

    if ( ths.m_paste ){

        ths.snap_x( &ths.m_current_x );
        ths.convert_x( ths.m_current_x, &tick_s );
        ths.m_paste = false;
        ths.m_seq->push_undo();
        ths.m_seq->paste_selected( tick_s, 0 );

    } else {

        /*      left mouse button     */
        if ( a_ev->button == 1 ){

            /* turn x,y in to tick/note */
            ths.convert_x( ths.m_drop_x, &tick_s );

            /* shift back a few ticks */
            tick_f = tick_s + (ths.m_zoom);
            tick_s -= (tick_w);

            if ( tick_s < 0 )
                tick_s = 0;

            if ( m_adding )
            {
                ths.m_painting = true;

                ths.snap_x( &ths.m_drop_x );
                /* turn x,y in to tick/note */
                ths.convert_x( ths.m_drop_x, &tick_s );
                /* add note, length = little less than snap */

                if ( ! ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_would_select ))
                {
                    ths.m_seq->push_undo();
                    ths.drop_event( tick_s );
                }

            }
            else /* selecting */
            {
                if ( ! ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_is_selected ))
                {
                    if ( ! (a_ev->state & GDK_CONTROL_MASK) )
                    {
                        ths.m_seq->unselect();
                    }

                    numsel = ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status,
                            ths.m_cc, sequence::e_select_one );

                    /* if we didnt select anyhing (user clicked empty space)
                       unselect all notes, and start selecting */

                    /* none selected, start selection box */
                    if ( numsel == 0 )
                    {
                        ths.m_selecting = true;
                    }
                    else
                    {
                        /// needs update
                    }
                }

                if ( ths.m_seq->select_events( tick_s, tick_f,
                            ths.m_status, ths.m_cc, sequence::e_is_selected ))
                {

                    ths.m_moving_init = true;
                    int note;

                    /* get the box that selected elements are in */
                    ths.m_seq->get_selected_box( &tick_s, &note,
                            &tick_f, &note );

                    tick_f += tick_w;

                    /* convert box to X,Y values */
                    ths.convert_t( tick_s, &x );
                    ths.convert_t( tick_f, &w );

                    /* w is actually corrids now, so we have to change */
                    w = w-x;

                    /* set the m_selected rectangle to hold the
                       x,y,w,h of our selected events */

                    ths.m_selected.x = x;
                    ths.m_selected.width=w;

                    ths.m_selected.y = (c_eventarea_y - c_eventevent_y)/2;
                    ths.m_selected.height = c_eventevent_y;


                    /* save offset that we get from the snap above */
                    int adjusted_selected_x = ths.m_selected.x;
                    ths.snap_x( &adjusted_selected_x );
                    ths.m_move_snap_offset_x = ( ths.m_selected.x - adjusted_selected_x);

                    /* align selection for drawing */
                    ths.snap_x( &ths.m_selected.x );
                    ths.snap_x( &ths.m_current_x );
                    ths.snap_x( &ths.m_drop_x );

                }
            }

        } /* end if button == 1 */

        if ( a_ev->button == 3 ){

            set_adding( true, ths );
        }
    }

    /* if they clicked, something changed */
    ths.update_pixmap();
    ths.draw_pixmap_on_window();

    return true;
}

bool Seq24SeqEventInput::on_button_release_event(GdkEventButton* a_ev, seqevent& ths)
{
    long tick_s;
    long tick_f;

    int x,w;
    int numsel;

    ths.grab_focus( );

    ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;;

    if ( ths.m_moving )
        ths.snap_x( &ths.m_current_x );

    int delta_x = ths.m_current_x - ths.m_drop_x;

    long delta_tick;

    if ( a_ev->button == 1 ){

        if ( ths.m_selecting ){

            ths.x_to_w( ths.m_drop_x, ths.m_current_x, &x, &w );

            ths.convert_x( x,   &tick_s );
            ths.convert_x( x+w, &tick_f );

            numsel = ths.m_seq->select_events( tick_s, tick_f,
                    ths.m_status,
                    ths.m_cc, sequence::e_select );
        }

        if ( ths.m_moving ){

            /* adjust for snap */
            delta_x -= ths.m_move_snap_offset_x;

            /* convert deltas into screen corridinates */
            ths.convert_x( delta_x, &delta_tick );

            /* not really notes, but still moves events */
            ths.m_seq->push_undo();
            ths.m_seq->move_selected_notes( delta_tick, 0 );
        }

        set_adding( m_adding, ths );
    }

    if ( a_ev->button == 3 ){

        set_adding( false, ths );

    }

    /* turn off */
    ths.m_selecting = false;
    ths.m_moving = false;
    ths.m_growing = false;
    ths.m_moving_init = false;
    ths.m_painting = false;

    ths.m_seq->unpaint_all();

    /* if they clicked, something changed */
    ths.update_pixmap();
    ths.draw_pixmap_on_window();

    return true;
}

bool Seq24SeqEventInput::on_motion_notify_event(GdkEventMotion* a_ev, seqevent& ths)
{
    long tick = 0;

    if ( ths.m_moving_init ){
        ths.m_moving_init = false;
        ths.m_moving = true;
    }

    if ( ths.m_selecting || ths.m_moving || ths.m_paste  ){

        ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;;

        if ( ths.m_moving || ths.m_paste )
            ths.snap_x( &ths.m_current_x );

        ths.draw_selection_on_window();
    }


    if ( ths.m_painting )
    {
        ths.m_current_x = (int) a_ev->x  + ths.m_scroll_offset_x;;
        ths.snap_x( &ths.m_current_x );
        ths.convert_x( ths.m_current_x, &tick );
        ths.drop_event( tick );
    }

    return true;
}
