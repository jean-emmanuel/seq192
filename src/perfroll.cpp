//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General)mm Public License as published by
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
#include "perfroll.h"

const int c_perfroll_background_x = (c_ppqn * 4 * 16) / c_perf_scale_x;
const int c_perfroll_size_box_w = 3;
const int c_perfroll_size_box_click_w = c_perfroll_size_box_w+1 ;

perfroll::perfroll( perform *a_perf,
		    Adjustment * a_hadjust,
		    Adjustment * a_vadjust  ) : DrawingArea() 
{    
    
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey = Gdk::Color( "grey" );
    m_lt_grey = Gdk::Color( "light grey" );
    
    //m_text_font_6_12 = Gdk_Font( c_font_6_12 );
  
    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    colormap->alloc_color( m_lt_grey );
    
    m_mainperf = a_perf;
    m_vadjust = a_vadjust;
    m_hadjust = a_hadjust;

    m_moving = false;
    m_growing = false;

    m_old_progress_ticks = 0;

    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::POINTER_MOTION_MASK |
		Gdk::KEY_PRESS_MASK |
		Gdk::KEY_RELEASE_MASK |
		Gdk::FOCUS_CHANGE_MASK |
		Gdk::SCROLL_MASK );


    set_size_request( 10, 10 );

    m_4bar_offset = 0;
    m_sequence_offset = 0;
    m_roll_length_ticks = 0;

    m_drop_sequence = 0;

    set_double_buffered( false );

    for( int i=0; i<c_total_seqs; ++i )
        m_sequence_active[i]=false;

} 

perfroll::~perfroll( )
{

}


void
perfroll::change_horz( )
{
    if ( m_4bar_offset != (int) m_hadjust->get_value() ){
	
	m_4bar_offset = (int) m_hadjust->get_value();
	queue_draw();
    }
}

void
perfroll::change_vert( )
{
    if ( m_sequence_offset != (int) m_vadjust->get_value() ){
	
	m_sequence_offset = (int) m_vadjust->get_value();
	queue_draw();
    }
}

void 
perfroll::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_flags( Gtk::CAN_FOCUS );

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    update_sizes();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_horz ));
    m_vadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_vert ));

    m_background = Gdk::Pixmap::create( m_window,
                                        c_perfroll_background_x,
                                        c_names_y, -1 );
    
    /* and fill the background ( dotted lines n' such ) */
    fill_background_pixmap();
    

}

void
perfroll::init_before_show( )
{
 
    m_roll_length_ticks = m_mainperf->get_max_trigger();
    m_roll_length_ticks = m_roll_length_ticks - 
	( m_roll_length_ticks % ( c_ppqn * 16 ));
    m_roll_length_ticks +=  c_ppqn * 4096;
}

void 
perfroll::update_sizes()
{
    int h_bars         = m_roll_length_ticks / (c_ppqn * 16);
    int h_bars_visable = (m_window_x * c_perf_scale_x) / (c_ppqn * 16);
    
    m_hadjust->set_lower( 0 );
    m_hadjust->set_upper( h_bars );
    m_hadjust->set_page_size( h_bars_visable );
    m_hadjust->set_step_increment( 1 );
    m_hadjust->set_page_increment( 1 );
    
    int h_max_value = h_bars - h_bars_visable;
    
    if ( m_hadjust->get_value() > h_max_value ){
       m_hadjust->set_value( h_max_value );
    }


    m_vadjust->set_lower( 0 );
    m_vadjust->set_upper( c_total_seqs ); 
    m_vadjust->set_page_size( m_window_y / c_names_y );
    m_vadjust->set_step_increment( 1 );
    m_vadjust->set_page_increment( 1 );

    int v_max_value = c_total_seqs - (m_window_y / c_names_y);

    if ( m_vadjust->get_value() > v_max_value ){
        m_vadjust->set_value(v_max_value);
    }

    if ( is_realized() ){
	m_pixmap = Gdk::Pixmap::create( m_window,
                                        m_window_x,
                                        m_window_y, -1 );
    }
    
    queue_draw();
}

void
perfroll::increment_size()
{
    m_roll_length_ticks += (c_ppqn * 512);
    update_sizes( );
}

