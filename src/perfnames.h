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


#ifndef SEQ24_PERFNAME
#define SEQ24_PERFNAME

#include "perform.h"
#include "sequence.h"
#include "seqmenu.h"

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/widget.h>
#include <gtkmm/adjustment.h> 

using namespace Gtk;

#include "globals.h"

/* holds the left side piano */
class perfnames : public virtual Gtk::DrawingArea, public virtual seqmenu
{
 private: 

    Glib::RefPtr<Gdk::GC>       m_gc;
    Glib::RefPtr<Gdk::Window>   m_window;
    Gdk::Color    m_black, m_white, m_grey;

    Glib::RefPtr<Gdk::Pixmap>   m_pixmap;
   
    perform      *m_mainperf;

    Adjustment   *m_vadjust;

    int m_window_x, m_window_y;

    int          m_sequence_offset;

    bool         m_sequence_active[c_total_seqs];

    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    void on_size_allocate(Gtk::Allocation& );
    bool on_scroll_event( GdkEventScroll* a_ev ) ;

    void draw_area();
    void update_pixmap();
 
    void convert_y( int a_y, int *a_note);

    void draw_sequence( int a_sequence );

    void change_vert( void );
    
    void redraw( int a_sequence );

 public:
    
    void redraw_dirty_sequences( void );

    perfnames( perform *a_perf,
	       Adjustment *a_vadjust   );


};

#endif
