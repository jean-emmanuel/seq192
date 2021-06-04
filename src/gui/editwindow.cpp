#include "editwindow.h"
#include "pianoroll.h"
#include "../core/globals.h"

EditWindow::EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq) :
    m_perform(p),
    m_sequence(seq),
    m_mainwindow(m),
    m_seqnum(seqnum),
    m_pianokeys(p, seq),
    m_eventroll(p, seq),
    m_pianoroll(p, seq, &m_pianokeys),
    m_dataroll(p, seq)
{

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_class("editwindow");
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_vbox);

    // menu bar
    m_menu_edit.set_label("_Edit");
    m_menu_edit.set_use_underline(true);
    m_menu_edit.set_submenu(m_submenu_edit);
    m_menu.append(m_menu_edit);

    m_menu_edit_undo.set_label("_Undo");
    m_menu_edit_undo.set_use_underline(true);
    m_menu_edit_undo.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'z', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_undo.signal_activate().connect([this]{menu_callback(EDIT_MENU_UNDO);});
    m_submenu_edit.append(m_menu_edit_undo);

    m_menu_edit_redo.set_label("_Redo");
    m_menu_edit_redo.set_use_underline(true);
    m_menu_edit_redo.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'y', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_redo.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'z', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_redo.signal_activate().connect([this]{menu_callback(EDIT_MENU_REDO);});
    m_submenu_edit.append(m_menu_edit_redo);

    m_submenu_edit.append(m_menu_separator0);

    m_menu_edit_cut.set_label("_Cut");
    m_menu_edit_cut.set_use_underline(true);
    m_menu_edit_cut.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'x', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_cut.signal_activate().connect([this]{menu_callback(EDIT_MENU_CUT);});
    m_submenu_edit.append(m_menu_edit_cut);

    m_menu_edit_copy.set_label("_Copy");
    m_menu_edit_copy.set_use_underline(true);
    m_menu_edit_copy.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'c', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_copy.signal_activate().connect([this]{menu_callback(EDIT_MENU_COPY);});
    m_submenu_edit.append(m_menu_edit_copy);

    m_menu_edit_paste.set_label("_Paste");
    m_menu_edit_paste.set_use_underline(true);
    m_menu_edit_paste.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'v', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_paste.signal_activate().connect([this]{menu_callback(EDIT_MENU_PASTE);});
    m_submenu_edit.append(m_menu_edit_paste);

    m_menu_edit_delete.set_label("Delete");
    m_menu_edit_delete.set_use_underline(true);
    m_menu_edit_delete.Gtk::Widget::add_accelerator("activate", get_accel_group(), GDK_KEY_Delete, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_edit_delete.signal_activate().connect([this]{menu_callback(EDIT_MENU_DELETE);});
    m_submenu_edit.append(m_menu_edit_delete);

    m_submenu_edit.append(m_menu_separator1);

    m_menu_edit_selectall.set_label("Select _All");
    m_menu_edit_selectall.set_use_underline(true);
    m_menu_edit_selectall.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'a', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_selectall.signal_activate().connect([this]{menu_callback(EDIT_MENU_SELECTALL);});
    m_submenu_edit.append(m_menu_edit_selectall);

    m_menu_edit_unselect.set_label("_Unselect");
    m_menu_edit_unselect.set_use_underline(true);
    m_menu_edit_unselect.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'a', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_unselect.signal_activate().connect([this]{menu_callback(EDIT_MENU_UNSELECT);});
    m_submenu_edit.append(m_menu_edit_unselect);

    m_submenu_edit.append(m_menu_separator2);

    m_menu_edit_close.set_label("_Close");
    m_menu_edit_close.set_use_underline(true);
    m_menu_edit_close.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'w', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_close.signal_activate().connect([this]{menu_callback(EDIT_MENU_CLOSE);});
    m_submenu_edit.append(m_menu_edit_close);

    // layout
    m_vbox.pack_start(m_menu, false, false);
    m_vbox.pack_end(m_grid, true, true);

    m_grid.insert_row(0);
    m_grid.insert_row(0);
    m_grid.insert_column(0);
    m_grid.insert_column(0);
    m_grid.insert_column(0);
    m_pianokeys_scroller.set_size_request(c_keys_width, -1);
    m_pianokeys.set_size_request(-1, c_key_height * c_num_keys);
    m_pianokeys_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianokeys_scroller.add(m_pianokeys);
    m_grid.attach(m_pianokeys_scroller, 0, 0);

    m_pianoroll_scroller.set_hexpand(true);
    m_pianoroll_scroller.set_vexpand(true);
    m_pianoroll.set_size_request(-1, c_key_height * c_num_keys);
    m_pianoroll_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianoroll_scroller.set_vadjustment(m_pianokeys_scroller.get_vadjustment());
    m_pianoroll_scroller.add(m_pianoroll);
    m_grid.attach(m_pianoroll_scroller, 1, 0);

    m_vscrollbar.set_orientation(ORIENTATION_VERTICAL);
    m_vscrollbar.set_adjustment(m_pianokeys_scroller.get_vadjustment());
    m_grid.attach(m_vscrollbar, 2, 0);

    m_eventroll.set_size_request(-1, c_eventroll_height + 1);
    m_grid.attach(m_eventroll, 1, 1);

    m_dataroll.set_size_request(-1, c_dataroll_height + 1);
    m_grid.attach(m_dataroll, 1, 2);

    m_grid.attach(m_hscrollbar, 1, 3);

    int height = c_key_height * c_num_keys;
    m_pianokeys_scroller.get_vadjustment()->configure((height - 500) / 2.0, 0.0, height, c_key_height, c_key_height * 12, 1);

    m_hscrollbar.get_adjustment()->configure(0, 0, m_sequence->get_length(), 1, 1, 1);


    // timer callback (50 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &EditWindow::timer_callback), 20);

    // zoom callback
    m_pianoroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_eventroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_pianoroll.signal_focus.connect(mem_fun(*this, &EditWindow::focus_callback));
    m_eventroll.signal_focus.connect(mem_fun(*this, &EditWindow::focus_callback));

    // add_events(Gdk::SCROLL_MASK);

    resize(800, 600);
    show_all();


}

