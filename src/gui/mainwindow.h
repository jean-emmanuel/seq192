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


#ifndef SEQ192_MAINWINDOW
#define SEQ192_MAINWINDOW

#include <gtkmm.h>

#include "../core/globals.h"
#include "../lib/nsm.h"

#include "widgets.h"
#include "sequencebutton.h"
#include "editwindow.h"

using namespace Gtk;

enum main_menu_action
{
    MAIN_MENU_NEW = 0,
    MAIN_MENU_OPEN,
    MAIN_MENU_SAVE,
    MAIN_MENU_SAVEAS,
    MAIN_MENU_IMPORT,
    MAIN_MENU_EXPORT_SCREENSET,
    MAIN_MENU_EXPORT_SEQUENCE,
    MAIN_MENU_QUIT
};

class SequenceButton;
class EditWindow;
class MainWindow : public Window {

    public:

        MainWindow(perform * p, Glib::RefPtr<Gtk::Application> app);
        ~MainWindow();

        // nsm
        void nsm_set_client(nsm_client_t *nsm, bool optional_gui);

        Glib::RefPtr<Gtk::Application> m_app;

        sigc::signal<void(string name)> signal_hover;


    private:

        perform            *m_perform;

        Glib::RefPtr<Gtk::AccelGroup>          m_accelgroup;

        // layout
        VBox                m_vbox;
        HBox                m_toolbar;
        ScrolledWindow      m_scroll_wrapper;
        Grid                m_sequence_grid;
        SequenceButton     *m_sequences[c_seqs_in_set];
        SequenceButton     *m_sequence_focus = NULL;
        bool                m_sequence_keyboard_nav = false;
        EditWindow         *m_editwindows[c_max_sequence];

        // menu
        MenuBar             m_menu;
        MenuItem            m_menu_file;
        Menu                m_submenu_file;
        MenuItem            m_menu_file_new;
        MenuItem            m_menu_file_open;
        MenuItem            m_menu_file_save;
        MenuItem            m_menu_file_saveas;
        MenuItem            m_menu_file_import;
        MenuItem            m_menu_file_export;
        MenuItem            m_menu_file_quit;
        SeparatorMenuItem   m_menu_separator1;
        SeparatorMenuItem   m_menu_separator2;

        MenuItem            m_menu_edit;
        Menu                m_submenu_edit;
        MenuItem            m_menu_edit_undo;
        MenuItem            m_menu_edit_redo;

        MenuItem            m_menu_transport;
        Menu                m_submenu_transport;
        AccelLabel          m_menu_transport_start_label;
        MenuItem            m_menu_transport_start;
        AccelLabel          m_menu_transport_stop_label;
        MenuItem            m_menu_transport_stop;

        void menu_callback(main_menu_action action, int data1, int data2);

        // toolbar
        Button              m_toolbar_panic;
        Button              m_toolbar_stop;
        Button              m_toolbar_play;
        bool                m_toolbar_play_state;

        HBox                m_toolbar_bpm_box;
        CustomEntry         m_toolbar_bpm;
        double              m_toolbar_bpm_value;
        CustomButton        m_toolbar_bpm_minus;
        Image               m_toolbar_minus_icon;
        CustomButton        m_toolbar_bpm_plus;
        Image               m_toolbar_plus_icon;

        HBox                m_toolbar_sset_box;
        Entry               m_toolbar_sset_name;
        CustomEntry         m_toolbar_sset;
        int                 m_toolbar_sset_value;
        CustomButton        m_toolbar_sset_prev;
        Image               m_toolbar_prev_icon;
        CustomButton        m_toolbar_sset_next;
        Image               m_toolbar_next_icon;

        Image               m_toolbar_logo;
        Image               m_toolbar_play_icon;
        Image               m_toolbar_stop_icon;
        Image               m_toolbar_panic_icon;

        // misc
        string              m_hover;


        // drag and drop
        SequenceButton     *m_drag_source;
        SequenceButton     *m_drag_destination;
        bool                m_button_pressed;
        void set_drag_source(SequenceButton *s);
        void set_drag_destination(SequenceButton *s);

        // hovered sequence
        void set_focus_sequence(SequenceButton *s);
        SequenceButton* get_focus_sequence(){return m_sequence_focus;};

        // edit
        void open_edit_window(int seqnum, sequence * seq);
        void close_edit_window(int seqnum);
        void close_all_edit_windows();

        // misc
        bool timer_callback();
        bool unsaved_changes();
        void update_window_title();
        void update_sset_name(int sset);
        void clear_focus();
        bool on_key_press(GdkEventKey* event);
        bool on_delete_event(GdkEventAny *event);
        bool on_scroll_event(GdkEventScroll* event);
        bool on_motion_notify_event(GdkEventMotion* event);

        // nsm
        bool m_nsm_optional_gui;
        bool m_nsm_visible;
        bool m_nsm_dirty;
        nsm_client_t *m_nsm;

    friend class SequenceButton;
    friend class EditWindow;
};

#endif
