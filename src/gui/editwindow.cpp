#include "editwindow.h"
#include "pianoroll.h"
#include "../core/globals.h"

EditWindow::EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq) :
    m_pianoroll(p, seq)
{
    m_perform = p;
    m_mainwindow = m;
    m_seqnum = seqnum;
    m_sequence = seq;

    add(m_vbox);

    // scroll wrapper
    m_scroll_wrapper.set_overlay_scrolling(true);
    m_scroll_wrapper.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);


    m_scroll_wrapper.add(m_pianoroll);


    m_vbox.pack_end(m_scroll_wrapper, true, true);

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
