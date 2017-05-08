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


#ifndef SEQ24_PERFTIME
#define SEQ24_PERFTIME

#include "perform.h"
#include "seqtime.h"

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

using namespace Gtk;

/* piano time*/
class perftime: public Gtk::DrawingArea
{

 private: 

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window>   m_window;
    Gdk::Color    m_black, m_white, m_grey;

    Glib::RefPtr<Gdk::Pixmap> m_pixmap;


    perform      *m_mainperf;

    Adjustment   *m_hadjust;

    int m_window_x, m_window_y;

    int m_4bar_offset;

    int m_snap, m_measure_length;

    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    void on_size_allocate(Gtk::Allocation &a_r );

    void update_sizes();
    void draw_pixmap_on_window();
    void draw_progress_on_window();
    void update_pixmap();

    int idle_progress();

    void change_horz( void );

 public:

    perftime( perform *a_perf, Adjustment *a_hadjust );

    void reset();
    void set_scale( int a_scale );
    void set_guides( int a_snap, int a_measure );

    void increment_size();
};

#endif