/* updates background */
void 
perfroll::fill_background_pixmap()
{
    /* clear background */
    m_gc->set_foreground(m_white);
    m_background->draw_rectangle(m_gc,true,
 				0,
 				0, 
 				c_perfroll_background_x, 
 				c_names_y );

    /* draw horz grey lines */
    m_gc->set_foreground(m_grey);

    gint8 dash = 1;
    m_gc->set_dashes( 0, &dash, 1 );  	
    
    m_gc->set_line_attributes( 1,
                               Gdk::LINE_ON_OFF_DASH,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );

    m_background->draw_line(m_gc,
			   0,
			   0,
			   c_perfroll_background_x,
			   0 );
    
    int beats = m_measure_length / m_beat_length;

    /* draw vert lines */
    for ( int i=0; i< beats ; ){
        
 	if ( i == 0 ){
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_SOLID,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
        }
        else
        {
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_ON_OFF_DASH,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
        }
	
        m_gc->set_foreground(m_grey);
        
 	/* solid line on every beat */
 	m_background->draw_line(m_gc,
 			       i * m_beat_length / c_perf_scale_x,
 			       0,
 			       i * m_beat_length / c_perf_scale_x,
 			       c_names_y );

        // jump 2 if 16th notes
        if ( m_beat_length < c_ppqn/2 )
        {
            i += (c_ppqn / m_beat_length);
        }
        else
        {
            ++i;
        }
            
    }

    /* reset line style */

    m_gc->set_line_attributes( 1,
                               Gdk::LINE_SOLID,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );
}


/* simply sets the snap member */
void
perfroll::set_guides( int a_snap, int a_measure, int a_beat )
{
    m_snap = a_snap;
    m_measure_length = a_measure;
    m_beat_length = a_beat;

    if ( is_realized() ){
        fill_background_pixmap();
    }
    
    queue_draw();
}

void 
perfroll::draw_progress()
{
    long tick = m_mainperf->get_tick(); 
    long tick_offset = m_4bar_offset * c_ppqn * 16;
    
    int progress_x =     ( tick - tick_offset ) / c_perf_scale_x ; 
    int old_progress_x = ( m_old_progress_ticks - tick_offset ) / c_perf_scale_x ; 

    /* draw old */
    m_window->draw_drawable(m_gc, 
			 m_pixmap, 
			 old_progress_x, 0,
			 old_progress_x, 0,
			 1, m_window_y );

    m_gc->set_foreground(m_black);
    m_window->draw_line(m_gc,
		       progress_x, 0,
		       progress_x, m_window_y);
    
    m_old_progress_ticks = tick;
}



