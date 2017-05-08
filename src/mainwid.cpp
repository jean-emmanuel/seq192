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

#include "mainwid.h"
#include "seqedit.h"
#include "font.h"


const char mainwid::m_seq_to_char[c_seqs_in_set] =
{
    '1', 'Q', 'A', 'Z', 
    '2', 'W', 'S', 'X', 
    '3', 'E', 'D', 'C', 
    '4', 'R', 'F', 'V', 
    '5', 'T', 'G', 'B', 
    '6', 'Y', 'H', 'N', 
    '7', 'U', 'J', 'M', 
    '8', 'I', 'K', ',' 
};

// Constructor

mainwid::mainwid( perform *a_p  ): DrawingArea(), seqmenu( a_p ) 
{    
    using namespace Menu_Helpers;

    m_mainperf = a_p;

    m_window_x = c_mainwid_x;
    m_window_y = c_mainwid_y;
    
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );

    set_size_request( c_mainwid_x, c_mainwid_y );
  
    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::KEY_PRESS_MASK |
		Gdk::BUTTON_MOTION_MASK |
		Gdk::FOCUS_CHANGE_MASK  );
  
    m_screenset = 0;

  
    m_button_down = false;
    m_moving = false;

    set_double_buffered( false );

} 


// GTK, on realize of window, init shiz
void 
mainwid::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_flags( Gtk::CAN_FOCUS );

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    p_font_renderer->init( m_window );

    m_pixmap = Gdk::Pixmap::create(m_window, 
                                   c_mainwid_x, 
                                   c_mainwid_y,
                                   -1 );

    fill_background_window();
    draw_sequences_on_pixmap();

    

}

// Fills Pixmap
void 
mainwid::draw_sequences_on_pixmap()
{
    for ( int i=0; i< c_mainwnd_rows *  c_mainwnd_cols; i++ ){
	
	draw_sequence_on_pixmap( i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ));
	m_last_tick_x[ i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols)  ] = 0;
    }
}


// updates background 
void 
mainwid::fill_background_window()
{
    /* clear background */

    m_pixmap->draw_rectangle(this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
    		    true,
    		    0,
    		    0, 
    		    m_window_x, 
    		    m_window_y );
}

int
mainwid::timeout( void )
{
    return true;
}


// Draws a specific sequence on the Pixmap
void 
mainwid::draw_sequence_on_pixmap( int a_seq )
{

    if ( a_seq >= (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ) &&
         a_seq <  ((m_screenset+1)  * c_mainwnd_rows * c_mainwnd_cols )){

	int i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
	int j =  a_seq % c_mainwnd_rows;

	int base_x = (c_mainwid_border +
		      (c_seqarea_x + c_mainwid_spacing) * i);
	int base_y = (c_mainwid_border +
		      (c_seqarea_y + c_mainwid_spacing) * j);

	/*int local_seq = a_seq % c_seqs_in_set;*/

	m_gc->set_foreground(m_black);
	m_pixmap->draw_rectangle(m_gc, true,
				base_x,
				base_y,
				c_seqarea_x,  
				c_seqarea_y );
    
	if ( m_mainperf->is_active( a_seq )){
	
	    sequence *seq = m_mainperf->get_sequence( a_seq );
	
	    if ( seq->get_playing() ){
	    
	      m_last_playing[a_seq] = true;
	      m_background = m_black;
	      m_foreground = m_white;
	    
	    } else {
	    
	      m_last_playing[a_seq] = false;
	      m_background = m_white;
	      m_foreground = m_black;
	    }

            m_gc->set_foreground(m_background);
	    m_pixmap->draw_rectangle(m_gc,true,
				    base_x + 1,
				    base_y + 1,
				    c_seqarea_x - 2,  
				    c_seqarea_y - 2 );
	
	    
	    m_gc->set_foreground(m_foreground);
	    char name[20];
	    sprintf( name, "%.13s", seq->get_name() );


        font::Color col = font::BLACK;;
        
        if ( m_foreground == m_black ){
            col = font::BLACK;
        }
        if ( m_foreground == m_white ){
            col = font::WHITE;
        }

        p_font_renderer->render_string_on_drawable( m_gc, 
                                                    base_x + c_text_x, 
                                                    base_y + 4,
                                                    m_pixmap, name, col);
	
	    /* midi channel + key + timesig */

		/*char key =  m_seq_to_char[local_seq];*/

	    char str[20];

        if (m_mainperf->show_ui_sequence_key())
        {
    	    sprintf( str, "%c", (char)m_mainperf->lookup_keyevent_key( a_seq ) );

    	    p_font_renderer->render_string_on_drawable(m_gc,
                                                       base_x + c_seqarea_x - 7,
                                                       base_y + c_text_y * 4 - 2,
                                                       m_pixmap, str, col );
        }

	    sprintf( str,
		     "%d-%d %ld/%ld",
		     seq->get_midi_bus(), 
		     seq->get_midi_channel()+1,
		     seq->get_bpm(), seq->get_bw() );

	    p_font_renderer->render_string_on_drawable(m_gc,
                                                   base_x + c_text_x, 
                                                   base_y + c_text_y * 4 - 2,
                                                   m_pixmap, str, col );

    

    
	    int rectangle_x = base_x + c_text_x - 1;
	    int rectangle_y = base_y + c_text_y + c_text_x - 1;

            if ( seq->get_queued() ){
                
                m_gc->set_foreground(m_grey);
                m_pixmap->draw_rectangle(m_gc,true,
                                         rectangle_x - 2,
                                         rectangle_y - 1,
                                         c_seqarea_seq_x + 3,  
                                         c_seqarea_seq_y + 3 );

                 m_foreground = m_black;
                
            }


            
	    m_gc->set_foreground(m_foreground);
	    m_pixmap->draw_rectangle(m_gc,false,
                                     rectangle_x - 2,
                                     rectangle_y - 1,
                                     c_seqarea_seq_x + 3,  
                                     c_seqarea_seq_y + 3 );
    
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
    
    
	    while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note, 
					      &selected, &velocity )) != DRAW_FIN ){
	
		int note_y = c_seqarea_seq_y - 
			     (c_seqarea_seq_y  * (note + 1 - lowest_note)) / height ; 
    
		int tick_s_x = (tick_s * c_seqarea_seq_x)  / length;
		int tick_f_x = (tick_f * c_seqarea_seq_x)  / length;
    
		if ( dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF )
		    tick_f_x = tick_s_x + 1;
		if ( tick_f_x <= tick_s_x )
		    tick_f_x = tick_s_x + 1; 
    
		m_gc->set_foreground(m_foreground);
		m_pixmap->draw_line(m_gc, rectangle_x + tick_s_x,
					 rectangle_y + note_y,
					 rectangle_x + tick_f_x,  
					 rectangle_y + note_y );
    
    
	    }
	
	} else {
    
	    /* not active */
    
	    m_gc->set_foreground(m_grey);
	    m_pixmap->draw_rectangle( this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
				    true,
				    base_x + 4,       base_y,
				    c_seqarea_x - 8,  c_seqarea_y );
    
	    m_pixmap->draw_rectangle( this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
				    true,
				    base_x + 1,       base_y + 1,
				    c_seqarea_x - 2,  c_seqarea_y - 2 ); 
	}
    }
}

