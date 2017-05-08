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
#include "perftime.h"
#include "font.h"


perftime::perftime( perform *a_perf, Adjustment *a_hadjust ): DrawingArea() 
{     
    m_mainperf = a_perf;

    add_events( Gdk::BUTTON_PRESS_MASK |  
		Gdk::BUTTON_RELEASE_MASK );

    // in the construor you can only allocate colors, 
    // get_window() returns 0 because we have not be realized
   Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey = Gdk::Color( "grey" );
    
    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );

    m_hadjust = a_hadjust;

    m_4bar_offset = 0;


    m_snap = c_ppqn;
    m_measure_length = c_ppqn * 4;

    
    m_hadjust->signal_value_changed().connect( mem_fun( *this, &perftime::change_horz ));

    set_double_buffered( false );
} 


void
perftime::increment_size()
{

}

void 
perftime::update_sizes()
{
 
}

void 
perftime::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    
    set_size_request( 10, c_timearea_y );
}


void
perftime::change_horz( )
{
    if ( m_4bar_offset != (int) m_hadjust->get_value() ){
	
	m_4bar_offset = (int) m_hadjust->get_value();
	queue_draw();
    }
}

void
perftime::set_guides( int a_snap, int a_measure )
{
    m_snap = a_snap;
    m_measure_length = a_measure;
    queue_draw();
}

int 
perftime::idle_progress( )
{
    return true;
}



void 
perftime::update_pixmap()
{

}




void 
perftime::draw_pixmap_on_window()
{

}

bool
perftime::on_expose_event(GdkEventExpose* a_e)
{
    /* clear background */
    m_gc->set_foreground(m_white);
    m_window->draw_rectangle(m_gc,true,
			    0,
			    0, 
			    m_window_x, 
			    m_window_y );

    m_gc->set_foreground(m_black);
    m_window->draw_line(m_gc,
		       0,
		       m_window_y - 1,
		       m_window_x,
		       m_window_y - 1 );
    
    
    /* draw vert lines */
    m_gc->set_foreground(m_grey);

    long tick_offset = (m_4bar_offset * 16 * c_ppqn);
    long first_measure = tick_offset / m_measure_length;

#if 0

    0   1   2   3   4   5   6
    |   |   |   |   |   |   |
    |    |    |    |    |    |
    0    1    2    3    4    5

#endif
        
    for ( int i=first_measure;
              i<first_measure+(m_window_x * c_perf_scale_x / (m_measure_length)) + 1; i++ )
    {
        int x_pos = ((i * m_measure_length) - tick_offset) / c_perf_scale_x;
        
	/* beat */
	m_window->draw_line(m_gc,
			   x_pos,
			   0,
			   x_pos,
			   m_window_y );
	
	char bar[5];
	snprintf( bar, sizeof(bar), "%d", i + 1 ); 
	
	m_gc->set_foreground(m_black);

        p_font_renderer->render_string_on_drawable(m_gc,
                                                   x_pos + 2, 
                                                   0,
                                                   m_window, bar, font::BLACK );
    }

    long left = m_mainperf->get_left_tick( );
    long right = m_mainperf->get_right_tick( );

    left -= (m_4bar_offset * 16 * c_ppqn);
    left /= c_perf_scale_x;
    right -= (m_4bar_offset * 16 * c_ppqn);
    right /= c_perf_scale_x;

    if ( left >=0 && left <= m_window_x ){

	m_gc->set_foreground(m_black);
	m_window->draw_rectangle(m_gc,true,
				    left, m_window_y - 9, 
				    7, 
				    10 );

	m_gc->set_foreground(m_white);
    p_font_renderer->render_string_on_drawable(m_gc,
                                               left + 1,
                                               9,
                                               m_window, "L", font::WHITE );
    }

    if ( right >=0 && right <= m_window_x ){
	
	m_gc->set_foreground(m_black);
	m_window->draw_rectangle(m_gc,true,
				    right - 6, m_window_y - 9, 
				    7, 
				    10 );

	m_gc->set_foreground(m_white);
    p_font_renderer->render_string_on_drawable(m_gc,
                                               right - 6 + 1,
                                               9,
                                               m_window, "R", font::WHITE );

    }

    return true;
}

bool
perftime::on_button_press_event(GdkEventButton* p0)
{
    long tick = (long) p0->x;
    tick *= c_perf_scale_x;
    //tick = tick - (tick % (c_ppqn * 4));
    tick += (m_4bar_offset * 16 * c_ppqn);

    tick = tick - (tick % m_snap);
    
    //if ( p0->button == 2 )
    //m_mainperf->set_start_tick( tick );
    if ( p0->button == 1 )
    {
	m_mainperf->set_left_tick( tick );
    }
    if ( p0->button == 3 )
    {
	m_mainperf->set_right_tick( tick + m_snap );
    }
    
    queue_draw();


    return true;
}

bool
perftime::on_button_release_event(GdkEventButton* p0)
{
    return false;
}



void
perftime::on_size_allocate(Gtk::Allocation &a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
}
