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
#include "seqkeys.h"
#include "font.h"


seqkeys::seqkeys(sequence *a_seq,
                 Gtk::Adjustment *a_vadjust ): DrawingArea() 
{     
    m_seq = a_seq;
    
    m_vadjust = a_vadjust;
    
    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::ENTER_NOTIFY_MASK |
		Gdk::LEAVE_NOTIFY_MASK |
		Gdk::POINTER_MOTION_MASK |
		Gdk::SCROLL_MASK);

    /* set default size */
    set_size_request( c_keyarea_x +1, 10 );

    //m_window_x = 10;
    //m_window_y = c_keyarea_y;

    // in the construor you can only allocate colors, 
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey = Gdk::Color( "grey" );
  
    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    
    m_keying = false;
    m_hint_state = false;

    m_scroll_offset_key = 0;
    m_scroll_offset_y = 0;

    m_scale = 0;
    m_key = 0;

    set_double_buffered( false );
}

void 
seqkeys::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    m_pixmap = Gdk::Pixmap::create(m_window,
                                   c_keyarea_x,
                                   c_keyarea_y,
                                   -1 );
  
    update_pixmap();

    m_vadjust->signal_value_changed().connect( mem_fun( *this, &seqkeys::change_vert ));

    change_vert();
}

/* sets the music scale */
void 
seqkeys::set_scale( int a_scale )
{
  if ( m_scale != a_scale ){
    m_scale = a_scale;
    reset();
  }

}

/* sets the key */
void 
seqkeys::set_key( int a_key )
{
  if ( m_key != a_key ){
    m_key = a_key;
    reset();
  }
}


void 
seqkeys::reset()
{
    update_pixmap();
    queue_draw();
}



void 
seqkeys::update_pixmap()
{
    m_gc->set_foreground(m_black);
    m_pixmap->draw_rectangle(m_gc,true,
                             0,
                             0, 
                             c_keyarea_x, 
                             c_keyarea_y  );
    
    m_gc->set_foreground(m_white);
    m_pixmap->draw_rectangle(m_gc,true,
                             1,
                             1, 
                             c_keyoffset_x - 1, 
                             c_keyarea_y - 2  );
    
    
    for ( int i=0; i<c_num_keys; i++ )
    {
        m_gc->set_foreground(m_white);
        m_pixmap->draw_rectangle(m_gc,true,
                                 c_keyoffset_x + 1,
                                 (c_key_y * i) + 1, 
                                 c_key_x - 2, 
                                 c_key_y - 1 );
        
        /* the the key in the octave */
        int key = (c_num_keys - i - 1) % 12;
        
        if ( key == 1 || 
             key == 3 || 
             key == 6 || 
             key == 8 || 
             key == 10 ){
            
            m_gc->set_foreground(m_black);
            m_pixmap->draw_rectangle(m_gc,true,
                                     c_keyoffset_x + 1,
                                     (c_key_y * i) + 2, 
                                     c_key_x - 3, 
                                     c_key_y - 3 );
        }

        char notes[20];
        
        if ( key == m_key  ){
            
        
            
            /* notes */
            int octave = ((c_num_keys - i - 1) / 12) - 1;
            if ( octave < 0 )
                octave *= -1;
            
            snprintf(notes, sizeof(notes), "%2s%1d", c_key_text[key], octave);
            
            p_font_renderer->render_string_on_drawable(m_gc,
                                                       2, 
                                                       c_key_y * i - 1,
                                                       m_pixmap, notes, font::BLACK );
        }

        //snprintf(notes, sizeof(notes), "%c %d", c_scales_symbol[m_scale][key], m_scale );
            
        //p_font_renderer->render_string_on_drawable(m_gc,
        //                                             2 + (c_text_x * 4), 
        //                                             c_key_y * i - 1,
        //                                             m_pixmap, notes, font::BLACK );
    }
}

void 
seqkeys::draw_area()
{
      update_pixmap();
      m_window->draw_drawable(m_gc, 
                              m_pixmap, 
                              0,
                              m_scroll_offset_y,
                              0,
                              0,
                              c_keyarea_x,
                              c_keyarea_y );
}


bool
seqkeys::on_expose_event(GdkEventExpose* a_e)
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            a_e->area.x,
                            a_e->area.y + m_scroll_offset_y,
                            a_e->area.x,
                            a_e->area.y,
                            a_e->area.width,
                            a_e->area.height );
    return true;
}