void
mainwid::draw_sequence_pixmap_on_window( int a_seq )
{
    if ( a_seq >= (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ) &&
         a_seq <  ((m_screenset+1)  * c_mainwnd_rows * c_mainwnd_cols )){
        
        int i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        int j =  a_seq % c_mainwnd_rows;
        
        int base_x = (c_mainwid_border +
                      (c_seqarea_x + c_mainwid_spacing) * i);
        int base_y = (c_mainwid_border +
                      (c_seqarea_y + c_mainwid_spacing) * j);

        m_window->draw_drawable(m_gc, 
                                m_pixmap, 
                                base_x,
                                base_y,
                                base_x,
                                base_y,
                                c_seqarea_x,
                                c_seqarea_y );    }
	    

    
}

void
mainwid::redraw( int a_sequence )
{
    draw_sequence_on_pixmap( a_sequence );
    draw_sequence_pixmap_on_window( a_sequence );
}

void 
mainwid::update_markers( int a_ticks )
{
    for ( int i=0; i< c_mainwnd_rows *  c_mainwnd_cols; i++ )
	draw_marker_on_sequence( i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ), a_ticks);
}


void 
mainwid::draw_marker_on_sequence( int a_seq, int a_tick )
{

   	if ( m_mainperf->is_dirty_main(a_seq ) ){
		update_sequence_on_window( a_seq );
	}

 
    if ( m_mainperf->is_active( a_seq )){

	sequence *seq = m_mainperf->get_sequence( a_seq );

	int i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
	int j =  a_seq % c_mainwnd_rows;

	int base_x = (c_mainwid_border +
		  (c_seqarea_x + c_mainwid_spacing) * i);
	int base_y = (c_mainwid_border +
		  (c_seqarea_y + c_mainwid_spacing) * j);

	int rectangle_x = base_x + c_text_x - 1;
	int rectangle_y = base_y + c_text_y + c_text_x - 1;
        
	int length = seq->get_length( );
        a_tick += (length - seq->get_trigger_offset( ));
	a_tick %= length;

	long tick_x = a_tick * c_seqarea_seq_x / length;

	   
        m_window->draw_drawable(m_gc, 
    			 m_pixmap, 
    			 rectangle_x + m_last_tick_x[a_seq],
			 rectangle_y + 1,
			 rectangle_x + m_last_tick_x[a_seq],
			 rectangle_y + 1,
			 1,
			 c_seqarea_seq_y );

	m_last_tick_x[a_seq] = tick_x;

	if ( seq->get_playing() ){
		m_gc->set_foreground(m_white);    
	} else {
		m_gc->set_foreground(m_black); 
	}

	if ( seq->get_queued()){
		m_gc->set_foreground(m_black);
	}


	m_window->draw_line(m_gc,
			   rectangle_x + tick_x,
			   rectangle_y + 1,
			   rectangle_x + tick_x,  
			   rectangle_y + c_seqarea_seq_y );
	
	//if ( seq->get_playing() ){
	//    
	//}
    }
}