EditWindow::~EditWindow()
{
    m_sequence->set_recording(false);
    m_perform->get_master_midi_bus()->set_sequence_input(false, NULL);
}

bool
EditWindow::on_delete_event(GdkEventAny *event)
{
    m_mainwindow->close_edit_window(m_seqnum);
    delete this;
    return false;
}

void
EditWindow::menu_callback(edit_menu_action action)
{
    switch (action) {
        case EDIT_MENU_UNDO:
            m_sequence->pop_undo();
            break;
        case EDIT_MENU_REDO:
            m_sequence->pop_redo();
            break;
        case EDIT_MENU_CUT:
            if(m_sequence->mark_selected())
            {
                m_sequence->push_undo();
                m_sequence->copy_selected();
                m_sequence->remove_marked();
            }
            break;
        case EDIT_MENU_COPY:
            m_sequence->copy_selected();
            break;
        case EDIT_MENU_PASTE:
            if (m_focus == "eventroll") {
                m_eventroll.start_paste();
            } else {
                m_pianoroll.start_paste();
            }
            break;
        case EDIT_MENU_DELETE:
            if(m_sequence->mark_selected())
            {
                m_sequence->push_undo();
                m_sequence->remove_marked();
            }
            break;
        case EDIT_MENU_SELECTALL:
            m_sequence->select_all();
            break;
        case EDIT_MENU_UNSELECT:
            m_sequence->unselect();
            break;
        case EDIT_MENU_CLOSE:
            close();
            break;
    }
}

bool
EditWindow::timer_callback()
{
    auto adj = m_hscrollbar.get_adjustment();
    adj->set_lower(0);
    adj->set_upper(m_sequence->get_length());
    adj->set_page_size(m_pianoroll.get_width() * m_pianoroll.get_zoom());
    adj->set_step_increment(c_ppqn / 4 * m_pianoroll.get_zoom());
    adj->set_page_increment(c_ppqn * m_sequence->get_bpm() * 4.0 / m_sequence->get_bw() * m_pianoroll.get_zoom());
    if (adj->get_value() > adj->get_upper()) adj->set_value(adj->get_upper());

    m_eventroll.set_hscroll(adj->get_value());
    m_pianoroll.set_hscroll(adj->get_value());

    m_eventroll.queue_draw();
    m_pianoroll.queue_draw();

    return true;
}


bool
EditWindow::scroll_callback(GdkEventScroll* event)
{
    guint modifiers = gtk_accelerator_get_default_mod_mask ();

    if ((event->state & modifiers) == GDK_CONTROL_MASK)
    {
        int zoom = m_pianoroll.get_zoom();
        if (event->direction == GDK_SCROLL_DOWN)
        {
            m_eventroll.set_zoom(zoom * 2);
            m_pianoroll.set_zoom(zoom * 2);
        }
        else if (event->direction == GDK_SCROLL_UP)
        {
            m_eventroll.set_zoom(zoom / 2);
            m_pianoroll.set_zoom(zoom / 2);
        }
        return true;
    }
    else if ((event->state & modifiers) == GDK_SHIFT_MASK)
    {
        auto adj = m_hscrollbar.get_adjustment();
        if (event->direction == GDK_SCROLL_DOWN)
        {
            adj->set_value(adj->get_value() + adj->get_step_increment());
        }
        else if (event->direction == GDK_SCROLL_UP)
        {
            adj->set_value(adj->get_value() - adj->get_step_increment());
        }
    }

    return false;
}

void
EditWindow::focus_callback(string name)
{
    m_focus = name;
}