void perfroll::draw_sequence_on( Glib::RefPtr<Gdk::Drawable> a_draw, int a_sequence )
{
    
    long tick_on;
    long tick_off;
    long offset;
    bool selected;

    long tick_offset = m_4bar_offset * c_ppqn * 16;
    long x_offset = tick_offset / c_perf_scale_x;
    
    if ( a_sequence < c_total_seqs ){

	if ( m_mainperf->is_active( a_sequence )){

            m_sequence_active[a_sequence] = true;
	    
	    sequence *seq =  m_mainperf->get_sequence( a_sequence );
	    
	    seq->reset_draw_trigger_marker();
	    
	    a_sequence -= m_sequence_offset;

	    long sequence_length = seq->get_length();
	    int length_w = sequence_length / c_perf_scale_x;
	    
	    while ( seq->get_next_trigger( &tick_on, &tick_off, &selected, &offset  )){
	
                if ( tick_off > 0 ){    

		    long x_on  = tick_on  / c_perf_scale_x;
		    long x_off = tick_off / c_perf_scale_x;
		    int  w     = x_off - x_on + 1;
		    
		    int x = x_on;
		    int y = c_names_y * a_sequence + 1;  // + 2
		    int h = c_names_y - 2; // - 4

                    // adjust to screen corrids
		    x = x - x_offset;

                    if ( selected )
                        m_gc->set_foreground(m_grey);
                    else
                        m_gc->set_foreground(m_white);
                    
		    a_draw->draw_rectangle(m_gc,true,
					   x,
					   y,
					   w,
					   h );
		    
		    m_gc->set_foreground(m_black);
		    a_draw->draw_rectangle(m_gc,false,
					   x,
					   y,
					   w,
					   h );

                    m_gc->set_foreground(m_black);
                    a_draw->draw_rectangle(m_gc,false,
					   x,
					   y,
					   c_perfroll_size_box_w,
					   c_perfroll_size_box_w );
                    
                    a_draw->draw_rectangle(m_gc,false,
					   x+w-c_perfroll_size_box_w,
					   y+h-c_perfroll_size_box_w,
					   c_perfroll_size_box_w,
					   c_perfroll_size_box_w );

		    m_gc->set_foreground(m_black);
    
                    long length_marker_first_tick = ( tick_on - (tick_on % sequence_length) + (offset % sequence_length) - sequence_length);

                                      
                    long tick_marker = length_marker_first_tick;
                    
                    while ( tick_marker < tick_off ){

                        long tick_marker_x = (tick_marker / c_perf_scale_x) - x_offset;

                        if ( tick_marker > tick_on ){
                            
                            m_gc->set_foreground(m_lt_grey);
                            a_draw->draw_rectangle(m_gc,true,
                                                   tick_marker_x,
                                                   y+4,
                                                   1,
                                                   h-8 );   
                        }
                        
                        int lowest_note = seq->get_lowest_note_event( );
                        int highest_note = seq->get_highest_note_event( );
    
                        int height = highest_note - lowest_note;
                        height += 2;
    
                        int length = seq->get_length( );

                        long tick_s;
                        long tick_f;
                        int note;
	
                        bool selected;
    
                        int velocity;
                        draw_type dt;
    
                        seq->reset_draw_marker();
    
                        m_gc->set_foreground(m_black);
                        while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note, 
                                                                &selected, &velocity )) != DRAW_FIN ){
	
                            int note_y = ((c_names_y-6) - 
                                ((c_names_y-6)  * (note - lowest_note)) / height) + 1; 
    
                            int tick_s_x = ((tick_s * length_w)  / length) + tick_marker_x;
                            int tick_f_x = ((tick_f * length_w)  / length) + tick_marker_x;
    
                            if ( dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF )
                                tick_f_x = tick_s_x + 1;
                            if ( tick_f_x <= tick_s_x )
                                tick_f_x = tick_s_x + 1;

                            if ( tick_s_x < x ){
                                tick_s_x = x;
                            }

                            if ( tick_f_x > x + w ){
                                tick_f_x = x + w;
                            }

                            /*
                                    [           ]
                             -----------
                                             ---------
                                   ----------------
                         ------                      ------
                            */
                            
                            if ( tick_f_x >= x && tick_s_x <= x+w )
                                m_pixmap->draw_line(m_gc, tick_s_x,
                                                    y + note_y,
                                                    tick_f_x,  
                                                    y + note_y );   
                        }

               
                        
                        tick_marker += sequence_length;
		    }   
		}
	    }
	}
    }
}




void perfroll::draw_background_on( Glib::RefPtr<Gdk::Drawable> a_draw, int a_sequence )
{
    long tick_offset = m_4bar_offset * c_ppqn * 16;
    long first_measure = tick_offset / m_measure_length;

    a_sequence -= m_sequence_offset;

    int y = c_names_y * a_sequence;
    int h = c_names_y;

    
    m_gc->set_foreground(m_white);
    a_draw->draw_rectangle(m_gc,true,
                         0,
                         y,
                         m_window_x,
                         h );

    m_gc->set_foreground(m_black);
    for ( int i = first_measure;
              i < first_measure +
                  (m_window_x * c_perf_scale_x /
                   (m_measure_length)) + 1;
          
              i++ )
    {
        int x_pos = ((i * m_measure_length) - tick_offset) / c_perf_scale_x;


           a_draw->draw_drawable(m_gc, m_background,
                                 0,
                                 0,
                                 x_pos,
                                 y,
                                 c_perfroll_background_x,
                                 c_names_y );

    }

    

}



