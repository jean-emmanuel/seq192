#include "mainwindow.h"
#include "../core/globals.h"

MainWindow::MainWindow(perform * p)
{

    m_perform = p;

    add(m_main_vbox);

    // menu bar
    m_main_menu_file.set_label("File");
    m_main_menu.append(m_main_menu_file);

    // scroll wrapper
    m_scroll_wrapper.set_overlay_scrolling(false);
    m_scroll_wrapper.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    for (int i = 0; i < c_mainwnd_rows; i++) {
        for (int j = 0; j < c_mainwnd_cols; j++) {
            int n = i + j * c_mainwnd_rows;
            m_sequences[n] = new SequenceButton(m_perform, n);
            m_sequences[n]->set_can_focus(false);
            m_sequence_grid.attach(*m_sequences[n], j, i);
        }
    }
    m_sequence_grid.set_size_request(c_mainwid_x, c_mainwid_y);
    m_sequence_grid.set_column_homogeneous(true);
    m_sequence_grid.set_row_homogeneous(true);
    m_sequence_grid.set_column_spacing(2);
    m_sequence_grid.set_row_spacing(2);

    m_scroll_wrapper.add(m_sequence_grid);

    // main layout packing
    m_main_vbox.pack_start(m_toolbar_vbox, false, false);
    m_main_vbox.pack_start(m_main_menu, false, false);
    m_main_vbox.pack_end(m_scroll_wrapper, true, true);

    signal_timeout().connect(mem_fun(*this, &MainWindow::timer_callback), 25);

    resize(800, 600);
    show_all();

}

MainWindow::~MainWindow()
{

}

bool
MainWindow::timer_callback()
{

    for (int i = 0; i < c_seqs_in_set; i++) {
        int seqnum = i + m_perform->get_screenset() * c_seqs_in_set;
        if (m_perform->is_active(seqnum)) {
            m_sequences[i]->queue_draw();
        } else if (!m_sequences[i]->get_clear()) {
            m_sequences[i]->queue_draw();
            m_sequences[i]->set_clear();
        }
    }

    return true;
}
