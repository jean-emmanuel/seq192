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
#include "seqtime.h"
#include "font.h"


seqtime::seqtime(sequence *a_seq, int a_zoom,
                 Gtk::Adjustment   *a_hadjust): DrawingArea() 
{     
    m_seq = a_seq;
    m_zoom = a_zoom;

    m_hadjust = a_hadjust;

    add_events( Gdk::BUTTON_PRESS_MASK | 
		Gdk::BUTTON_RELEASE_MASK );

    // in the construor you can only allocate colors, 
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );

    /* set default size */
    set_size_request( 10, c_timearea_y );

    m_scroll_offset_ticks = 0;
    m_scroll_offset_x = 0;

    set_double_buffered( false );
    

}



void 
seqtime::update_sizes()
{
 
    /* set these for later */
    if( is_realized() ) {
        
        m_pixmap = Gdk::Pixmap::create( m_window,
                                        m_window_x,
                                        m_window_y, -1 );
        update_pixmap();
        queue_draw();

    }
}

void 
seqtime::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    //Gtk::Main::idle.connect(mem_fun(this,&seqtime::idleProgress));
    Glib::signal_timeout().connect(mem_fun(*this,&seqtime::idle_progress), 50);

  

    
    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &seqtime::change_horz ));

    update_sizes();
}


void
seqtime::change_horz( )
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;

    update_pixmap();
    force_draw();    
}



void
seqtime::on_size_allocate(Gtk::Allocation & a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );
    
    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
    
    update_sizes(); 
 
}



bool
seqtime::idle_progress( )
{
    return true;
}



void 
seqtime::set_zoom( int a_zoom )
{
    m_zoom = a_zoom;

    reset();
 
}

void 
seqtime::reset()
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;

    update_sizes();
    update_pixmap();
    draw_pixmap_on_window();
}


void 
seqtime::redraw()
{
    
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;
    
    update_pixmap();
    draw_pixmap_on_window();
    
}

void 
seqtime::update_pixmap()
{

  
    
    /* clear background */
    m_gc->set_foreground(m_white);
    m_pixmap->draw_rectangle(m_gc,true,
                             0,
                             0, 
                             m_window_x, 
                             m_window_y );

   


    m_gc->set_foreground(m_black);
    m_pixmap->draw_line(m_gc,
		       0,
		       m_window_y - 1,
		       m_window_x,
		       m_window_y - 1 );

    // at 32, a bar every measure
    // at 16
/*

    zoom   32         16         8        4        1

    
    ml
    c_ppqn  
    *
    1      128
    2      64
    4      32        16         8
    8      16m       8          4          2       1
    16     8m        4          2          1       1
    32     4m        2          1          1       1
    64     2m        1          1          1       1 
    128    1m        1          1          1       1
    
    
*/      

    int measure_length_32nds =  m_seq->get_bpm() * 32 /
        m_seq->get_bw();

    //printf ( "measure_length_32nds[%d]\n", measure_length_32nds );
    
    int measures_per_line = (128 / measure_length_32nds) / (32 / m_zoom);
    if ( measures_per_line <= 0 )
        measures_per_line = 1;

    //printf( "measures_per_line[%d]\n", measures_per_line );

    int ticks_per_measure =  m_seq->get_bpm() * (4 * c_ppqn) / m_seq->get_bw();
    int ticks_per_step =  ticks_per_measure * measures_per_line;
    int start_tick = m_scroll_offset_ticks - (m_scroll_offset_ticks % ticks_per_step );
    int end_tick = (m_window_x * m_zoom) + m_scroll_offset_ticks;

    //printf ( "ticks_per_step[%d] start_tick[%d] end_tick[%d]\n",
    //         ticks_per_step, start_tick, end_tick );

    /* draw vert lines */
    m_gc->set_foreground(m_black);
    for ( int i=start_tick; i<end_tick; i += ticks_per_step )
    {
        int base_line = i / m_zoom;
        
        /* beat */
        m_pixmap->draw_line(m_gc,
                            base_line -  m_scroll_offset_x ,
                            0,
                            base_line -  m_scroll_offset_x ,
                            m_window_y );
        
            
        char bar[5];
        snprintf(bar, sizeof(bar), "%d", (i/ ticks_per_measure ) + 1); 
        
        m_gc->set_foreground(m_black);
        
        p_font_renderer->render_string_on_drawable(m_gc,
                                                   base_line + 2 -  m_scroll_offset_x , 
                                                   0,
                                                   m_pixmap, bar, font::BLACK );           
    
    }

    long end_x = m_seq->get_length() / m_zoom - m_scroll_offset_x;

    m_gc->set_foreground(m_black);
    m_pixmap->draw_rectangle(m_gc,true,
                             end_x,
                             9, 
                             19, 
                             8 );
       
    p_font_renderer->render_string_on_drawable(m_gc,
                                               end_x + 1, 
                                               9,
                                               m_pixmap, "END", font::WHITE );
}



void 
seqtime::draw_pixmap_on_window()
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            0,0,
                            0,0,
                            m_window_x,
                            m_window_y );
}

bool
seqtime::on_expose_event(GdkEventExpose* a_e)
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

void
seqtime::force_draw( void )
{
    m_window->draw_drawable(m_gc, 
                            m_pixmap, 
                            0,0,
                            0,0,
                            m_window_x,
                            m_window_y );
}

bool
seqtime::on_button_press_event(GdkEventButton* p0)
{
    return false;
}

bool
seqtime::on_button_release_event(GdkEventButton* p0)
{
    return false;
}
