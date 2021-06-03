#include "editwindow.h"
#include "pianoroll.h"
#include "../core/globals.h"

EditWindow::EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq) :
    m_perform(p),
    m_sequence(seq),
    m_mainwindow(m),
    m_seqnum(seqnum),
    m_pianokeys(p, seq),
    m_dataroll(p, seq),
    m_pianoroll(p, seq, &m_pianokeys, &m_dataroll)
{

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_class("editwindow");
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_vbox);

    m_vbox.pack_start(m_hbox, true, true);

    m_hbox.pack_start(m_left_vbox, false, true);
    m_hbox.pack_start(m_hscroll_wrapper, false, true);
    m_hbox.pack_end(m_right_vbox, false, false);

    m_left_vbox.set_size_request(100, 0);
    m_pianokeys_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianokeys_scroller.add(m_pianokeys);
    m_left_vbox.pack_start(m_pianokeys_scroller, true, true);
    m_left_vbox.pack_end(m_dummy1, false, false);
    m_dummy1.set_size_request(0, 100);

    m_hscroll_wrapper.set_hexpand(true);
    m_hscroll_wrapper.set_vexpand(true);
    m_hscroll_wrapper.add(m_hscroll_vbox);

    m_pianoroll_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianoroll_scroller.set_vadjustment(m_pianokeys_scroller.get_vadjustment());
    m_pianoroll_scroller.add(m_pianoroll);
    m_hscroll_vbox.pack_start(m_pianoroll_scroller, true, true);
    m_hscroll_vbox.pack_end(m_dataroll, false, true);

    m_vscrollbar.set_orientation(ORIENTATION_VERTICAL);
    m_vscrollbar.set_adjustment(m_pianokeys_scroller.get_vadjustment());
    m_right_vbox.pack_start(m_vscrollbar, true, true);
    m_right_vbox.pack_end(m_dummy2, false, false);
    m_dummy2.set_size_request(0, 100);

    update_size();

    // timer callback (25 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &EditWindow::timer_callback), 40);

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
EditWindow::update_size()
{
    int width = m_sequence->get_length();
    int height = c_key_height * c_num_keys;
    m_pianoroll_scroller.set_size_request(width, -1);
    m_pianoroll.set_size_request(-1, height);
    m_pianokeys.set_size_request(-1, height);
    m_dataroll.set_size_request(-1, 100);
    m_pianokeys_scroller.get_vadjustment()->configure((height - 500) / 2.0, 0.0, height, c_key_height, c_key_height * 12, 1);

}

bool
EditWindow::timer_callback()
{
    m_pianoroll.queue_draw();
    return true;
}
