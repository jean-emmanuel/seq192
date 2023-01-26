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


#include "widgets.h"
#include "mainwindow.h"

bool
CustomButton::on_enter_notify_event(GdkEventCrossing* event) {
    ((MainWindow*)get_toplevel())->signal_hover.emit((string)get_name());
    return Gtk::Widget::on_enter_notify_event(event);
}
bool
CustomButton::on_leave_notify_event(GdkEventCrossing* event) {
    ((MainWindow*)get_toplevel())->signal_hover.emit((string)"");
    return Gtk::Widget::on_leave_notify_event(event);
}


bool
CustomEntry::on_enter_notify_event(GdkEventCrossing* event) {
    ((MainWindow*)get_toplevel())->signal_hover.emit((string)get_name());
    return Gtk::Widget::on_enter_notify_event(event);
}
bool
CustomEntry::on_leave_notify_event(GdkEventCrossing* event) {
    ((MainWindow*)get_toplevel())->signal_hover.emit((string)"");
    return Gtk::Widget::on_leave_notify_event(event);
}


CustomHBox::CustomHBox()
{
    add(m_box);
    add_events(
        Gdk::ENTER_NOTIFY_MASK |
        Gdk::LEAVE_NOTIFY_MASK
    );
}
void
CustomHBox::pack_start(Widget &w, bool a, bool b) {
    m_box.pack_start(w, a, b);
}
void
CustomHBox::pack_end(Widget &w, bool a, bool b) {
    m_box.pack_end(w, a, b);
}
bool
CustomHBox::on_enter_notify_event(GdkEventCrossing* event) {
    get_style_context()->add_class("hover");
    return Gtk::Widget::on_enter_notify_event(event);
}
bool
CustomHBox::on_leave_notify_event(GdkEventCrossing* event) {
    get_style_context()->remove_class("hover");
    return Gtk::Widget::on_leave_notify_event(event);
}
