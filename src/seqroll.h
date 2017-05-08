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


#ifndef SEQ24_SEQROLL
#define SEQ24_SEQROLL

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
#include "seqdata.h"
#include "seqevent.h"
#include "perform.h"


using namespace Gtk;

class rect
{
 public:
    int x, y, height, width;
};

class seqroll;
struct FruitySeqRollInput
{
    FruitySeqRollInput() : m_adding( false ), m_canadd( true ), m_erase_painting( false )
    {}
    bool on_button_press_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_button_release_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_motion_notify_event(GdkEventMotion* a_ev, seqroll& ths);
    void updateMousePtr(seqroll& ths);
    bool m_adding;
    bool m_canadd;
    bool m_erase_painting;
    long m_drag_paste_start_pos[2];
};
struct Seq24SeqRollInput
{
    Seq24SeqRollInput() : m_adding( false )
    {}
    bool on_button_press_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_button_release_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_motion_notify_event(GdkEventMotion* a_ev, seqroll& ths);
    void set_adding( bool a_adding, seqroll& ths );
    bool m_adding;
};


/* piano roll */
class seqroll : public Gtk::DrawingArea
{

 private: 
    friend struct FruitySeqRollInput;
    FruitySeqRollInput m_fruity_interaction;

    friend struct Seq24SeqRollInput;
    Seq24SeqRollInput m_seq24_interaction;


    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window>   m_window;
    Gdk::Color    m_black, m_white, m_grey, m_dk_grey, m_red;

    Glib::RefPtr<Gdk::Pixmap> m_pixmap;
    Glib::RefPtr<Gdk::Pixmap> m_background;
 
    rect         m_old;
    rect         m_selected;

    sequence     *m_seq;
    sequence     *m_clipboard;
    perform      *m_perform;
    seqdata      *m_seqdata_wid;
    seqevent     *m_seqevent_wid;
    seqkeys      *m_seqkeys_wid;

    int m_pos;

    /* one pixel == m_zoom ticks */
    int          m_zoom;
    int          m_snap;
    int          m_note_length;

    int          m_scale;
    int          m_key;

    int m_window_x, m_window_y;

	/* what is the data window currently editing ? */
    unsigned char m_status;
    unsigned char m_cc;

    /* when highlighting a bunch of events */
    bool m_selecting;
    bool m_moving;
    bool m_moving_init;
    bool m_growing;
    bool m_painting;
    bool m_paste;
    bool m_is_drag_pasting;
    bool m_is_drag_pasting_start;
    bool m_justselected_one;

    /* where the dragging started */
    int m_drop_x; 
    int m_drop_y;
    int m_move_delta_x;
    int m_move_delta_y;
    int m_current_x;
    int m_current_y;

    int m_move_snap_offset_x;

    int m_old_progress_x;

    Adjustment   *m_vadjust;
    Adjustment   *m_hadjust;

    int m_scroll_offset_ticks;
    int m_scroll_offset_key;

    int m_scroll_offset_x;
    int m_scroll_offset_y;

    int m_background_sequence;
    bool m_drawing_background_seq;

    bool m_ignore_redraw;
    
    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev); 
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_ev);
    bool on_key_press_event(GdkEventKey* a_p0);
    bool on_focus_in_event(GdkEventFocus*);
    bool on_focus_out_event(GdkEventFocus*);
    bool on_scroll_event( GdkEventScroll* a_ev);

    bool on_leave_notify_event	(GdkEventCrossing* a_p0);
    bool on_enter_notify_event	(GdkEventCrossing* a_p0);


    void convert_xy( int a_x, int a_y, long *a_ticks, int *a_note);
    void convert_tn( long a_ticks, int a_note, int *a_x, int *a_y);

    void snap_y( int *a_y );
    void snap_x( int *a_x );

    void xy_to_rect( int a_x1,  int a_y1,
		     int a_x2,  int a_y2,
		     int *a_x,  int *a_y,
		     int *a_w,  int *a_h );

    void convert_tn_box_to_rect( long a_tick_s, long a_tick_f,
				 int a_note_h, int a_note_l,
				 int *a_x, int *a_y, 
				 int *a_w, int *a_h );
	
    void draw_events_on(  Glib::RefPtr<Gdk::Drawable> a_draw );

  
    int idle_progress();
    
    void on_size_allocate(Gtk::Allocation& );

    void change_horz( void );
    void change_vert( void );

    void force_draw( void );


 public:

    void reset();
    void redraw();
    void redraw_events();
    void set_zoom( int a_zoom );
    void set_snap( int a_snap );
	void set_note_length( int a_note_length );
    void set_ignore_redraw(bool a_ignore);
    
    void set_scale( int a_scale );
    void set_key( int a_key );
    
    void update_sizes();
    void update_background();
    void draw_background_on_pixmap();
    void draw_events_on_pixmap();
    void draw_selection_on_window();
    void update_pixmap();
    int idle_redraw();

    void draw_progress_on_window();

    void start_paste( );
    
    void set_background_sequence( bool a_state, int a_seq );

    seqroll( perform *a_perf,
             sequence *a_seq, int a_zoom, int a_snap, 
             seqdata *a_seqdata_wid, 
             seqevent *a_seqevent_wid,
             seqkeys *a_seqkeys_wid, 
             int a_pos, 
             Adjustment *a_hadjust,
             Adjustment *a_vadjust );

    void set_data_type( unsigned char a_status, unsigned char a_control  );
 
    ~seqroll( );
};

#endif
