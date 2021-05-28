#include "mainwindow.h"
#include "../core/globals.h"
#include <time.h>

MainWindow::MainWindow(perform * p)
{

    m_perform = p;

    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_class("MainWindow");
    this->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_main_vbox);

    // menu bar
    m_main_menu_file.set_label("_File");
    m_main_menu_file.set_use_underline(true);
    m_main_menu_file.set_submenu(m_main_submenu_file);
    m_main_menu.append(m_main_menu_file);

    m_main_menu_file_new.set_label("_New");
    m_main_menu_file_new.set_use_underline(true);
    m_main_menu_file_new.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'n', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_main_menu_file_new.signal_activate().connect([this]{menu_callback(MAIN_MENU_NEW, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_new);

    m_main_menu_file_open.set_label("_Open");
    m_main_menu_file_open.set_use_underline(true);
    m_main_menu_file_open.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'o', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_main_menu_file_open.signal_activate().connect([this]{menu_callback(MAIN_MENU_OPEN, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_open);

    m_main_menu_file_save.set_label("_Save");
    m_main_menu_file_save.set_use_underline(true);
    m_main_menu_file_save.Gtk::Widget::add_accelerator("activate", get_accel_group(), 's', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_main_menu_file_save.signal_activate().connect([this]{menu_callback(MAIN_MENU_SAVE, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_save);

    m_main_menu_file_saveas.set_label("Save _As");
    m_main_menu_file_saveas.set_use_underline(true);
    m_main_menu_file_saveas.Gtk::Widget::add_accelerator("activate", get_accel_group(), 's', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_main_menu_file_saveas.signal_activate().connect([this]{menu_callback(MAIN_MENU_SAVEAS, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_saveas);

    m_main_submenu_file.append(m_main_menu_separator1);

    m_main_menu_file_import.set_label("_Import");
    m_main_menu_file_import.set_use_underline(true);
    m_main_menu_file_import.signal_activate().connect([this]{menu_callback(MAIN_MENU_IMPORT, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_import);

    m_main_menu_file_export.set_label("_Export Screeen Set");
    m_main_menu_file_export.set_use_underline(true);
    m_main_menu_file_export.signal_activate().connect([this]{menu_callback(MAIN_MENU_EXPORT_SCREENSET, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_export);

    m_main_submenu_file.append(m_main_menu_separator2);

    m_main_menu_file_quit.set_label("_Quit");
    m_main_menu_file_quit.set_use_underline(true);
    m_main_menu_file_quit.Gtk::Widget::add_accelerator("activate", get_accel_group(), 'q', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_main_menu_file_quit.signal_activate().connect([this]{menu_callback(MAIN_MENU_QUIT, 0, 0);});
    m_main_submenu_file.append(m_main_menu_file_quit);



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
    m_sequence_grid.set_column_spacing(c_grid_spacing);
    m_sequence_grid.set_row_spacing(c_grid_spacing);
    m_sequence_grid.set_margin_left(c_grid_padding);
    m_sequence_grid.set_margin_right(c_grid_padding);
    m_sequence_grid.set_margin_top(c_grid_padding);
    m_sequence_grid.set_margin_bottom(c_grid_padding);
    m_scroll_wrapper.add(m_sequence_grid);

    // main layout packing
    m_main_vbox.pack_start(m_toolbar_vbox, false, false);
    m_main_vbox.pack_start(m_main_menu, false, false);
    m_main_vbox.pack_end(m_scroll_wrapper, true, true);

    // timer callback (25 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &MainWindow::timer_callback), 40);

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

void
MainWindow::menu_callback(main_menu_action action, int data1, int data2)
{

    switch (action) {
        case MAIN_MENU_NEW:
            if (!unsaved_changes()) return;
            m_perform->clear_all();
            global_filename = "";
            update_window_title();
            global_is_modified = false;
            // TODO: ajust bpm & sset name
            break;
        case MAIN_MENU_OPEN:
        case MAIN_MENU_IMPORT:
            {
                if (action == MAIN_MENU_OPEN && !unsaved_changes()) return;

                FileChooserDialog dialog("Open MIDI file", Gtk::FILE_CHOOSER_ACTION_OPEN);

                if (action == MAIN_MENU_IMPORT) dialog.set_title("Import MIDI file in current screenset");

                dialog.set_transient_for(*this);

                dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
                dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

                auto filter_midi = Gtk::FileFilter::create();
                filter_midi->set_name("MIDI files");
                filter_midi->add_pattern("*.midi");
                filter_midi->add_pattern("*.mid");
                dialog.add_filter(filter_midi);

                auto filter_any = Gtk::FileFilter::create();
                filter_any->set_name("Any files");
                filter_any->add_pattern("*");
                dialog.add_filter(filter_any);

                dialog.set_current_folder(last_used_dir);
                if (dialog.run() == Gtk::RESPONSE_OK)
                {
                    auto fn = dialog.get_filename();

                    if (action == MAIN_MENU_OPEN) m_perform->clear_all();

                    midifile f(fn);
                    bool result = f.parse(m_perform, action == MAIN_MENU_OPEN ? 0 : m_perform->get_screenset());
                    global_is_modified = !result;

                    if (!result) {
                        MessageDialog errdialog(*this, "Error reading file: " + fn, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                        errdialog.run();
                        return;
                    }

                    last_used_dir = fn.substr(0, fn.rfind("/") + 1);
                    global_filename = fn;
                    update_window_title();

                    // TODO: ajust bpm & sset name
                }
                break;
            }
        case MAIN_MENU_SAVE:
            if (global_filename == "") {
                menu_callback(MAIN_MENU_SAVEAS, -1, -1);
            } else {
                midifile f(global_filename);
                bool result = f.write(m_perform, -1, -1);
                if (!result) {
                    Gtk::MessageDialog errdialog(*this, "Error writing file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                    errdialog.run();
                } else {
                    global_is_modified = false;
                }
            }
            break;
        case MAIN_MENU_SAVEAS:
        case MAIN_MENU_EXPORT_SCREENSET:
        case MAIN_MENU_EXPORT_SEQUENCE:
            {
                FileChooserDialog dialog("Save file as", Gtk::FILE_CHOOSER_ACTION_SAVE);

                if (action == MAIN_MENU_EXPORT_SCREENSET) dialog.set_title("Midi export screenset");
                if (action == MAIN_MENU_EXPORT_SEQUENCE) dialog.set_title("Midi export sequence");

                dialog.set_transient_for(*this);

                dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
                dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

                auto filter_midi = Gtk::FileFilter::create();
                filter_midi->set_name("MIDI files");
                filter_midi->add_pattern("*.midi");
                filter_midi->add_pattern("*.mid");
                dialog.add_filter(filter_midi);

                auto filter_any = Gtk::FileFilter::create();
                filter_any->set_name("Any files");
                filter_any->add_pattern("*");
                dialog.add_filter(filter_any);
                dialog.set_current_folder(last_used_dir);

                if (dialog.run() == Gtk::RESPONSE_OK)
                {
                    auto fn = dialog.get_filename();

                    if (dialog.get_filter()->get_name() == "MIDI files") {
                        // check for MIDI file extension; if missing, add .midi
                        std::string suffix = fn.substr( fn.find_last_of(".") + 1, std::string::npos);
                        for (basic_string<char>::iterator p = suffix.begin();
                                p != suffix.end(); p++) {
                            *p = tolower(*p);
                        }
                        if (suffix != "midi" && suffix != "mid") fn = fn + ".midi";
                    }

                    if (Glib::file_test(fn, Glib::FILE_TEST_EXISTS)) {
                        Gtk::MessageDialog warning(*this,
                                "File already exists!\n"
                                "Do you want to overwrite it?",
                                false,
                                Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);

                        if (warning.run() == Gtk::RESPONSE_NO) return;
                    }

                    midifile f(fn);
                    int a_sset = action == MAIN_MENU_EXPORT_SCREENSET ? m_perform->get_screenset() : -1;
                    int a_seq  = action == MAIN_MENU_EXPORT_SEQUENCE ? data1 : -1;
                    bool result = f.write(m_perform, a_sset, a_seq);
                    if (!result) {
                        MessageDialog errdialog (*this, "Error writing file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                        errdialog.run();
                    } else if (action == MAIN_MENU_SAVEAS) {
                        global_filename = fn;
                        global_is_modified = false;
                    }
                }

                }
            break;
        case MAIN_MENU_QUIT:
            close();
            break;
    }

}


void
MainWindow::update_window_title()
{
    std::string title;

    if (global_filename == "")
        title = ( PACKAGE ) + string( " - [unnamed]" );
    else
        title =
            ( PACKAGE )
            + string( " - [" )
            + Glib::filename_to_utf8(global_filename)
            + string( "]" );

    set_title ( title.c_str());
}



bool
MainWindow::on_delete_event(GdkEventAny *event)
{
    bool result = global_is_modified && !unsaved_changes();

    if (result) m_perform->stop_playing();

    return result;
}

bool
MainWindow::unsaved_changes()
{
    bool result = false;

    if (global_is_modified)
    {

        Glib::ustring query_str;

        if (global_filename == "")
            query_str = "'Unnamed' has changes, do you want to save them ?";
        else
            query_str = "'" + global_filename + "' has changes, do you want to save them ?";

        MessageDialog dialog(*this, query_str, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);

        dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES);
        dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dialog.add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);

        switch (dialog.run()) {
            case Gtk::RESPONSE_YES:
                menu_callback(MAIN_MENU_SAVE, -1, -1);
                if (!global_is_modified) result = true;
                break;
            case Gtk::RESPONSE_NO:
                result = true;
                break;
            case Gtk::RESPONSE_CANCEL:
                break;

        }
    }

    return result;
}
