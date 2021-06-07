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
    EDIT_MENU_RECORD_THRU
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
        Label               m_dummy1;
        Label               m_dummy2;

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
        MenuItem            m_menu_edit_close;

        MenuItem            m_menu_edit_select;
        Menu                m_submenu_select;
        MenuItem            m_menu_edit_selectall;
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
        MenuItem            m_menu_edit_movefineleft;
        MenuItem            m_menu_edit_movefineright;

        MenuItem            m_menu_edit_quantize;

        MenuItem            m_menu_record;
        Menu                m_submenu_record;
        MenuItem            m_menu_record_recording;
        CheckMenuItem       m_menu_record_quantized;
        CheckMenuItem       m_menu_record_through;
        bool                m_menu_record_state;

        SeparatorMenuItem   m_menu_separator0;
        SeparatorMenuItem   m_menu_separator1;
        SeparatorMenuItem   m_menu_separator2;

        // toolbar
        Entry               m_toolbar_name;
        Entry               m_toolbar_bpm;
        Label               m_toolbar_slash;
        Entry               m_toolbar_bw;
        Label               m_toolbar_times;
        Entry               m_toolbar_measures;
        Button              m_toolbar_snap_label;
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

        void update_midibus_name();
        void create_midibus_menu();

        void menu_callback(edit_menu_action action);
        void menu_callback(edit_menu_action action, double data1);

        bool timer_callback();

        bool scroll_callback(GdkEventScroll* event);
        void focus_callback(string name);

        void clear_focus();

        bool on_key_press(GdkEventKey* event);
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
