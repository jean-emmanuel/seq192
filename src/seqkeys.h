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


#ifndef SEQ24_SEQKEYS
#define SEQ24_SEQKEYS

#include "sequence.h"

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

/* holds the left side piano */
class seqkeys : public Gtk::DrawingArea
{
 private: 

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color    m_black, m_white, m_grey;

    Glib::RefPtr<Gdk::Pixmap> m_pixmap;
    
    sequence *m_seq;

    Gtk::Adjustment   *m_vadjust;
    
    int m_scroll_offset_key;
    int m_scroll_offset_y;

    int m_window_x;
    int m_window_y;
    
    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_p0);
    bool on_leave_notify_event(GdkEventCrossing* p0);
    bool on_enter_notify_event(GdkEventCrossing* p0);
    bool on_scroll_event( GdkEventScroll* a_ev);

    void draw_area();
    void update_pixmap();
    void convert_y( int a_y, int *a_note);

    bool m_hint_state;
    int m_hint_key;

    bool m_keying;
    int m_keying_note;

    int          m_scale;
    int          m_key;

    void draw_key( int a_key, bool a_state );
    void on_size_allocate(Gtk::Allocation&);
    
    void change_vert( void );
    void force_draw( void );

    void update_sizes( void );

    void reset();

public:

    /* sets key to grey */
    void set_hint_key( int a_key );

    /* true == on, false == off */
    void set_hint_state( bool a_state );

    seqkeys( sequence *a_seq,
             Gtk::Adjustment *a_vadjust );

    
    void set_scale( int a_scale );
    void set_key( int a_key );


};

#endif
