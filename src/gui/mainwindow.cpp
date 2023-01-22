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


#include "mainwindow.h"
#include "../core/globals.h"
#include "../lib/nsm.h"
#include "../package.h"
#include <time.h>

#include "../xpm/seq192.xpm"
#include "../xpm/seq192_32.xpm"

MainWindow::MainWindow(perform * p, Glib::RefPtr<Gtk::Application> app)
{
    m_perform = p;
    m_app = app;

    m_nsm = 0;
    m_nsm_dirty = false;
    m_nsm_visible = true;

    m_drag_source = NULL;
    m_drag_destination = NULL;

    m_toolbar_play_state = false;

    m_accelgroup = Gtk::AccelGroup::create();
    add_accel_group(m_accelgroup);

    for (int i = 0; i < c_max_sequence; i++)
    {
        m_editwindows[i] = NULL;
    }

    // create instrument colors
    Gdk::RGBA color;
    for (int i = 0; i < c_max_instruments; i++)
    {
        if (!global_user_instrument_definitions[i].color.empty()) {
            color = Gdk::RGBA(global_user_instrument_definitions[i].color);
            global_user_instrument_colors[i] = {color.get_red(), color.get_green(), color.get_blue()};
        }
    }


    Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(c_mainwindow_css);
    this->get_style_context()->add_class("mainwindow");
    this->get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add(m_vbox);

    // menu bar
    m_menu_file.set_label("_File");
    m_menu_file.set_use_underline(true);
    m_menu_file.set_submenu(m_submenu_file);
    m_menu.append(m_menu_file);

    m_menu_file_new.set_label("_New");
    m_menu_file_new.set_use_underline(true);
    m_menu_file_new.add_accelerator("activate", m_accelgroup, 'n', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_new.signal_activate().connect([this]{menu_callback(MAIN_MENU_NEW, 0, 0);});
    m_submenu_file.append(m_menu_file_new);

    m_menu_file_open.set_label("_Open");
    m_menu_file_open.set_use_underline(true);
    m_menu_file_open.add_accelerator("activate", m_accelgroup, 'o', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_open.signal_activate().connect([this]{menu_callback(MAIN_MENU_OPEN, 0, 0);});
    m_submenu_file.append(m_menu_file_open);

    m_menu_file_save.set_label("_Save");
    m_menu_file_save.set_use_underline(true);
    m_menu_file_save.add_accelerator("activate", m_accelgroup, 's', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_save.signal_activate().connect([this]{menu_callback(MAIN_MENU_SAVE, 0, 0);});
    m_submenu_file.append(m_menu_file_save);

    m_menu_file_saveas.set_label("Save _As");
    m_menu_file_saveas.set_use_underline(true);
    m_menu_file_saveas.add_accelerator("activate", m_accelgroup, 's', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_saveas.signal_activate().connect([this]{menu_callback(MAIN_MENU_SAVEAS, 0, 0);});
    m_submenu_file.append(m_menu_file_saveas);

    m_submenu_file.append(m_menu_separator1);

    m_menu_file_import.set_label("_Import");
    m_menu_file_import.set_use_underline(true);
    m_menu_file_import.signal_activate().connect([this]{menu_callback(MAIN_MENU_IMPORT, 0, 0);});
    m_submenu_file.append(m_menu_file_import);

    m_menu_file_export.set_label("_Export Screeen Set");
    m_menu_file_export.set_use_underline(true);
    m_menu_file_export.signal_activate().connect([this]{menu_callback(MAIN_MENU_EXPORT_SCREENSET, 0, 0);});
    m_submenu_file.append(m_menu_file_export);

    m_submenu_file.append(m_menu_separator2);

    m_menu_file_quit.set_label("_Quit");
    m_menu_file_quit.set_use_underline(true);
    m_menu_file_quit.add_accelerator("activate", m_accelgroup, 'q', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_file_quit.signal_activate().connect([this]{menu_callback(MAIN_MENU_QUIT, 0, 0);});
    m_submenu_file.append(m_menu_file_quit);

    m_menu_transport.set_label("_Transport");
    m_menu_transport.set_use_underline(true);
    m_menu_transport.set_submenu(m_submenu_transport);
    m_menu.append(m_menu_transport);

    m_menu_transport_start_label.set_label("Start");
    m_menu_transport_start_label.set_xalign(0.0);
    m_menu_transport_start_label.set_accel(GDK_KEY_space, (Gdk::ModifierType)0);
    m_menu_transport_start.add(m_menu_transport_start_label);
    m_menu_transport_start.signal_activate().connect([&]{
        m_perform->start_playing();
        clear_focus();});
    m_submenu_transport.append(m_menu_transport_start);

    m_menu_transport_stop_label.set_label("Stop");
    m_menu_transport_stop_label.set_xalign(0.0);
    m_menu_transport_stop_label.set_accel(GDK_KEY_Escape, (Gdk::ModifierType)0);
    m_menu_transport_stop.add(m_menu_transport_stop_label);
    m_menu_transport_stop.signal_activate().connect([&]{
        m_perform->stop_playing();
        clear_focus();});
    m_submenu_transport.append(m_menu_transport_stop);


    // toolbar
    m_toolbar.set_size_request(0, 55);
    m_toolbar.get_style_context()->add_class("toolbar");
    m_toolbar.get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    m_toolbar.set_spacing(c_toolbar_spacing);

    m_toolbar_panic.set_size_request(36, 0);
    m_toolbar_panic.set_can_focus(false);
    m_toolbar_panic.set_label("◭");
    m_toolbar_panic.set_tooltip_text("Disable all sequences");
    m_toolbar_panic.get_style_context()->add_class("panic");
    m_toolbar_panic.signal_clicked().connect([&]{
        m_perform->panic();
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_panic, false, false);

    m_toolbar_stop.set_size_request(36, 0);
    m_toolbar_stop.set_can_focus(false);
    m_toolbar_stop.set_label("◼");
    m_toolbar_stop.set_tooltip_text("Stop transport");
    m_toolbar_stop.get_style_context()->add_class("stop");
    m_toolbar_stop.signal_clicked().connect([&]{
        m_perform->stop_playing();
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_stop, false, false);

    m_toolbar_play.set_size_request(36, 0);
    m_toolbar_play.set_can_focus(false);
    m_toolbar_play.set_label("▶");
    m_toolbar_play.set_tooltip_text("Start transport");
    m_toolbar_play.get_style_context()->add_class("play");
    m_toolbar_play.signal_clicked().connect([&]{
        m_perform->start_playing();
        clear_focus();
    });
    m_toolbar.pack_start(m_toolbar_play, false, false);

    m_toolbar_bpm_adj = Gtk::Adjustment::create(m_perform->get_bpm(), c_bpm_minimum, c_bpm_maximum, 1, 10, 1);
    m_toolbar_bpm.set_name("bpm");
    m_toolbar_bpm.set_tooltip_text("Beats per minute");
    m_toolbar_bpm.set_width_chars(6);
    m_toolbar_bpm.set_digits(2);
    m_toolbar_bpm.set_numeric(true);
    m_toolbar_bpm.set_halign(Gtk::ALIGN_CENTER);
    m_toolbar_bpm.set_adjustment(m_toolbar_bpm_adj);
    m_toolbar_bpm.signal_activate().connect([&]{clear_focus();});
    m_toolbar_bpm.signal_value_changed().connect([&]{
        m_perform->set_bpm(m_toolbar_bpm.get_value());
    });
    m_toolbar.pack_start(m_toolbar_bpm, false, false);

    m_toolbar_sset_name.set_name("sset_name");
    m_toolbar_sset_name.set_tooltip_text("Screen set name");
    m_toolbar_sset_name.set_alignment(0.5);
    m_toolbar_sset_name.signal_activate().connect([&]{clear_focus();});
    m_toolbar_sset_name.signal_focus_out_event().connect([&](GdkEventFocus *focus)->bool{
        string s = m_toolbar_sset_name.get_text();
        m_perform->set_screen_set_notepad(m_toolbar_sset.get_value(), &s);
        return false;
    });
    m_toolbar.pack_start(m_toolbar_sset_name);

    m_toolbar_sset_adj = Gtk::Adjustment::create(0, 0, c_max_sets, 1, 1, 1);
    m_toolbar_sset.set_name("sset");
    m_toolbar_sset.set_tooltip_text("Screen set number");
    m_toolbar_sset.set_width_chars(2);
    m_toolbar_sset.set_numeric(true);
    m_toolbar_sset.set_alignment(0.5);
    m_toolbar_sset.set_adjustment(m_toolbar_sset_adj);
    m_toolbar_sset.signal_activate().connect([&]{clear_focus();});
    m_toolbar_sset.signal_value_changed().connect([&]{
        int sset = m_toolbar_sset.get_value();
        m_perform->set_screenset(sset);
        update_sset_name(sset);
    });
    m_toolbar.pack_start(m_toolbar_sset, false, false);
    update_sset_name(m_perform->get_screenset());

    m_toolbar_logo.set(Gdk::Pixbuf::create_from_xpm_data(seq192_xpm));
    m_toolbar.pack_end(m_toolbar_logo, false, false);


    // scroll wrapper
    m_scroll_wrapper.set_overlay_scrolling(true);
    m_scroll_wrapper.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    for (int i = 0; i < c_mainwnd_rows; i++) {
        for (int j = 0; j < c_mainwnd_cols; j++) {
            int n = i + j * c_mainwnd_rows;
            m_sequences[n] = new SequenceButton(m_perform, this, n);
            m_sequences[n]->set_size_request(100,60);
            m_sequences[n]->set_can_focus(false);
            m_sequence_grid.attach(*m_sequences[n], j, i);
        }
    }
    // m_sequence_grid.set_size_request(c_mainwid_x, c_mainwid_y);
    m_sequence_grid.set_column_homogeneous(true);
    m_sequence_grid.set_row_homogeneous(true);
    m_sequence_grid.set_column_spacing(c_grid_spacing);
    m_sequence_grid.set_row_spacing(c_grid_spacing);
    m_sequence_grid.set_margin_start(c_grid_padding);
    m_sequence_grid.set_margin_end(c_grid_padding);
    m_sequence_grid.set_margin_top(c_grid_padding);
    m_sequence_grid.set_margin_bottom(c_grid_padding);
    m_scroll_wrapper.add(m_sequence_grid);

    // main layout packing
    m_vbox.pack_start(m_menu, false, false);
    m_vbox.pack_start(m_toolbar, false, false);
    m_vbox.pack_end(m_scroll_wrapper, true, true);

    // accelerators
    signal_key_press_event().connect(mem_fun(*this, &MainWindow::on_key_press), false);

    // timer callback (25 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &MainWindow::timer_callback), 40);

    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq192_32_xpm));

    clear_focus();
    update_window_title();
    set_position(Gtk::WIN_POS_CENTER);
    resize(1024, 600);
    show_all();

}

MainWindow::~MainWindow()
{

}

bool
MainWindow::on_key_press(GdkEventKey* event)
{
    if (get_focus() != NULL) {
        string focus = get_focus()->get_name();
        if (event->keyval == GDK_KEY_space && focus == "sset_name") return false;
        if ((event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down) &&
            (focus == "bpm" || focus == "sset"))
        {
            return false;
        }
    }
    switch (event->keyval) {
        case GDK_KEY_Escape:
            m_perform->stop_playing();
            clear_focus();
            break;
        case GDK_KEY_space:
            m_perform->start_playing();
            clear_focus();
            break;
        case GDK_KEY_Up:
        case GDK_KEY_Down:
            return true;
            break;
        default:
            return false;
    }

    return false;
}

bool
MainWindow::timer_callback()
{

    if (!global_is_running) {
        // SIGINT: ignore unsave modifications and quit
        global_is_modified = false;
        close();
    }

    // screenset name
    int sset = m_perform->get_screenset();
    if (m_toolbar_sset.get_value() != sset) {
        update_sset_name(sset);
        m_toolbar_sset.set_value(sset);
    }

    // bpm
    double bpm = m_perform->get_bpm();
    if (m_toolbar_bpm.get_value() != bpm) {
        m_toolbar_bpm.set_value(bpm);
    }

    // sequence grid
    for (int i = 0; i < c_seqs_in_set; i++) {
        int seqnum = i + m_perform->get_screenset() * c_seqs_in_set;
        int active = m_perform->is_active(seqnum);
        bool changed = m_sequences[i]->set_active(active) || m_sequences[i]->get_last_sequence_number() != m_sequences[i]->get_sequence_number();

        if (changed) {
            m_sequences[i]->set_last_sequence_number();
            m_sequences[i]->draw_background();
            m_sequences[i]->queue_draw();
        }
        else if (active) {
            m_sequences[i]->update();
        }
    }

    // play button state
    bool playing = m_perform->is_running();
    if (playing != m_toolbar_play_state) {
        m_toolbar_play_state = playing;
        if (playing) {
            m_toolbar_play.get_style_context()->add_class("on");
        } else {
            m_toolbar_play.get_style_context()->remove_class("on");
        }
    }

    // nsm
    if (m_nsm) {
        nsm_check_nowait(m_nsm);
        if (m_nsm_optional_gui && m_nsm_visible != global_nsm_gui) {
            m_nsm_visible = global_nsm_gui;
            if (m_nsm_visible)
            {
                show();
                nsm_send_is_shown(m_nsm);
            }
            else
            {
                m_app->hold();
                close_all_edit_windows();
                hide();
                nsm_send_is_hidden(m_nsm);
            }
        }
        if (m_nsm_dirty != global_is_modified) {
            m_nsm_dirty = global_is_modified;
            if (m_nsm_dirty) nsm_send_is_dirty(m_nsm);
            else nsm_send_is_clean(m_nsm);
        }
    }

    return true;
}

void
MainWindow::update_sset_name(int sset)
{
    m_toolbar_sset_name.set_text(*m_perform->get_screen_set_notepad(sset));
}

void
MainWindow::clear_focus()
{
    m_toolbar_bpm.select_region(0, 0);
    m_toolbar_sset_name.select_region(0, 0);
    m_toolbar_sset.select_region(0, 0);
    m_scroll_wrapper.set_can_focus(true);
    m_scroll_wrapper.grab_focus();
    m_scroll_wrapper.set_can_focus(false);
}

void
MainWindow::menu_callback(main_menu_action action, int data1, int data2)
{

    switch (action) {
        case MAIN_MENU_NEW:
            if (global_is_modified && !unsaved_changes()) return;
            close_all_edit_windows();
            m_perform->file_new();
            update_window_title();
            update_sset_name(m_perform->get_screenset());
            for (int i = 0; i < c_seqs_in_set; i++) {
                m_sequences[i]->queue_draw();
            }
            break;
        case MAIN_MENU_OPEN:
        case MAIN_MENU_IMPORT:
            {
                if (action == MAIN_MENU_OPEN && global_is_modified && !unsaved_changes()) return;

                if (action == MAIN_MENU_OPEN) close_all_edit_windows();

                FileChooserDialog dialog("Open MIDI file", Gtk::FILE_CHOOSER_ACTION_OPEN);

                if (action == MAIN_MENU_IMPORT) dialog.set_title("Import MIDI file in current screenset");

                dialog.set_transient_for(*this);

                dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
                dialog.add_button("_Open", Gtk::RESPONSE_OK);

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
                    bool result;

                    if (action == MAIN_MENU_OPEN) {
                        result = m_perform->file_open(fn);
                    } else if (action == MAIN_MENU_IMPORT) {
                        result = m_perform->file_import(fn);
                    }

                    if (!result) {
                        MessageDialog errdialog(*this, "Error reading file: " + fn, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                        errdialog.run();
                        return;
                    }

                    last_used_dir = fn.substr(0, fn.rfind("/") + 1);

                    update_window_title();
                    update_sset_name(m_perform->get_screenset());
                    for (int i = 0; i < c_seqs_in_set; i++) {
                        m_sequences[i]->queue_draw();
                    }
                }
                break;
            }
        case MAIN_MENU_SAVE:
            if (global_filename == "") {
                menu_callback(MAIN_MENU_SAVEAS, -1, -1);
            } else {
                bool result = m_perform->file_save();
                if (!result) {
                    Gtk::MessageDialog errdialog(*this, "Error writing file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                    errdialog.run();
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

                dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
                dialog.add_button("_Save", Gtk::RESPONSE_OK);

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

                    bool result;
                    if (action == MAIN_MENU_SAVEAS) {
                        if (m_nsm) {
                            result = m_perform->file_export(fn);
                        } else {
                            result = m_perform->file_saveas(fn);
                        }
                    } else if (action == MAIN_MENU_EXPORT_SCREENSET) {
                        result = m_perform->file_export_screenset(fn);
                    } else if (action == MAIN_MENU_EXPORT_SEQUENCE) {
                        result = m_perform->file_export_sequence(fn, data1);
                    }

                    if (!result) {
                        MessageDialog errdialog (*this, "Error writing file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                        errdialog.run();
                    }
                }
            }
            break;
        case MAIN_MENU_QUIT:
            if (m_nsm && m_nsm_optional_gui) global_nsm_gui = false;
            else close();
            break;
    }

}


void
MainWindow::update_window_title()
{
    std::string title;

    if (global_filename == "")
        title = string(PACKAGE) + " - Untitled";
    else
        title = string(PACKAGE) + " - " + global_filename;

    set_title(title.c_str());
}



bool
MainWindow::on_delete_event(GdkEventAny *event)
{
    if (m_nsm && m_nsm_optional_gui) {
        // nsm : hide gui instead of closing
        global_nsm_gui = false;
        return true;
    }

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
            query_str = "'Untitled' has changes, do you want to save them ?";
        else
            query_str = "'" + global_filename + "' has changes, do you want to save them ?";

        MessageDialog dialog(*this, query_str, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, false);

        dialog.add_button("_Save", Gtk::RESPONSE_YES);
        dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_No", Gtk::RESPONSE_NO);

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

void
MainWindow::set_drag_source(SequenceButton *s)
{
    m_drag_source = s;
}

void
MainWindow::set_drag_destination(SequenceButton *s)
{
    if (m_drag_source != NULL && m_drag_source != s) {
        m_drag_destination = s;
        int seqnum_src = m_drag_source->get_sequence_number();
        int seqnum_dest = m_drag_destination->get_sequence_number();
        if (m_editwindows[seqnum_src] != NULL) m_editwindows[seqnum_src]->close();
        m_perform->move_sequence(seqnum_src, seqnum_dest);
        m_drag_source->queue_draw();
        m_drag_destination->queue_draw();
        m_drag_source = NULL;
        m_drag_destination = NULL;
    }
}


void
MainWindow::open_edit_window(int seqnum, sequence * seq)
{
    if (m_editwindows[seqnum] == NULL) {
        m_editwindows[seqnum] = new EditWindow(m_perform, this, seqnum, seq);
    } else {
        m_editwindows[seqnum]->raise();
    }
}

void
MainWindow::close_edit_window(int seqnum)
{
    m_editwindows[seqnum] = NULL;
}

void
MainWindow::close_all_edit_windows()
{
    for (int i = 0; i < c_max_sequence; i++)
    {
        if (m_editwindows[i] != NULL)
        {
            m_editwindows[i]->close();

        }
    }
}

void
MainWindow::nsm_set_client(nsm_client_t *nsm, bool optional_gui)
{
    m_nsm = nsm;
    m_nsm_optional_gui = optional_gui;
    m_menu_file_new.set_sensitive(false);
    m_menu_file_open.set_sensitive(false);
    m_menu_file_saveas.set_label("Export session");
    if (m_nsm_optional_gui) m_menu_file_quit.set_label("Hide");
}
