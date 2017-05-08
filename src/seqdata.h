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


#ifndef SEQ24_SEQDATA
#define SEQ24_SEQDATA

#include "sequence.h"
#include "seqkeys.h"

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

#include "globals.h"

/* piano event */
class seqdata : public Gtk::DrawingArea
{

 private: 

    Glib::RefPtr<Gdk::GC>     m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color    m_black, m_white, m_grey;

    Glib::RefPtr<Gdk::Pixmap>   m_pixmap;
    Glib::RefPtr<Gdk::Pixmap>   m_numbers[c_dataarea_y];
 
    
    sequence     *m_seq;

    /* one pixel == m_zoom ticks */
    int          m_zoom;

    int m_window_x, m_window_y;

    int m_drop_x, m_drop_y;
    int m_current_x, m_current_y;


    Gtk::Adjustment   *m_hadjust;

    int m_scroll_offset_ticks;
    int m_scroll_offset_x;

    int m_background_tile_x;
    int m_background_tile_y;

    /* what is the data window currently editing ? */
    unsigned char m_status;
    unsigned char m_cc;

    GdkRectangle m_old;

    bool m_dragging;

    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);

    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_p0);
    bool on_leave_notify_event(GdkEventCrossing* p0);
    bool on_scroll_event( GdkEventScroll* a_ev ) ;
    
    void update_sizes();
    void draw_events_on_pixmap();
    void draw_pixmap_on_window();
    void update_pixmap();
    void draw_line_on_window();

    void convert_x( int a_x, long *a_tick );

    void xy_to_rect( int a_x1,  int a_y1,
		     int a_x2,  int a_y2,
		     int *a_x,  int *a_y,
		     int *a_w,  int *a_h );
    
    void draw_events_on( Glib::RefPtr<Gdk::Drawable> a_draw );

    void on_size_allocate(Gtk::Allocation& );    
    void change_horz( void );

    void force_draw( void );

 public:
    
    seqdata( sequence *a_seq, int a_zoom,  Gtk::Adjustment   *a_hadjust );

    void reset();
    void redraw();
    void set_zoom( int a_zoom );
    void set_data_type( unsigned char a_status, unsigned char a_control  );

    int idle_redraw();

    friend class seqroll;
    friend class seqevent;

};

#endif