bool 
perfroll::on_expose_event(GdkEventExpose* e)
{

    int y_s = e->area.y / c_names_y;
    int y_f = (e->area.y  + e->area.height) / c_names_y;
    
    for ( int y=y_s; y<=y_f; y++ ){

        /*
	for ( int x=x_s; x<=x_f; x++ ){
	    
	    m_pixmap->draw_drawable(m_gc, m_background,
				 0,
				 0,
				 x * c_perfroll_background_x,
				 c_names_y * y,
				 c_perfroll_background_x,
				 c_names_y );
	}

        */
        
        draw_background_on(m_pixmap, y + m_sequence_offset );
	draw_sequence_on(m_pixmap, y + m_sequence_offset );
    }

    m_window->draw_drawable( m_gc, m_pixmap,
			  e->area.x,
			  e->area.y,
			  e->area.x,
			  e->area.y,
			  e->area.width,
			  e->area.height );
    return true;
}


void
perfroll::redraw_dirty_sequences( void )
{
    bool draw = false;
    
    int y_s = 0;
    int y_f = m_window_y / c_names_y;
    
    for ( int y=y_s; y<=y_f; y++ ){

        int seq = y + m_sequence_offset; // 4am
        
            
            bool dirty = (m_mainperf->is_dirty_perf(seq ));
            
            if (dirty)
            {
                draw_background_on(m_pixmap,seq );
                draw_sequence_on(m_pixmap,seq );
                draw = true;
            }
    }                

    if ( draw )
        m_window->draw_drawable( m_gc, m_pixmap,
                                 0,
                                 0,
                                 0,
                                 0,
                                 m_window_x,
                                 m_window_y );
}




void
perfroll::draw_drawable_row( Glib::RefPtr<Gdk::Drawable> a_dest, Glib::RefPtr<Gdk::Drawable> a_src,  long a_y )
{
    int s = a_y / c_names_y;
    a_dest->draw_drawable(m_gc, a_src,
                          0,
                          c_names_y * s,
                          0,
                          c_names_y * s,
                          m_window_x,
                          c_names_y );
}


void 
perfroll::start_playing( void )
{
    // keep in sync with perfedit's start_playing... wish i could call it directly...
    m_mainperf->position_jack( true );
    m_mainperf->start_jack( );
    m_mainperf->start( true );
}

void
perfroll::stop_playing( void )
{ 
    // keep in sync with perfedit's stop_playing... wish i could call it directly...
    m_mainperf->stop_jack(); 
    m_mainperf->stop();
}


bool
perfroll::on_button_press_event(GdkEventButton* a_ev)
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


bool
perfroll::on_button_release_event(GdkEventButton* a_ev)
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
perfroll::on_scroll_event( GdkEventScroll* a_ev )
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();
    
    if ((a_ev->state & modifiers) == GDK_SHIFT_MASK)
    {
        double val = m_hadjust->get_value();
        
        if ( a_ev->direction == GDK_SCROLL_UP ){
            val -= m_hadjust->get_step_increment();
        }
        else if ( a_ev->direction == GDK_SCROLL_DOWN ){
            val += m_hadjust->get_step_increment();
        }
        
        m_hadjust->clamp_page(val, val + m_hadjust->get_page_size());
    }
    else
    {
        double val = m_vadjust->get_value();
        
        if ( a_ev->direction == GDK_SCROLL_UP ){
            val -= m_vadjust->get_step_increment();
        }
        else if ( a_ev->direction == GDK_SCROLL_DOWN ){
            val += m_vadjust->get_step_increment();
        }
        
        m_vadjust->clamp_page(val, val + m_vadjust->get_page_size());
    }
    return true;
}

 

bool
perfroll::on_motion_notify_event(GdkEventMotion* a_ev)
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

bool 
perfroll::on_key_press_event(GdkEventKey* a_p0)
{
    bool ret = false;

    if ( m_mainperf->is_active( m_drop_sequence)){
        
        if ( a_p0->type == GDK_KEY_PRESS ){
            
            if ( a_p0->keyval ==  GDK_Delete || a_p0->keyval == GDK_BackSpace ){
                
                m_mainperf->push_trigger_undo();
                m_mainperf->get_sequence( m_drop_sequence )->del_selected_trigger();
                
                ret = true;
            }
            
            if ( a_p0->state & GDK_CONTROL_MASK ){
                
                /* cut */
                if ( a_p0->keyval == GDK_x || a_p0->keyval == GDK_X ){

                    m_mainperf->push_trigger_undo();
                    m_mainperf->get_sequence( m_drop_sequence )->cut_selected_trigger();
                    ret = true;
                }
                /* copy */
                if ( a_p0->keyval == GDK_c || a_p0->keyval == GDK_C ){
                    
                    m_mainperf->get_sequence( m_drop_sequence )->copy_selected_trigger();                
                    ret = true;
                }
                
                /* paste */
                if ( a_p0->keyval == GDK_v || a_p0->keyval == GDK_V ){

                    m_mainperf->push_trigger_undo();
                    m_mainperf->get_sequence( m_drop_sequence )->paste_trigger();
                    ret = true;
                }
            }
        }
    }
    
    if ( ret == true ){
        
        fill_background_pixmap();
        queue_draw();
        return true;
    }
    else
	return false;
}