void
mainwid::update_sequences_on_window()
{
    draw_sequences_on_pixmap( );
    draw_pixmap_on_window();
}

void
mainwid::update_sequence_on_window( int a_seq   )
{
    draw_sequence_on_pixmap( a_seq );
    draw_sequence_pixmap_on_window( a_seq );
}

// queues blit of pixmap to window
void 
mainwid::draw_pixmap_on_window()
{
    queue_draw();
}


// GTK expose event
bool
mainwid::on_expose_event(GdkEventExpose* a_e)
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

// Translates XY corridinates to a sequence number
int 
mainwid::seq_from_xy( int a_x, int a_y )
{
    /* adjust for border */
    int x = a_x - c_mainwid_border;
    int y = a_y - c_mainwid_border;

    /* is it in the box ? */
    if ( x < 0 
	 || x >= ((c_seqarea_x + c_mainwid_spacing ) * c_mainwnd_cols )
	 || y < 0 
	 || y >= ((c_seqarea_y + c_mainwid_spacing ) * c_mainwnd_rows )){
	
	return -1;
    }

    /* gives us in box corrdinates */
    int box_test_x = x % (c_seqarea_x + c_mainwid_spacing);
    int box_test_y = y % (c_seqarea_y + c_mainwid_spacing);

    /* right inactive side of area */
    if ( box_test_x > c_seqarea_x 
	 || box_test_y > c_seqarea_y ){

	return -1;
    }

    x /= (c_seqarea_x + c_mainwid_spacing);
    y /= (c_seqarea_y + c_mainwid_spacing);

    int sequence =  ( (x * c_mainwnd_rows + y) 
		      + ( m_screenset * c_mainwnd_rows * c_mainwnd_cols ));

    return sequence;

}

 

// press a mouse button
bool
mainwid::on_button_press_event(GdkEventButton* a_p0)
{
    grab_focus();

    m_current_seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    if ( m_current_seq != -1  && a_p0->button == 1 ){
 
	m_button_down = true;
    }

    return true;
}


bool
mainwid::on_button_release_event(GdkEventButton* a_p0)
{

    m_current_seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    m_button_down = false;

    /* it hit a sequence ? */
    // toggle play mode of sequence (left button)

    if ( m_current_seq != -1  && a_p0->button == 1 && !m_moving ){

        if ( m_mainperf->is_active( m_current_seq )){

            //sequence *seq = m_mainperf->get_sequence(  m_current_seq );
            //seq->set_playing( !seq->get_playing() );

            m_mainperf->sequence_playing_toggle( m_current_seq );

            draw_sequence_on_pixmap(  m_current_seq );
            draw_sequence_pixmap_on_window( m_current_seq);
        }
    }

    if ( a_p0->button == 1 && m_moving ){

        m_moving = false;

        if ( ! m_mainperf->is_active( m_current_seq ) && m_current_seq != -1
                && !m_mainperf->is_sequence_in_edit( m_current_seq )  ){

            m_mainperf->new_sequence( m_current_seq  );
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;

            draw_sequence_on_pixmap( m_current_seq  );
            draw_sequence_pixmap_on_window( m_current_seq );

        } else {

            m_mainperf->new_sequence( m_old_seq  );
            *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;

            draw_sequence_on_pixmap( m_old_seq  );
            draw_sequence_pixmap_on_window( m_old_seq );
        }


    }
    // launch menu (right button)
    if (  m_current_seq != -1 && a_p0->button == 3  ){
        popup_menu();
    }


    return true;
}

bool
mainwid::on_motion_notify_event(GdkEventMotion* a_p0)
{
    int seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    if ( m_button_down ){

	if ( seq != m_current_seq && !m_moving &&  
		 !m_mainperf->is_sequence_in_edit( m_current_seq ) ){

	    if ( m_mainperf->is_active( m_current_seq )){

		m_old_seq = m_current_seq;
		m_moving = true;

		m_moving_seq = *(m_mainperf->get_sequence( m_current_seq ));
		m_mainperf->delete_sequence( m_current_seq );
		draw_sequence_on_pixmap( m_current_seq  );
		draw_sequence_pixmap_on_window( m_current_seq );
	    }
	}
    }

    return true;
}



// redraws everything, queues redraw
void 
mainwid::reset( )
{
    draw_sequences_on_pixmap();
    draw_pixmap_on_window();
}


//int 
//mainwid::get_screenset( )
//{
//    return m_screenset;
//}

void 
mainwid::set_screenset( int a_ss )
{
    m_screenset = a_ss;

    if ( m_screenset < 0 ) 
	m_screenset = c_max_sets - 1;

    if ( m_screenset >= c_max_sets )
	m_screenset = 0;

	m_mainperf->set_offset(m_screenset);

    reset();
}




mainwid::~mainwid( )
{
    
}



bool
mainwid::on_focus_in_event(GdkEventFocus*)
{
    set_flags(Gtk::HAS_FOCUS);
    return false;
}

bool
mainwid::on_focus_out_event(GdkEventFocus*)
{
    unset_flags(Gtk::HAS_FOCUS);
    return false;
}
