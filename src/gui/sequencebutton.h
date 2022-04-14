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


#ifndef SEQ192_SEQUENCEBUTTON
#define SEQ192_SEQUENCEBUTTON

#include <gtkmm.h>

#include "../core/globals.h"
#include "../core/perform.h"

#include "styles.h"
#include "mainwindow.h"

using namespace Gtk;

enum context_menu_action
{
    MENU_NEW = 0,
    MENU_EDIT,
    MENU_CUT,
    MENU_COPY,
    MENU_EXPORT,
    MENU_PASTE,
    MENU_MIDI_BUS
};

class MainWindow;
class SequenceButton : public DrawingArea {

    public:

        SequenceButton(perform * p, MainWindow * m, int seqpos);
        ~SequenceButton();

        sequence * get_sequence();
        int get_sequence_number();
        int get_last_sequence_number();
        void set_last_sequence_number();
        void draw_background();
        void update();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
        bool on_button_release_event(GdkEventButton* event);
        bool on_button_press_event(GdkEventButton* event);
        bool on_leave_notify_event(GdkEventCrossing* event);
        bool on_enter_notify_event(GdkEventCrossing* event);

    private:

        perform * m_perform;
        MainWindow * m_mainwindow;
        int m_seqpos;
        int m_last_seqnum;
        bool m_click;
        bool m_drag_start;
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        void menu_callback(context_menu_action action, int data1, int data2);

        int m_rect_x;
        int m_rect_y;
        int m_rect_w;
        int m_rect_h;

        int m_last_marker_pos;
        int m_next_marker_pos;

    friend class MainWindow;

};

#endif