/* performs a 'snap' on x */
void 
perfroll::snap_x( int *a_x )
{
    // snap = number pulses to snap to
    // m_scale = number of pulses per pixel
    //	so snap / m_scale  = number pixels to snap to

    int mod = (m_snap / c_perf_scale_x );

    if ( mod <= 0 )
 	mod = 1;
    
    *a_x = *a_x - (*a_x % mod );
}


void 
perfroll::convert_x( int a_x, long *a_tick )
{

    long tick_offset = m_4bar_offset * c_ppqn * 16;
    *a_tick = a_x * c_perf_scale_x;
    *a_tick += tick_offset;
}


void 
perfroll::convert_xy( int a_x, int a_y, long *a_tick, int *a_seq)
{

    long tick_offset = m_4bar_offset * c_ppqn * 16;

    *a_tick = a_x * c_perf_scale_x;  
    *a_seq = a_y / c_names_y;  

    *a_tick += tick_offset;
    *a_seq  += m_sequence_offset;

    if ( *a_seq >= c_total_seqs )
	*a_seq = c_total_seqs - 1;

    if ( *a_seq < 0 )
	*a_seq = 0;
}


bool
perfroll::on_focus_in_event(GdkEventFocus*)
{
    set_flags(Gtk::HAS_FOCUS);
    return false;
}


bool
perfroll::on_focus_out_event(GdkEventFocus*)
{
    unset_flags(Gtk::HAS_FOCUS);
    return false;
}


void
perfroll::on_size_allocate(Gtk::Allocation& a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();

    update_sizes();
}

void 
perfroll::on_size_request(GtkRequisition* a_r )
{
}




//////////////////////////
// interaction methods
//////////////////////////


void FruityPerfInput::updateMousePtr( perfroll& ths )
{
    // context sensitive mouse
    long drop_tick;
    int drop_sequence;
    ths.convert_xy( m_current_x, m_current_y, &drop_tick, &drop_sequence );
    if (ths.m_mainperf->is_active( drop_sequence ))
    {
         long start, end;
         if (ths.m_mainperf->get_sequence(drop_sequence)->intersectTriggers( drop_tick, start, end ))
	     {
             if (start <= drop_tick && drop_tick <= start + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                 (m_current_y % c_names_y) <= c_perfroll_size_box_click_w + 1)
             {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::RIGHT_PTR ));
             }
             else if (end - (c_perfroll_size_box_click_w * c_perf_scale_x) <= drop_tick && drop_tick <= end &&
                      (m_current_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1)
             {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ));
             }
             else
             {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::CENTER_PTR ));
             }
         }
         else
         {
             ths.get_window()->set_cursor( Gdk::Cursor( Gdk::PENCIL ));
         }
    }
    else
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::CROSSHAIR ));
    }
}