void
seqkeys::force_draw( void )
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            0,m_scroll_offset_y,
                            0,0,
                            m_window_x,
                            m_window_y );
}


/* takes screen corrdinates, give us notes and ticks */
void 
seqkeys::convert_y( int a_y, int *a_note)
{
    *a_note = (c_rollarea_y - a_y - 2) / c_key_y; 
}


bool
seqkeys::on_button_press_event(GdkEventButton *a_e)
{
    int y,note;
   
    if ( a_e->type == GDK_BUTTON_PRESS ){

	y = (int) a_e->y + m_scroll_offset_y;

	if ( a_e->button == 1 ){
	    
	    m_keying = true;

	    convert_y( y,&note );
	    m_seq->play_note_on(  note );

	    m_keying_note = note;
	}
    }
    return true;
}


bool
seqkeys::on_button_release_event(GdkEventButton* a_e)
{   
    if ( a_e->type == GDK_BUTTON_RELEASE ){

	if ( a_e->button == 1 && m_keying ){
	    
	    m_keying = false;
	    m_seq->play_note_off( m_keying_note );
	}
    }
    return true;
}


bool
seqkeys::on_motion_notify_event(GdkEventMotion* a_p0)
{

    int y, note;
 
    y = (int) a_p0->y + m_scroll_offset_y;
    convert_y( y,&note );

    set_hint_key( note );
    
    if ( m_keying ){

        if ( note != m_keying_note ){

	    m_seq->play_note_off( m_keying_note );
	    m_seq->play_note_on(  note );
	    m_keying_note = note;

	}
    }

    return false;
}



bool
seqkeys::on_enter_notify_event(GdkEventCrossing* a_p0)
{
  set_hint_state( true );
  return false;
}



bool
seqkeys::on_leave_notify_event(GdkEventCrossing* p0)
{
    if ( m_keying ){

	m_keying = false;
	m_seq->play_note_off( m_keying_note );

    }
    set_hint_state( false );

    return true;
}

/* sets key to grey */
void 
seqkeys::set_hint_key( int a_key )
{
    draw_key( m_hint_key, false );
    
    m_hint_key = a_key;
    
    if ( m_hint_state )
        draw_key( a_key, true );
}

/* true == on, false == off */
void 
seqkeys::set_hint_state( bool a_state )
{
    m_hint_state = a_state;
    
    if ( !a_state )
        draw_key( m_hint_key, false );
}

/* a_state, false = normal, true = grayed */
void 
seqkeys::draw_key( int a_key, bool a_state )
{

  /* the the key in the octave */
  int key = a_key % 12;

  a_key = c_num_keys - a_key - 1; 

  if ( key == 1 || 
       key == 3 || 
       key == 6 || 
       key == 8 || 
       key == 10 ){
    
    m_gc->set_foreground(m_black);
  }
  else
    m_gc->set_foreground(m_white);


  m_window->draw_rectangle(m_gc,true,
			  c_keyoffset_x + 1,
			  (c_key_y * a_key) + 2 -  m_scroll_offset_y, 
			  c_key_x - 3, 
			  c_key_y - 3 );

  if ( a_state ){

    m_gc->set_foreground(m_grey);
 
    m_window->draw_rectangle(m_gc,true,
			    c_keyoffset_x + 1,
			    (c_key_y * a_key) + 2 - m_scroll_offset_y, 
			    c_key_x - 3, 
			    c_key_y - 3 );

  }
}



void
seqkeys::change_vert( )
{
   
    m_scroll_offset_key = (int) m_vadjust->get_value();
    m_scroll_offset_y = m_scroll_offset_key * c_key_y,
    
    force_draw();
    
}



void
seqkeys::on_size_allocate(Gtk::Allocation& a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();

  

    queue_draw();
 
}


bool
seqkeys::on_scroll_event( GdkEventScroll* a_ev )
{
	double val = m_vadjust->get_value();

	if ( a_ev->direction == GDK_SCROLL_UP ){
		val -= m_vadjust->get_step_increment()/6;
	} else if (  a_ev->direction == GDK_SCROLL_DOWN ){
		val += m_vadjust->get_step_increment()/6;
	} else {
		return true;
	}

	m_vadjust->clamp_page( val, val + m_vadjust->get_page_size() );
    return true;

}

