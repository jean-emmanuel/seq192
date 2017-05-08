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
#include "perfnames.h"
#include "font.h"


perfnames::perfnames( perform *a_perf, Adjustment *a_vadjust ): DrawingArea(), seqmenu(a_perf)
{     
    m_mainperf = a_perf;

    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK |
		Gdk::SCROLL_MASK );

    /* set default size */	
    set_size_request( c_names_x, 100 );

    // in the construor you can only allocate colors, 
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap>  colormap= get_default_colormap();

  

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );

    m_vadjust = a_vadjust;
    m_vadjust->signal_value_changed().connect( mem_fun( *(this), &perfnames::change_vert ));

    m_sequence_offset = 0;

    set_double_buffered( false );
    
    for( int i=0; i<c_total_seqs; ++i )
        m_sequence_active[i]=false;

}




void 
perfnames::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();
    
    m_pixmap = Gdk::Pixmap::create(m_window,
                                   c_names_x,
                                   c_names_y  * c_total_seqs + 1,
                                   -1);
}


void
perfnames::change_vert( )
{   
    if ( m_sequence_offset != (int) m_vadjust->get_value() ){
        
        m_sequence_offset = (int) m_vadjust->get_value();
        queue_draw();
    }
}

void 
perfnames::update_pixmap()
{

}

void 
perfnames::draw_area(){

}


void 
perfnames::redraw( int sequence )
{
    draw_sequence( sequence);
}

void
perfnames::draw_sequence( int sequence )
{

    int i = sequence - m_sequence_offset;
    
    if ( sequence < c_total_seqs ){

   
        
        m_gc->set_foreground(m_black);
        m_window->draw_rectangle(m_gc,true,
                                 0,
                                 (c_names_y * i) ,
                                 c_names_x, 
                                 c_names_y + 1 );
	    
        if ( sequence % c_seqs_in_set == 0 ){
		
            char ss[3];
            snprintf(ss, sizeof(ss), "%2d", sequence / c_seqs_in_set );
		
            m_gc->set_foreground(m_white);
                
            p_font_renderer->render_string_on_drawable(m_gc,
                                                       2, 
                                                       c_names_y * i + 2,
                                                       m_window, ss, font::WHITE );
        }
	    
        else {
		
            m_gc->set_foreground(m_white);
            m_window->draw_rectangle(m_gc,true,
                                     1,
                                     (c_names_y * (i)), 
                                     (6*2) + 1, 
                                     c_names_y );
        }
	    
	    
        if ( m_mainperf->is_active( sequence ))
            m_gc->set_foreground(m_white);
        else
            m_gc->set_foreground(m_grey);
	    
        m_window->draw_rectangle(m_gc,true,
                                 6 * 2 + 3,
                                 (c_names_y * i) + 1, 
                                 c_names_x - 3 - (6*2), 
                                 c_names_y - 1  );
	    
        if ( m_mainperf->is_active( sequence )){

            m_sequence_active[sequence]=true;
		
            /* names */
            char name[50];
            snprintf(name, sizeof(name), "%-14.14s   %2d", 
                     m_mainperf->get_sequence(sequence)->get_name(),
                     m_mainperf->get_sequence(sequence)->get_midi_channel() + 1);
                
            p_font_renderer->render_string_on_drawable(m_gc,
                                                       5 + 6*2,  
                                                       c_names_y * i + 2,
                                                       m_window, name, font::BLACK );
                
            char str[20];
            snprintf(str, sizeof(str), 
                     "%d-%d %ld/%ld",
                     m_mainperf->get_sequence(sequence)->get_midi_bus(), 
                     m_mainperf->get_sequence(sequence)->get_midi_channel()+1,
                     m_mainperf->get_sequence(sequence)->get_bpm(),
                     m_mainperf->get_sequence(sequence)->get_bw() );
                
            p_font_renderer->render_string_on_drawable(m_gc,
                                                       5 + 6*2,  
                                                       c_names_y * i + 12,
                                                       m_window, str, font::BLACK );

            bool muted = m_mainperf->get_sequence(sequence)->get_song_mute();

            m_gc->set_foreground(m_black);
            m_window->draw_rectangle(m_gc,muted,
                                     6*2 + 6 * 20 + 2, 
                                     (c_names_y * i),
                                     10, 
                                     c_names_y  );
                
            if ( muted ){
                    
                p_font_renderer->render_string_on_drawable(m_gc,
                                                           5 + 6*2 + 6 * 20,  
                                                           c_names_y * i + 2,
                                                           m_window, "M", font::WHITE );
            } else {

                p_font_renderer->render_string_on_drawable(m_gc,
                                                           5 + 6*2 + 6 * 20,  
                                                           c_names_y * i + 2,
                                                           m_window, "M", font::BLACK );
            }
        }
    }
    else {
            
        m_gc->set_foreground(m_grey);
        m_window->draw_rectangle(m_gc,true,
                                 0,
                                 (c_names_y * i) + 1 ,
                                 c_names_x, 
                                 c_names_y );
            
    }

}



bool
perfnames::on_expose_event(GdkEventExpose* a_e)
{
    int seqs = (m_window_y / c_names_y) + 1;
    
    for ( int i=0; i< seqs; i++ ){
        
	int sequence = i + m_sequence_offset;
        
        draw_sequence(sequence);
        
    }
    return true;
}


void 
perfnames::convert_y( int a_y, int *a_seq)
{
    *a_seq = a_y / c_names_y;
    *a_seq  += m_sequence_offset;

    if ( *a_seq >= c_total_seqs )
	*a_seq = c_total_seqs - 1;
    
    if ( *a_seq < 0 )
	*a_seq = 0;
}


bool
perfnames::on_button_press_event(GdkEventButton *a_e)
{
    int sequence;
    
    /*int x = (int) a_e->x;*/
    int y = (int) a_e->y;
    
    convert_y( y, &sequence );
    
    m_current_seq = sequence;
    
    /*      left mouse button     */
    if ( a_e->button == 1 ){

        if ( m_mainperf->is_active( sequence )){

            bool muted = m_mainperf->get_sequence(sequence)->get_song_mute();
            m_mainperf->get_sequence(sequence)->set_song_mute( !muted );

            queue_draw();
        }
    }
    
    return true;
}


bool
perfnames::on_button_release_event(GdkEventButton* p0)
{
    /*     right mouse button      */
    if ( p0->button == 3 ){
        popup_menu();    
    }
    
    return false;
}

bool
perfnames::on_scroll_event( GdkEventScroll* a_ev )
{
	double val = m_vadjust->get_value();

    if (  a_ev->direction == GDK_SCROLL_UP ){
		val -= m_vadjust->get_step_increment();
    }
    if (  a_ev->direction == GDK_SCROLL_DOWN ){
		val += m_vadjust->get_step_increment();
    }

	m_vadjust->clamp_page( val, val + m_vadjust->get_page_size());
    return true;
}


void
perfnames::on_size_allocate(Gtk::Allocation &a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
}



void
perfnames::redraw_dirty_sequences( void )
{
    bool draw = false;
    
    int y_s = 0;
    int y_f = m_window_y / c_names_y;
    
    for ( int y=y_s; y<=y_f; y++ ){

        int seq = y + m_sequence_offset; // 4am

        if ( seq < c_total_seqs){
            
                bool dirty = (m_mainperf->is_dirty_names( seq ));
                
                if (dirty)
                {
                    draw_sequence( seq );
                    draw = true;
                }
        }
    }
}