bool FruityPerfInput::on_button_press_event(GdkEventButton* a_ev, perfroll& ths)
{
    ths.grab_focus( );

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
    {
        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->unselect_triggers( );
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    }

    ths.m_drop_x = (int) a_ev->x;
    ths.m_drop_y = (int) a_ev->y;
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;


    ths.convert_xy( ths.m_drop_x, ths.m_drop_y, &ths.m_drop_tick, &ths.m_drop_sequence );

    /*      left mouse button     */
    if ( a_ev->button == 1 && !(a_ev->state & GDK_CONTROL_MASK)){

        long tick = ths.m_drop_tick;

        /* add a new note if we didnt select anything */
        //if (  m_adding )
        {

            m_adding_pressed = true;

            if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

                long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );

                bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

                // resize the event, or move it, depending on where clicked.
                if ( state )
                {
                    //m_adding = false;
                    m_adding_pressed = false;
                    ths.m_mainperf->push_trigger_undo();
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->select_trigger( tick );

                    long start_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_start_tick();
                    long end_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick();

                    if ( tick >= start_tick &&
                            tick <= start_tick + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                            (ths.m_drop_y % c_names_y) <= c_perfroll_size_box_click_w + 1 )
                    {
                        // clicked left side: begin a grow/shrink for the left side
                        ths.m_growing = true;
                        ths.m_grow_direction = true;
                        ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                            ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                            get_selected_trigger_start_tick( );
                    }
                    else
                        if ( tick >= end_tick - (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                                tick <= end_tick &&
                                (ths.m_drop_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1 )
                        {
                            // clicked right side: grow/shrink the right side
                            ths.m_growing = true;
                            ths.m_grow_direction = false;
                            ths.m_drop_tick_trigger_offset =
                                ths.m_drop_tick -
                                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick( );
                        }
                        else
                        {
                             // clicked in the middle - move it
                            ths.m_moving = true;
                            ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                get_selected_trigger_start_tick( );

                        }

                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
                }

                // add an event:
                else
                {

                    // snap to length of sequence
                    tick = tick - (tick % seq_length);

                    ths.m_mainperf->push_trigger_undo();
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->add_trigger( tick, seq_length );
                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);

                    //m_drop_tick_last = (m_drop_tick + seq_length - 1);
                }
            }
        }
    }

    /*     right mouse button      */
    if ( a_ev->button == 3 ){
        //set_adding( false );

        long tick = ths.m_drop_tick;

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

            //long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length();

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

            if ( state )
            {
                ths.m_mainperf->push_trigger_undo();
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->del_trigger( tick );
            }
        }
    }

    /* left-ctrl, or middle: split */
    if ( a_ev->button == 2 ||
         a_ev->button == 1 && (a_ev->state & GDK_CONTROL_MASK) )
    {
        long tick = ths.m_drop_tick;

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

            if ( state )
            {
                ths.m_mainperf->push_trigger_undo();

                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->split_trigger( tick );

                ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
            }
        }
    }
    updateMousePtr( ths );
    return true;
}

bool FruityPerfInput::on_button_release_event(GdkEventButton* a_ev, perfroll& ths)
{
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;

    if ( a_ev->button == 1 || a_ev->button == 3 )
    {
        m_adding_pressed = false;
    }

    ths.m_moving = false;
    ths.m_growing = false;
    m_adding_pressed = false;

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence  )){

        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y );
    }

    updateMousePtr( ths );
    return true;
}

bool FruityPerfInput::on_motion_notify_event(GdkEventMotion* a_ev, perfroll& ths)
{
    long tick;
    int x = (int) a_ev->x;
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;
    
    if (  m_adding_pressed ){

    	ths.convert_x( x, &tick );

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

            long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );
            tick = tick - (tick % seq_length);

            /*long min_tick = (tick < m_drop_tick) ? tick : m_drop_tick;*/
            long length = seq_length;

    	    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                          ->grow_trigger( ths.m_drop_tick, tick, length);
    	    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
    	    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    	}
    }
    else if ( ths.m_moving || ths.m_growing )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence))
        {
            ths.convert_x( x, &tick );
            tick -= ths.m_drop_tick_trigger_offset;

            tick = tick - tick % ths.m_snap;

            if ( ths.m_moving )
            {
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                              ->move_selected_triggers_to( tick, true );
            }
            if ( ths.m_growing )
            {
                if ( ths.m_grow_direction )
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                                  ->move_selected_triggers_to( tick, false, 0 );
                else
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                                  ->move_selected_triggers_to( tick-1, false, 1 );
            }


            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }

    updateMousePtr( ths );
    return true;
}


/* popup menu calls this */
void
Seq24PerfInput::set_adding( bool a_adding, perfroll& ths )
{
    if ( a_adding )
    {
	   ths.get_window()->set_cursor(  Gdk::Cursor( Gdk::PENCIL ));
	   m_adding = true;
    }
    else
    {
	    ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ));
	    m_adding = false;
    }
}

