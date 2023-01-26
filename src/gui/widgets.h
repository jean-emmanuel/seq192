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


#ifndef SEQ192_GTKWIDGETS
#define SEQ192_GTKWIDGETS

#include <gtkmm.h>
#include "styles.h"

using namespace Gtk;


class CustomButton : public Button {
    // Button that notifies parent window when hovered

    private:
        bool on_enter_notify_event(GdkEventCrossing* event);
        bool on_leave_notify_event(GdkEventCrossing* event);

};

class CustomEntry : public Entry {
    // Entry that notifies parent window when hovered

    private:
        bool on_enter_notify_event(GdkEventCrossing* event);
        bool on_leave_notify_event(GdkEventCrossing* event);

};


class CustomHBox : public EventBox {
    // HBox with css :hover state as .hover
    // And settable color

    public:
        CustomHBox();
        void pack_start(Widget &w, bool a, bool b);
        void pack_end(Widget &w, bool a, bool b);
        void set_color(color * c);


    private:

        color * m_color;
        HBox m_box;
        bool on_enter_notify_event(GdkEventCrossing* event);
        bool on_leave_notify_event(GdkEventCrossing* event);

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

};

#endif
