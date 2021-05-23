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
#include "maintime.h"

class maintime;


#ifndef SEQ24_MAINWINDOW
#define SEQ24_MAINWINDOW

#include <map>
#include <gtkmm.h>
#include <string>

#include "globals.h"



using namespace Gtk;


using namespace Menu_Helpers;


class mainwnd : public Gtk::Window
{

 private:

    bool      m_modified;

    MenuBar  *m_menubar;
    Menu     *m_menu_file;
    Menu     *m_menu_view;
    Menu     *m_menu_help;

    Adjustment * m_hadjust;
    Adjustment * m_vadjust;
    HScrollbar * m_hscroll;
    VScrollbar * m_vscroll;
    void on_scrollbar_resize ();
    bool on_scroll_event (GdkEventScroll * ev);

    perform  *m_mainperf;

    mainwid  *m_main_wid;
    maintime *m_main_time;

    Gdk::Cursor   m_main_cursor;

    Button      *m_button_stop;
    Button      *m_button_play;

    SpinButton  *m_spinbutton_bpm;
    Adjustment  *m_adjust_bpm;

    SpinButton  *m_spinbutton_ss;
    Adjustment  *m_adjust_ss;

    SpinButton  *m_spinbutton_load_offset;
    Adjustment  *m_adjust_load_offset;

    Entry       *m_entry_notes;

    sigc::connection   m_timeout_connect;

    void file_import_dialog();
    void options_dialog();
    void about_dialog();

    void adj_callback_ss( );
    void adj_enter_callback_ss( );
    void adj_callback_bpm( );
    void adj_enter_callback_bpm( );
    bool edit_callback_notepad( GdkEventFocus *focus );
    void edit_enter_callback_notepad( );
    bool timer_callback( );

    void start_playing();
    void stop_playing();
    void update_window_title();
    void toLower(basic_string<char>&);
    bool is_modified();
    void file_new();
    void file_open();
    void file_save();
    void file_save_as(file_type_e type, int a_sset, int a_seq);
    void file_export(const Glib::ustring& fn, int a_sset, int a_seq);

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

    friend class seqmenu;

};


#endif
