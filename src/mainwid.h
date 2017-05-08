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



#include "globals.h"
#include "perform.h"
#include "seqmenu.h"

class seqedit;

#ifndef SEQ24_MAINWID
#define SEQ24_MAINWID


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
#include <gtkmm/style.h> 



using namespace Gtk;


const int c_seqarea_seq_x = c_text_x * 13;
const int c_seqarea_seq_y = c_text_y * 2;

/* piano roll */
class mainwid : public Gtk::DrawingArea, public seqmenu
{

 private: 

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color   m_black, m_white, m_grey, m_dk_grey;
    Gdk::Color   m_background, m_foreground;

    Glib::RefPtr<Gdk::Pixmap>   m_pixmap;
   
    GdkRectangle m_old;
    GdkRectangle m_selected;
    
    int          m_screenset;

    perform      *m_mainperf;

    sequence     m_clipboard;
    sequence     m_moving_seq;

    int          m_window_x, 
                 m_window_y;

    bool         m_button_down;
    bool         m_moving;

    /* when highlighting a bunch of events */
 
    /* where the dragging started */
    int m_drop_x; 
    int m_drop_y;
    int m_current_x;
    int m_current_y;

    int m_old_seq;

    long m_last_tick_x[c_max_sequence];
    bool m_last_playing[c_max_sequence];
    static const char m_seq_to_char[c_seqs_in_set];

    void on_realize();

    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_p0);
    bool on_focus_in_event(GdkEventFocus*);
    bool on_focus_out_event(GdkEventFocus*);

    void draw_sequence_on_pixmap( int a_seq );
    void draw_sequences_on_pixmap();

    void fill_background_window();
    void draw_pixmap_on_window();
    void draw_sequence_pixmap_on_window( int a_seq );

    int seq_from_xy( int a_x, int a_y );

    int timeout( void );

    void redraw( int a_seq ); 

 public:

    mainwid( perform *a_p );
    ~mainwid( );

    void reset();
 
    //int get_screenset( ); 
    void set_screenset( int a_ss );

    void update_sequence_on_window( int a_seq  );
    void update_sequences_on_window( );
   
    void update_markers( int a_ticks );
    void draw_marker_on_sequence( int a_seq, int a_tick );

};

#endif
