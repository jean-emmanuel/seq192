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

#include "maintime.h"


maintime::maintime( ): DrawingArea() 
{     
    // in the construor you can only allocate colors, 
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

    m_black = Gdk::Color( "black" );
    m_white = Gdk::Color( "white" );
    m_grey  = Gdk::Color( "grey" );

    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );

    m_tick = 0;
} 

void 
maintime::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    
    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    /* set default size */
    set_size_request( c_maintime_x , c_maintime_y );
 
}


int 
maintime::idle_progress( long a_ticks )
{
  m_tick = a_ticks;

  m_window->clear();

  m_gc->set_foreground(m_black);
  m_window->draw_rectangle(m_gc,false,
			  0,
			  0, 
			  c_maintime_x - 1, 
			  c_maintime_y - 1  );

  int width = c_maintime_x - 1 - c_pill_width;

  int tick_x = ((m_tick % c_ppqn) * (c_maintime_x - 1) ) / c_ppqn ;
  int beat_x = (((m_tick / 4) % c_ppqn) * width) / c_ppqn ;
  int bar_x = (((m_tick / 16) % c_ppqn) * width) / c_ppqn ;

  if ( tick_x <= (c_maintime_x / 4 )){

    m_gc->set_foreground(m_grey);
    m_window->draw_rectangle(m_gc,true,
			    2, //tick_x + 2,
			    2, 
			    c_maintime_x - 4, 
			    c_maintime_y - 4  );
  }

  

  m_gc->set_foreground(m_black);
  m_window->draw_rectangle(m_gc,true,
			  beat_x + 2,
			  2, 
			  c_pill_width, 
			  c_maintime_y - 4  );
  
  m_window->draw_rectangle(m_gc,true,
			  bar_x + 2,
			  2, 
			  c_pill_width, 
			  c_maintime_y - 4  );

  return true;
}





bool
maintime::on_expose_event(GdkEventExpose* a_e)
{

  idle_progress( m_tick );
  return true;
}
