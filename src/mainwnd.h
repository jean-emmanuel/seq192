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


#include "mainwid.h"
#include "perform.h"
#include "sequence.h"
#include "event.h"
#include "options.h"
#include "maintime.h"
#include "perfedit.h"
#include "options.h"



#ifndef SEQ24_MAINWINDOW
#define SEQ24_MAINWINDOW

#include <map>
#include <gtkmm.h>
#include <string>

#include "globals.h"



using namespace Gtk;


using namespace Menu_Helpers;


class mainwnd : public Gtk::Window, public performcallback
{
    /* notification handler for learn mode toggle */
    virtual void on_grouplearnchange(bool state);

 private:

    bool      m_modified;
    
#if GTK_MINOR_VERSION < 12
    Tooltips *m_tooltips;
#endif
    MenuBar  *m_menubar;
    Menu     *m_menu_file;
    Menu     *m_menu_view;
    Menu     *m_menu_help;

    perform  *m_mainperf;

    mainwid  *m_main_wid;
    maintime *m_main_time;

    perfedit *m_perf_edit;
    options *m_options;

    Gdk::Cursor   m_main_cursor;
    
    Button      *m_button_learn;

    Button      *m_button_stop;
    Button      *m_button_play;
    Button      *m_button_perfedit;

    SpinButton  *m_spinbutton_bpm;
    Adjustment  *m_adjust_bpm;

    SpinButton  *m_spinbutton_ss;
    Adjustment  *m_adjust_ss;

    SpinButton  *m_spinbutton_load_offset;
    Adjustment  *m_adjust_load_offset;

    Entry       *m_entry_notes;

    sigc::connection   m_timeout_connect;

    void file_import_dialog( void );
    void options_dialog( void );
    void about_dialog( void );

    void adj_callback_ss( );
    void adj_callback_bpm( );
    void edit_callback_notepad( );
    bool timer_callback( );

    void start_playing();
    void stop_playing();
    void learn_toggle();
    void open_performance_edit( );
    void sequence_key( int a_seq );
    void update_window_title();
    void toLower(basic_string<char>&);
    bool is_modified();
    void file_new();
    void file_open();
    void file_save();
    void file_save_as();
    void file_exit();
    void new_file();
    void open_file(const Glib::ustring&);
    bool save_file();
    void choose_file();
    int query_save_changes();
    bool is_save();

 public:

    mainwnd(perform *a_p);
    ~mainwnd();

    bool on_delete_event(GdkEventAny *a_e);
    bool on_key_press_event(GdkEventKey* a_ev);
    bool on_key_release_event(GdkEventKey* a_ev);


};


#endif