bool
Seq24PerfInput::on_button_press_event(GdkEventButton* a_ev, perfroll& ths)
{
    ths.grab_focus( );


    if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
    {
        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->unselect_triggers( );
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    }

    ths.m_drop_x = (int) a_ev->x;
    ths.m_drop_y = (int) a_ev->y;

    ths.convert_xy( ths.m_drop_x, ths.m_drop_y, &ths.m_drop_tick, &ths.m_drop_sequence );

    /*      left mouse button     */
    if ( a_ev->button == 1 ){

        long tick = ths.m_drop_tick;

        /* add a new note if we didnt select anything */
        if (  m_adding ){

            m_adding_pressed = true;

            if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

                long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );

                bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

                if ( state )
                {
                    ths.m_mainperf->push_trigger_undo();
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->del_trigger( tick );
                }
                else
                {

                    // snap to length of sequence
                    tick = tick - (tick % seq_length);
                    //m_adding_pressed_state = true;

                    ths.m_mainperf->push_trigger_undo();
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->add_trigger( tick, seq_length );
                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);

                    //m_drop_tick_last = (m_drop_tick + seq_length - 1);
                }
            }
        }
        else {

            if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

                ths.m_mainperf->push_trigger_undo();
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->select_trigger( tick );

                long start_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_start_tick();
                long end_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick();

                if ( tick >= start_tick &&
                        tick <= start_tick + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                        (ths.m_drop_y % c_names_y) <= c_perfroll_size_box_click_w + 1 )
                {
                    ths.m_growing = true;
                    ths.m_grow_direction = true;
                    ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                        get_selected_trigger_start_tick( );
                }
                else
                    if ( tick >= end_tick - (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                            tick <= end_tick &&
                            (ths.m_drop_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1 )
                    {
                        ths.m_growing = true;
                        ths.m_grow_direction = false;
                        ths.m_drop_tick_trigger_offset =
                            ths.m_drop_tick -
                            ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick( );
                    }
                    else 
                    {
                        ths.m_moving = true;
                        ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                            ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                            get_selected_trigger_start_tick( );

                    }

                ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
            }
        }        
    }

    /*     right mouse button      */
    if ( a_ev->button == 3 ){
        set_adding( true, ths );
    }

    /* middle, split */
    if ( a_ev->button == 2 )
    {
        long tick = ths.m_drop_tick;

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

            if ( state )
            {
                ths.m_mainperf->push_trigger_undo();

                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->split_trigger( tick );

                ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
            }
        }
    }
    return true;
}

bool Seq24PerfInput::on_button_release_event(GdkEventButton* a_ev, perfroll& ths)
{
    if ( a_ev->button == 1 ){

        if ( m_adding ){
            m_adding_pressed = false;
        }
    }

    if ( a_ev->button == 3 ){
	m_adding_pressed = false;
 	set_adding( false, ths );
    }
    
    ths.m_moving = false;
    ths.m_growing = false;
    m_adding_pressed = false;

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence  )){
        
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y );
    }
    
    return true;
}

bool Seq24PerfInput::on_motion_notify_event(GdkEventMotion* a_ev, perfroll& ths)
{
    long tick;
    int x = (int) a_ev->x;
    
    if (  m_adding && m_adding_pressed ){
        
    	ths.convert_x( x, &tick );

    	if ( ths.m_mainperf->is_active( ths.m_drop_sequence )){

            long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );
            tick = tick - (tick % seq_length);

            /*long min_tick = (tick < m_drop_tick) ? tick : m_drop_tick;*/
            long length = seq_length;
            
    	    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                          ->grow_trigger( ths.m_drop_tick, tick, length);
    	    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
    	    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    	}
    }
    else if ( ths.m_moving || ths.m_growing ){

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence)){
            
            ths.convert_x( x, &tick );
            tick -= ths.m_drop_tick_trigger_offset;

            tick = tick - tick % ths.m_snap;

            if ( ths.m_moving )
            {
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                              ->move_selected_triggers_to( tick, true );
            }
            if ( ths.m_growing )
            {
                if ( ths.m_grow_direction )
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                                  ->move_selected_triggers_to( tick, false, 0 );
                else
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                                  ->move_selected_triggers_to( tick-1, false, 1 );
            }

            
            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }

    return true;
}
