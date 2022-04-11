// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SEQ192_EDITWINDOW
#define SEQ192_EDITWINDOW

#include <gtkmm.h>

#include "../core/globals.h"
#include "../core/mutex.h"

#include "styles.h"
#include "mainwindow.h"
#include "sequencebutton.h"
#include "timeroll.h"
#include "pianoroll.h"
#include "eventroll.h"
#include "pianokeys.h"
#include "dataroll.h"

using namespace Gtk;

enum edit_menu_action
{
    EDIT_MENU_UNDO = 0,
    EDIT_MENU_REDO,
    EDIT_MENU_CUT,
    EDIT_MENU_COPY,
    EDIT_MENU_PASTE,
    EDIT_MENU_DELETE,
    EDIT_MENU_INVERT,
    EDIT_MENU_SELECTALL,
    EDIT_MENU_UNSELECT,
    EDIT_MENU_TRANSPOSE,
    EDIT_MENU_QUANTIZE,
    EDIT_MENU_MOVE,
    EDIT_MENU_MULTIPLY,
    EDIT_MENU_REVERSE,
    EDIT_MENU_CLOSE,

    EDIT_MENU_RECORD,
    EDIT_MENU_RECORD_QUANTIZED,
    EDIT_MENU_RECORD_THRU,

    EDIT_MENU_PLAY,
    EDIT_MENU_RESUME
};

class MainWindow;
class EditWindow : public Window {

    public:

        EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq);
        ~EditWindow();

    private:

        perform            *m_perform;
        sequence           *m_sequence;
        MainWindow         *m_mainwindow;
        int                 m_seqnum;
        int                 m_bg_seqnum;

        Glib::RefPtr<Gtk::AccelGroup>          m_accelgroup;
        Glib::RefPtr<Gtk::AccelGroup>          m_accelgroup_persistent;

        // components
        PianoKeys           m_pianokeys;
        EventRoll           m_eventroll;
        TimeRoll            m_timeroll;
        PianoRoll           m_pianoroll;
        DataRoll            m_dataroll;

        string              m_focus;

        // layout
        VBox                m_vbox;
        HBox                m_toolbar;
        Grid                m_grid;
        HBox                m_hbox;
        VBox                m_left_vbox;
        VBox                m_right_vbox;
        ScrolledWindow      m_hscroll_wrapper;
        VBox                m_hscroll_vbox;
        ScrolledWindow      m_pianokeys_scroller;
        ScrolledWindow      m_pianoroll_scroller;
        Scrollbar           m_vscrollbar;
        Scrollbar           m_hscrollbar;
        Label               m_event_dropdown_label;
        MenuButton          m_event_dropdown;

        // menu
        MenuBar             m_menu;
        MenuItem            m_menu_edit;
        Menu                m_submenu_edit;
        MenuItem            m_menu_edit_undo;
        MenuItem            m_menu_edit_redo;
        MenuItem            m_menu_edit_cut;
        MenuItem            m_menu_edit_copy;
        MenuItem            m_menu_edit_paste;
        MenuItem            m_menu_edit_delete;
        AccelLabel          m_menu_edit_delete_label;
        MenuItem            m_menu_edit_close;

        MenuItem            m_menu_edit_select;
        Menu                m_submenu_select;
        MenuItem            m_menu_edit_selectall;
        MenuItem            m_menu_edit_invert;
        MenuItem            m_menu_edit_unselect;

        MenuItem            m_menu_edit_transpose;
        Menu                m_submenu_transpose;
        MenuItem            m_menu_edit_transpose_up;
        MenuItem            m_menu_edit_transpose_down;
        MenuItem            m_menu_edit_transpose_octup;
        MenuItem            m_menu_edit_transpose_octdown;

        MenuItem            m_menu_edit_pattern;
        Menu                m_submenu_pattern;
        MenuItem            m_menu_edit_expand;
        MenuItem            m_menu_edit_compress;
        MenuItem            m_menu_edit_reverse;

        MenuItem            m_menu_edit_move;
        Menu                m_submenu_move;
        MenuItem            m_menu_edit_moveleft;
        MenuItem            m_menu_edit_moveright;
        AccelLabel          m_menu_edit_movefineleft_label;
        MenuItem            m_menu_edit_movefineleft;
        AccelLabel          m_menu_edit_movefineright_label;
        MenuItem            m_menu_edit_movefineright;

        MenuItem            m_menu_edit_quantize;

        MenuItem            m_menu_view;
        Menu                m_submenu_view;
        MenuItem            m_menu_view_bg_sequence;
        Menu                m_submenu_bg_sequence;
        CheckMenuItem      *m_menu_items_bgseq[c_seqs_in_set + 1];

        MenuItem            m_menu_record;
        Menu                m_submenu_record;
        MenuItem            m_menu_record_recording;
        CheckMenuItem       m_menu_record_quantized;
        CheckMenuItem       m_menu_record_through;
        bool                m_menu_record_state;

        SeparatorMenuItem   m_menu_separator0;
        SeparatorMenuItem   m_menu_separator1;
        SeparatorMenuItem   m_menu_separator2;

        MenuItem            m_menu_transport;
        Menu                m_submenu_transport;
        AccelLabel          m_menu_transport_start_label;
        MenuItem            m_menu_transport_start;
        AccelLabel          m_menu_transport_stop_label;
        MenuItem            m_menu_transport_stop;

        MenuItem            m_menu_playback;
        Menu                m_submenu_playback;
        MenuItem            m_menu_playback_playing;
        bool                m_menu_playing_state;
        CheckMenuItem       m_menu_playback_resume;

        // event menu
        Menu                m_event_menu;
        CheckMenuItem       m_menu_item_noteon;
        CheckMenuItem       m_menu_item_noteoff;
        CheckMenuItem       m_menu_item_aftertouch;
        CheckMenuItem       m_menu_item_program;
        CheckMenuItem       m_menu_item_pitch;
        CheckMenuItem       m_menu_item_pressure;
        MenuItem            m_menu_item_control;
        Menu                m_submenu_control;
        CheckMenuItem       *m_menu_items_control[128];
        MenuItem            m_menu_item_alt_control;
        Menu                m_submenu_alt_control;
        CheckMenuItem       *m_menu_items_alt_control[129];
        MenuItem            m_menu_item_toggle_alt_control;

        // toolbar
        Entry               m_toolbar_name;
        Entry               m_toolbar_bpm;
        Label               m_toolbar_slash;
        Entry               m_toolbar_bw;
        Label               m_toolbar_times;
        Entry               m_toolbar_measures;
        ToggleButton        m_toolbar_snap_active;
        ComboBoxText        m_toolbar_snap;
        Button              m_toolbar_length_label;
        ComboBoxText        m_toolbar_length;
        Button              m_toolbar_bus_label;
        Entry               m_toolbar_bus;
        MenuButton          m_toolbar_bus_dropdown;
        Menu                m_toolbar_bus_menu;


        int                 m_bpm;
        int                 m_bw;
        int                 m_measures;

        int                 m_midibus;
        int                 m_midichannel;

        unsigned char m_status;
        unsigned char m_cc;

        unsigned char m_alt_status;
        unsigned char m_alt_cc;
        bool m_alt_control_view;

        void update_midibus_name();
        void create_midibus_menu();
        void create_event_menu();
        void update_event_menu();

        void update_background_menu();
        void set_background_sequence(int i, sequence * seq);

        void set_data_type(unsigned char status, unsigned char control = 0, bool alt=false);

        void menu_callback(edit_menu_action action);
        void menu_callback(edit_menu_action action, double data1);

        bool timer_callback();

        bool scroll_callback(GdkEventScroll* event);
        void focus_callback(string name);

        void update_hscrollbar_visibility();
        void update_window_title();

        void clear_focus();

        void on_focus_out();
        bool on_key_press(GdkEventKey* event);
        bool on_key_release(GdkEventKey* event);
        bool on_delete_event(GdkEventAny *event);


        std::map<std::string, int> divs_to_ticks = {
            {"1",       c_ppqn * 4},
            {"1/2",     c_ppqn * 2},
            {"1/4",     c_ppqn * 1},
            {"1/8",     c_ppqn / 2},
            {"1/16",    c_ppqn / 4},
            {"1/32",    c_ppqn / 8},
            {"1/64",    c_ppqn / 16},
            {"1/128",   c_ppqn / 32},
            {"1/3",     c_ppqn * 4 / 3},
            {"1/6",     c_ppqn * 2 / 3},
            {"1/12",    c_ppqn * 1 / 3},
            {"1/24",    c_ppqn / 2 / 3},
            {"1/48",    c_ppqn / 4 / 3},
            {"1/96",    c_ppqn / 8 / 3},
            {"1/192",   c_ppqn / 16 / 3}
        };
        std::map<int, int> ticks_to_divs = {
            {c_ppqn * 4,        0},
            {c_ppqn * 2,        1},
            {c_ppqn * 1,        2},
            {c_ppqn / 2,        3},
            {c_ppqn / 4,        4},
            {c_ppqn / 8,        5},
            {c_ppqn / 16,       6},
            {c_ppqn / 32,       7},
            {c_ppqn * 4 / 3,    8},
            {c_ppqn * 2 / 3,    9},
            {c_ppqn * 1 / 3,    10},
            {c_ppqn / 2 / 3,    11},
            {c_ppqn / 4 / 3,    12},
            {c_ppqn / 8 / 3,    13},
            {c_ppqn / 16 / 3,   14}
        };

};

#endif
