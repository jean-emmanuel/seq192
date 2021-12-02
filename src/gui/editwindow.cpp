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


#include "editwindow.h"
#include "pianoroll.h"
#include "../core/globals.h"
#include "../package.h"
#include "../core/controllers.h"

#include "../xpm/seq192_32.xpm"

EditWindow::EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq) :
    m_perform(p),
    m_sequence(seq),
    m_mainwindow(m),
    m_seqnum(seqnum),
    m_bg_seqnum(-1),
    m_pianokeys(p, seq),
    m_eventroll(p, seq),
    m_timeroll(p, seq),
    m_pianoroll(p, seq, &m_pianokeys),
    m_dataroll(p, seq),
    m_midibus(-1),
    m_midichannel(-1),
    m_status(EVENT_NOTE_ON),
    m_cc(0),
    m_alt_status(0),
    m_alt_cc(0),
    m_alt_control_view(false)
{

    m_accelgroup = Gtk::AccelGroup::create();
    add_accel_group(m_accelgroup);

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
    m_menu_edit_undo.add_accelerator("activate", m_accelgroup, 'z', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_undo.signal_activate().connect([&]{menu_callback(EDIT_MENU_UNDO);});
    m_submenu_edit.append(m_menu_edit_undo);

    m_menu_edit_redo.set_label("_Redo");
    m_menu_edit_redo.set_use_underline(true);
    m_menu_edit_redo.add_accelerator("activate", m_accelgroup, 'y', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_redo.add_accelerator("activate", m_accelgroup, 'z', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_redo.signal_activate().connect([&]{menu_callback(EDIT_MENU_REDO);});
    m_submenu_edit.append(m_menu_edit_redo);

    m_submenu_edit.append(m_menu_separator0);

    m_menu_edit_cut.set_label("_Cut");
    m_menu_edit_cut.set_use_underline(true);
    m_menu_edit_cut.add_accelerator("activate", m_accelgroup, 'x', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_cut.signal_activate().connect([&]{menu_callback(EDIT_MENU_CUT);});
    m_submenu_edit.append(m_menu_edit_cut);

    m_menu_edit_copy.set_label("_Copy");
    m_menu_edit_copy.set_use_underline(true);
    m_menu_edit_copy.add_accelerator("activate", m_accelgroup, 'c', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_copy.signal_activate().connect([&]{menu_callback(EDIT_MENU_COPY);});
    m_submenu_edit.append(m_menu_edit_copy);

    m_menu_edit_paste.set_label("_Paste");
    m_menu_edit_paste.set_use_underline(true);
    m_menu_edit_paste.add_accelerator("activate", m_accelgroup, 'v', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_paste.signal_activate().connect([&]{menu_callback(EDIT_MENU_PASTE);});
    m_submenu_edit.append(m_menu_edit_paste);

    m_menu_edit_delete_label.set_label("Delete");
    m_menu_edit_delete_label.set_xalign(0.0);
    m_menu_edit_delete_label.set_accel(GDK_KEY_Delete, (Gdk::ModifierType)0);
    m_menu_edit_delete.add(m_menu_edit_delete_label);
    m_menu_edit_delete.set_use_underline(true);
    m_menu_edit_delete.signal_activate().connect([&]{menu_callback(EDIT_MENU_DELETE);});
    m_submenu_edit.append(m_menu_edit_delete);

    m_submenu_edit.append(m_menu_separator1);

    m_menu_edit_select.set_label("_Select");
    m_menu_edit_select.set_use_underline(true);
    m_submenu_edit.append(m_menu_edit_select);
    m_menu_edit_select.set_submenu(m_submenu_select);

    m_menu_edit_invert.set_label("_Invert Selection");
    m_menu_edit_invert.set_use_underline(true);
    m_menu_edit_invert.add_accelerator("activate", m_accelgroup, 'i', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_invert.signal_activate().connect([&]{menu_callback(EDIT_MENU_INVERT);});
    m_submenu_select.append(m_menu_edit_invert);

    m_menu_edit_selectall.set_label("Select _All");
    m_menu_edit_selectall.set_use_underline(true);
    m_menu_edit_selectall.add_accelerator("activate", m_accelgroup, 'a', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_selectall.signal_activate().connect([&]{menu_callback(EDIT_MENU_SELECTALL);});
    m_submenu_select.append(m_menu_edit_selectall);

    m_menu_edit_unselect.set_label("_Unselect All");
    m_menu_edit_unselect.set_use_underline(true);
    m_menu_edit_unselect.add_accelerator("activate", m_accelgroup, 'a', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_unselect.signal_activate().connect([&]{menu_callback(EDIT_MENU_UNSELECT);});
    m_submenu_select.append(m_menu_edit_unselect);


    m_menu_edit_transpose.set_label("_Transpose Selection");
    m_menu_edit_transpose.set_use_underline(true);
    m_submenu_edit.append(m_menu_edit_transpose);
    m_menu_edit_transpose.set_submenu(m_submenu_transpose);

    m_menu_edit_transpose_octup.set_label("+1 octave");
    m_menu_edit_transpose_octup.add_accelerator("activate", m_accelgroup, GDK_KEY_Up, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_transpose_octup.signal_activate().connect([&]{menu_callback(EDIT_MENU_TRANSPOSE, 12);});
    m_submenu_transpose.append(m_menu_edit_transpose_octup);

    m_menu_edit_transpose_up.set_label("+1 semitone");
    m_menu_edit_transpose_up.add_accelerator("activate", m_accelgroup, GDK_KEY_Up, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_edit_transpose_up.signal_activate().connect([&]{menu_callback(EDIT_MENU_TRANSPOSE, 1);});
    m_submenu_transpose.append(m_menu_edit_transpose_up);

    m_menu_edit_transpose_down.set_label("-1 semitone");
    m_menu_edit_transpose_down.add_accelerator("activate", m_accelgroup, GDK_KEY_Down, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_edit_transpose_down.signal_activate().connect([&]{menu_callback(EDIT_MENU_TRANSPOSE, -1);});
    m_submenu_transpose.append(m_menu_edit_transpose_down);

    m_menu_edit_transpose_octdown.set_label("-1 octave");
    m_menu_edit_transpose_octdown.add_accelerator("activate", m_accelgroup, GDK_KEY_Down, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_transpose_octdown.signal_activate().connect([&]{menu_callback(EDIT_MENU_TRANSPOSE, -12);});
    m_submenu_transpose.append(m_menu_edit_transpose_octdown);

    m_menu_edit_move.set_label("_Move Selection");
    m_menu_edit_move.set_use_underline(true);
    m_submenu_edit.append(m_menu_edit_move);
    m_menu_edit_move.set_submenu(m_submenu_move);

    m_menu_edit_quantize.set_label("Quantize");
    m_menu_edit_quantize.signal_activate().connect([&]{menu_callback(EDIT_MENU_QUANTIZE);});
    m_submenu_move.append(m_menu_edit_quantize);

    m_menu_edit_moveleft.set_label("Left");
    m_menu_edit_moveleft.add_accelerator("activate", m_accelgroup, GDK_KEY_Left, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_edit_moveleft.signal_activate().connect([&]{menu_callback(EDIT_MENU_MOVE, -m_pianoroll.m_snap);});
    m_submenu_move.append(m_menu_edit_moveleft);

    m_menu_edit_movefineleft_label.set_label("Left (fine)");
    m_menu_edit_movefineleft_label.set_accel(GDK_KEY_Left, Gdk::SHIFT_MASK);
    m_menu_edit_movefineleft.add(m_menu_edit_movefineleft_label);
    m_menu_edit_movefineleft.signal_activate().connect([&]{menu_callback(EDIT_MENU_MOVE, -1);});
    m_submenu_move.append(m_menu_edit_movefineleft);

    m_menu_edit_moveright.set_label("Right");
    m_menu_edit_moveright.add_accelerator("activate", m_accelgroup, GDK_KEY_Right, (Gdk::ModifierType)0, Gtk::ACCEL_VISIBLE);
    m_menu_edit_moveright.signal_activate().connect([&]{menu_callback(EDIT_MENU_MOVE, m_pianoroll.m_snap);});
    m_submenu_move.append(m_menu_edit_moveright);

    m_menu_edit_movefineright_label.set_label("Right (fine)");
    m_menu_edit_movefineright_label.set_accel(GDK_KEY_Right, Gdk::SHIFT_MASK);
    m_menu_edit_movefineright.add(m_menu_edit_movefineright_label);
    m_menu_edit_movefineright.signal_activate().connect([&]{menu_callback(EDIT_MENU_MOVE, 1);});
    m_submenu_move.append(m_menu_edit_movefineright);

    m_menu_edit_pattern.set_label("_Pattern");
    m_menu_edit_pattern.set_use_underline(true);
    m_submenu_edit.append(m_menu_edit_pattern);
    m_menu_edit_pattern.set_submenu(m_submenu_pattern);

    m_menu_edit_expand.set_label("Expand (x2)");
    m_menu_edit_expand.signal_activate().connect([&]{menu_callback(EDIT_MENU_MULTIPLY, 2);});
    m_submenu_pattern.append(m_menu_edit_expand);

    m_menu_edit_compress.set_label("Compress (/2)");
    m_menu_edit_compress.signal_activate().connect([&]{menu_callback(EDIT_MENU_MULTIPLY, 0.5);});
    m_submenu_pattern.append(m_menu_edit_compress);

    m_menu_edit_reverse.set_label("Reverse");
    m_menu_edit_reverse.signal_activate().connect([&]{menu_callback(EDIT_MENU_REVERSE);});
    m_submenu_pattern.append(m_menu_edit_reverse);

    m_submenu_edit.append(m_menu_separator2);

    m_menu_edit_close.set_label("_Close");
    m_menu_edit_close.set_use_underline(true);
    m_menu_edit_close.add_accelerator("activate", m_accelgroup, 'w', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_edit_close.signal_activate().connect([&]{menu_callback(EDIT_MENU_CLOSE);});
    m_submenu_edit.append(m_menu_edit_close);

    m_menu_view.set_label("_View");
    m_menu_view.set_use_underline(true);
    m_menu_view.set_submenu(m_submenu_view);
    m_menu.append(m_menu_view);

    m_menu_view_bg_sequence.set_label("Background sequence");
    m_menu_view_bg_sequence.set_submenu(m_submenu_bg_sequence);
    m_submenu_view.append(m_menu_view_bg_sequence);
    m_menu_items_bgseq[c_seqs_in_set] =  new CheckMenuItem();
    m_menu_items_bgseq[c_seqs_in_set]->set_label("None");
    m_menu_items_bgseq[c_seqs_in_set]->signal_toggled().connect([&]{
        set_background_sequence(-1, NULL);
    });
    m_submenu_bg_sequence.append(*m_menu_items_bgseq[c_seqs_in_set]);
    int off_i = ((int) m_seqnum / c_seqs_in_set) * c_seqs_in_set;
    for (int i=0; i < c_seqs_in_set; i++) {
        m_menu_items_bgseq[i] = new CheckMenuItem();
        m_menu_items_bgseq[i]->signal_toggled().connect([&, i, off_i]{
            if (m_bg_seqnum != i + off_i) {
                set_background_sequence(i + off_i, m_perform->get_sequence(i + off_i));
            }
        });
        m_submenu_bg_sequence.append(*m_menu_items_bgseq[i]);
        m_menu_items_bgseq[i]->hide();
    }
    m_submenu_view.signal_popped_up().connect([&](const Gdk::Rectangle* const&, const Gdk::Rectangle* const&, const bool&, const bool&){
        update_background_menu();
    });
    update_background_menu();

    m_menu_transport.set_label("_Transport");
    m_menu_transport.set_use_underline(true);
    m_menu_transport.set_submenu(m_submenu_transport);
    m_menu.append(m_menu_transport);

    m_menu_transport_start_label.set_label("Start");
    m_menu_transport_start_label.set_accel(GDK_KEY_space, (Gdk::ModifierType)0);
    m_menu_transport_start_label.set_xalign(0.0);
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

    m_menu_record.set_label("_Record");
    m_menu_record.set_use_underline(true);
    m_menu_record.set_submenu(m_submenu_record);
    m_menu.append(m_menu_record);

    m_menu_record_recording.set_label("Toggle recording");
    m_menu_record_recording.add_accelerator("activate", m_accelgroup, 'r', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_record_recording.signal_activate().connect([&]{menu_callback(EDIT_MENU_RECORD);});
    m_menu_record_state = false;
    m_submenu_record.append(m_menu_record_recording);

    m_menu_record_quantized.set_label("Quantized record");
    m_menu_record_quantized.set_active(m_sequence->get_quantized_rec());
    m_menu_record_quantized.signal_toggled().connect([&]{menu_callback(EDIT_MENU_RECORD_QUANTIZED);});
    m_submenu_record.append(m_menu_record_quantized);

    m_menu_record_through.set_label("Pass events to ouput");
    m_menu_record_through.set_active(m_sequence->get_thru());
    m_menu_record_through.signal_toggled().connect([&]{menu_callback(EDIT_MENU_RECORD_THRU);});
    m_submenu_record.append(m_menu_record_through);

    m_menu_playback.set_label("_Playback");
    m_menu_playback.set_use_underline(true);
    m_menu_playback.set_submenu(m_submenu_playback);
    m_menu.append(m_menu_playback);

    m_menu_playback_playing.set_label("Toggle playback");
    m_menu_playback_playing.add_accelerator("activate", m_accelgroup, 'e', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_playback_playing.signal_activate().connect([&]{menu_callback(EDIT_MENU_PLAY);});
    m_menu_playing_state = false;
    m_submenu_playback.append(m_menu_playback_playing);

    m_menu_playback_resume.set_label("Resume notes when enabling");
    m_menu_playback_resume.set_active(m_sequence->get_resume());
    m_menu_playback_resume.signal_activate().connect([&]{menu_callback(EDIT_MENU_RESUME);});
    m_submenu_playback.append(m_menu_playback_resume);


    // toolbar
    m_toolbar.set_size_request(0, 55);
    m_toolbar.get_style_context()->add_class("toolbar");
    m_toolbar.get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    m_toolbar.set_spacing(c_toolbar_spacing);

    m_toolbar_name.set_name("seqname");
    m_toolbar_name.set_tooltip_text("Sequence name");
    m_toolbar_name.set_width_chars(16);
    m_toolbar_name.set_alignment(0);
    m_toolbar_name.signal_activate().connect([&]{clear_focus();});
    m_toolbar_name.signal_focus_out_event().connect([&](GdkEventFocus *focus)->bool{
        string s = m_toolbar_name.get_text();
        m_sequence->set_name(s);
        update_window_title();
        return false;
    });
    m_toolbar_name.set_text(m_sequence->get_name());
    m_toolbar.pack_start(m_toolbar_name, true, true);

    m_toolbar_bpm.set_name("bpm");
    m_toolbar_bpm.set_tooltip_text("Beats per measure");
    m_toolbar_bpm.set_alignment(0.5);
    m_toolbar_bpm.set_width_chars(2);
    m_toolbar_bpm.signal_activate().connect([&]{clear_focus();});
    m_toolbar_bpm.signal_focus_out_event().connect([&](GdkEventFocus *focus)->bool{
        string s = m_toolbar_bpm.get_text();
        int bpm = atof(s.c_str());
        if (bpm > 0) m_bpm = bpm;
        m_sequence->set_bpm(m_bpm);
        m_sequence->set_length(m_measures * m_bpm * ((c_ppqn * 4) / m_bw));
        m_toolbar_bpm.set_text(to_string(m_bpm));
        return false;
    });
    m_bpm = m_sequence->get_bpm();
    m_toolbar_bpm.set_text(to_string(m_bpm));
    m_toolbar.pack_start(m_toolbar_bpm, false, false);

    m_toolbar_slash.set_label("/");
    m_toolbar.pack_start(m_toolbar_slash, false, false);

    m_toolbar_bw.set_name("bw");
    m_toolbar_bw.set_tooltip_text("Beat unit");
    m_toolbar_bw.set_alignment(0.5);
    m_toolbar_bw.set_width_chars(2);
    m_toolbar_bw.signal_activate().connect([&]{clear_focus();});
    m_toolbar_bw.signal_focus_out_event().connect([&](GdkEventFocus *focus)->bool{
        string s = m_toolbar_bw.get_text();
        int bw = atof(s.c_str());
        if (bw > 0) m_bw = bw;
        m_sequence->set_bw(m_bw);
        m_sequence->set_length(m_measures * m_bpm * ((c_ppqn * 4) / m_bw));
        m_toolbar_bw.set_text(to_string(m_bw));
        return false;
    });
    m_bw = m_sequence->get_bw();
    m_toolbar_bw.set_text(to_string(m_bw));
    m_toolbar.pack_start(m_toolbar_bw, false, false);

    m_toolbar_times.set_label("x");
    m_toolbar.pack_start(m_toolbar_times, false, false);

    m_toolbar_measures.set_name("measures");
    m_toolbar_measures.set_tooltip_text("Number of measures");
    m_toolbar_measures.set_alignment(0.5);
    m_toolbar_measures.set_width_chars(2);
    m_toolbar_measures.signal_activate().connect([&]{clear_focus();});
    m_toolbar_measures.signal_focus_out_event().connect([&](GdkEventFocus *focus)->bool{
        string s = m_toolbar_measures.get_text();
        int measures = atof(s.c_str());
        if (measures > 0) m_measures = measures;
        m_sequence->set_length(m_measures * m_bpm * ((c_ppqn * 4) / m_bw));
        m_toolbar_measures.set_text(to_string(m_measures));
        return false;
    });
    long units = ((m_sequence->get_bpm() * (c_ppqn * 4)) /  m_sequence->get_bw() );
    m_measures = (m_sequence->get_length() / units);
    if (m_sequence->get_length() % units != 0) m_measures++;
    m_toolbar_measures.set_text(to_string(m_measures));
    m_toolbar.pack_start(m_toolbar_measures, false, false);


    m_toolbar_snap_active.set_label("Snap");
    m_toolbar_snap_active.set_tooltip_text("Snap to grid");
    m_toolbar_snap_active.get_style_context()->add_class("nomargin");
    m_toolbar_snap_active.get_style_context()->add_class("togglebutton");
    m_toolbar_snap_active.set_focus_on_click(false);
    m_toolbar_snap_active.signal_clicked().connect([&]{
        m_pianoroll.set_snap_active(m_toolbar_snap_active.get_active());
        m_eventroll.set_snap_active(m_toolbar_snap_active.get_active());
    });
    m_toolbar_snap_active.set_active(true);

    m_toolbar_length_label.set_label("Note");
    m_toolbar_length_label.set_sensitive(false);
    m_toolbar_length_label.get_style_context()->add_class("nomargin");
    m_toolbar_snap.set_tooltip_text("Grid size");
    m_toolbar_snap.append("1");
    m_toolbar_length.set_tooltip_text("Note size");
    m_toolbar_length.append("1");
    char s[13];
    for (int i = 1; i < 8; i++) {
        snprintf(s, sizeof(s), "1/%d", int(pow(2, i)));
        m_toolbar_snap.append(s);
        m_toolbar_length.append(s);
    }
    for (int i = 0; i < 7; i++) {
        snprintf(s, sizeof(s), "1/%d", int(3 * pow(2, i)));
        m_toolbar_snap.append(s);
        m_toolbar_length.append(s);
    }
    m_toolbar_snap.signal_changed().connect([&]{
        int ticks = divs_to_ticks[m_toolbar_snap.get_active_text()];
        m_pianoroll.set_snap(ticks);
        m_pianoroll.queue_draw_background();
        m_eventroll.set_snap(ticks);
        m_eventroll.queue_draw_background();
        m_sequence->set_snap_tick(ticks);
    });
    m_toolbar_length.signal_changed().connect([&]{
        int ticks = divs_to_ticks[m_toolbar_length.get_active_text()];
        m_pianoroll.set_note_length(ticks);
    });
    m_toolbar_snap.set_active(ticks_to_divs[m_pianoroll.get_snap()]);
    m_toolbar_length.set_active(ticks_to_divs[m_pianoroll.get_note_length()]);
    m_toolbar_snap.set_focus_on_click(false);
    m_toolbar_length.set_focus_on_click(false);

    m_toolbar.pack_start(m_toolbar_snap_active, false, false);
    m_toolbar.pack_start(m_toolbar_snap, false, false);
    m_toolbar.pack_start(m_toolbar_length_label, false, false);
    m_toolbar.pack_start(m_toolbar_length, false, false);

    m_toolbar_bus_label.set_label("Output");
    m_toolbar_bus_label.set_sensitive(false);
    m_toolbar_bus_label.get_style_context()->add_class("nomargin");
    m_toolbar_bus.set_can_focus(false);
    m_toolbar_bus.set_editable(false);
    m_toolbar_bus.get_style_context()->add_class("nomargin");
    m_toolbar_bus_dropdown.set_sensitive(true);
    m_toolbar_bus_dropdown.set_direction(Gtk::ARROW_DOWN);

    m_toolbar.pack_end(m_toolbar_bus_dropdown, false, false);
    m_toolbar.pack_end(m_toolbar_bus, true, true);
    m_toolbar.pack_end(m_toolbar_bus_label, false, false);


    // layout
    m_vbox.pack_start(m_menu, false, false);
    m_vbox.pack_start(m_toolbar, false, false);
    m_vbox.pack_end(m_grid, true, true);


    m_timeroll.set_size_request(-1, c_timeroll_height);
    m_grid.attach(m_timeroll, 0, 0, 3, 1);


    m_pianokeys_scroller.set_size_request(c_keys_width, -1);
    m_pianokeys.set_size_request(-1, c_key_height * c_num_keys);
    m_pianokeys_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianokeys_scroller.add(m_pianokeys);
    m_grid.attach(m_pianokeys_scroller, 0, 1);

    m_pianoroll_scroller.set_hexpand(true);
    m_pianoroll_scroller.set_vexpand(true);
    m_pianoroll.set_size_request(-1, c_key_height * c_num_keys);
    m_pianoroll_scroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_EXTERNAL);
    m_pianoroll_scroller.set_vadjustment(m_pianokeys_scroller.get_vadjustment());
    m_pianoroll_scroller.add(m_pianoroll);
    m_grid.attach(m_pianoroll_scroller, 1, 1);

    m_vscrollbar.set_orientation(ORIENTATION_VERTICAL);
    m_vscrollbar.set_adjustment(m_pianokeys_scroller.get_vadjustment());
    m_vscrollbar.get_style_context()->add_class("editwindow-vscrollbar");
    m_grid.attach(m_vscrollbar, 2, 1);

    m_eventroll.set_size_request(-1, c_eventroll_height + 1);
    m_grid.attach(m_eventroll, 1, 2, 2, 1);

    m_dataroll.set_size_request(-1, c_dataroll_height + 1);
    m_grid.attach(m_dataroll, 0, 3, 3, 1);

    m_hscrollbar.get_style_context()->add_class("editwindow-hscrollbar");
    m_grid.attach(m_hscrollbar, 0, 4, 3, 1);

    m_event_dropdown_label.set_label("Note On");
    m_event_dropdown_label.set_halign(Gtk::ALIGN_END);
    m_event_dropdown.add(m_event_dropdown_label);
    m_event_dropdown.set_sensitive(true);
    m_event_dropdown.get_style_context()->add_class("editwindow-eventbutton");
    m_grid.attach(m_event_dropdown, 0, 2, 1, 1);



    int height = c_key_height * c_num_keys;
    m_pianokeys_scroller.get_vadjustment()->configure((height - 500) / 2.0, 0.0, height, c_key_height, c_key_height * 12, 1);

    m_hscrollbar.get_adjustment()->configure(0, 0, m_sequence->get_length(), 1, 1, 1);

    create_midibus_menu();
    update_midibus_name();
    create_event_menu();

    signal_key_press_event().connect(mem_fun(*this, &EditWindow::on_key_press), false);
    signal_key_release_event().connect(mem_fun(*this, &EditWindow::on_key_release), false);

    // timer callback (50 fps)
    Glib::signal_timeout().connect(mem_fun(*this, &EditWindow::timer_callback), 20);

    // zoom callback
    m_timeroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_pianoroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_eventroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_dataroll.signal_scroll.connect(mem_fun(*this, &EditWindow::scroll_callback));
    m_pianoroll.signal_focus.connect(mem_fun(*this, &EditWindow::focus_callback));
    m_eventroll.signal_focus.connect(mem_fun(*this, &EditWindow::focus_callback));

    signal_size_allocate().connect([&](Gdk::Rectangle){update_hscrollbar_visibility();});
    signal_focus_out_event().connect([&](GdkEventFocus *e)->bool{on_focus_out();return false;});
    m_submenu_edit.signal_popped_up().connect([&](const Gdk::Rectangle*, const Gdk::Rectangle*, bool, bool){on_focus_out();});
    m_submenu_transport.signal_popped_up().connect([&](const Gdk::Rectangle*, const Gdk::Rectangle*, bool, bool){on_focus_out();});
    m_submenu_record.signal_popped_up().connect([&](const Gdk::Rectangle*, const Gdk::Rectangle*, bool, bool){on_focus_out();});


    if (m_sequence->get_alt_cc() != -1) set_data_type(EVENT_CONTROL_CHANGE, m_sequence->get_alt_cc(), true);

    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq192_32_xpm));

    clear_focus();
    update_window_title();
    set_position(Gtk::WIN_POS_CENTER);
    resize(1024, 600);
    show_all();

}

EditWindow::~EditWindow()
{
}


bool
EditWindow::on_key_press(GdkEventKey* event)
{
    if (get_focus() != NULL) {
        string focus = get_focus()->get_name();
        if (event->keyval == GDK_KEY_space && focus == "seqname") return false;
        if ((event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_Right || event->keyval == GDK_KEY_Delete) &&
            (focus == "seqname" || focus == "bpm" || focus == "bw" || focus == "measures"))
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
            menu_callback(EDIT_MENU_TRANSPOSE, event->state & GDK_CONTROL_MASK ? 12 : 1);
            return true;
        case GDK_KEY_Down:
            menu_callback(EDIT_MENU_TRANSPOSE, event->state & GDK_CONTROL_MASK ? -12 : -1);
            return true;
        case GDK_KEY_Left:
            menu_callback(EDIT_MENU_MOVE, event->state & GDK_SHIFT_MASK ? -1 : -m_pianoroll.m_snap);
            return true;
        case GDK_KEY_Right:
            menu_callback(EDIT_MENU_MOVE, event->state & GDK_SHIFT_MASK ? 1 : m_pianoroll.m_snap);
            return true;
        case GDK_KEY_Delete:
            menu_callback(EDIT_MENU_DELETE);
            return true;
        case GDK_KEY_Alt_L:
            m_pianoroll.set_snap_bypass(true);
            m_eventroll.set_snap_bypass(true);
            m_toolbar_snap_active.get_style_context()->add_class("bypass");
        default:
            return false;
    }


    return false;
}


bool
EditWindow::on_key_release(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Alt_L:
            m_pianoroll.set_snap_bypass(false);
            m_eventroll.set_snap_bypass(false);
            m_toolbar_snap_active.get_style_context()->remove_class("bypass");
        default:
            return false;
    }

    return false;
}

void
EditWindow::on_focus_out()
{
    m_pianoroll.set_snap_bypass(false);
    m_eventroll.set_snap_bypass(false);
    m_toolbar_snap_active.get_style_context()->remove_class("bypass");
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
    menu_callback(action, 0);
}

void
EditWindow::menu_callback(edit_menu_action action, double data1)
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
        case EDIT_MENU_INVERT:
            if (m_focus == "eventroll") {
                m_sequence->select_events(m_status, m_cc, true);
            } else {
                m_sequence->select_events(EVENT_NOTE_ON, 0, true);
                m_sequence->select_events(EVENT_NOTE_OFF, 0, true);
            }
            break;
        case EDIT_MENU_SELECTALL:
            if (m_focus == "eventroll") {
                m_sequence->select_events(m_status, m_cc);
            } else {
                m_sequence->select_events(EVENT_NOTE_ON, 0);
                m_sequence->select_events(EVENT_NOTE_OFF, 0);
            }
            break;
        case EDIT_MENU_UNSELECT:
            m_sequence->unselect();
            break;
        case EDIT_MENU_TRANSPOSE:
            m_sequence->transpose_notes(data1);
            break;
        case EDIT_MENU_QUANTIZE:
            if (m_focus == "eventroll") {
                m_sequence->quantize_events(m_status, m_cc, m_pianoroll.m_snap, 1);
            } else {
                m_sequence->quantize_events(EVENT_NOTE_ON, 0, m_pianoroll.m_snap, 1, true);
            }
            break;
        case EDIT_MENU_MOVE:
            m_sequence->shift_events(data1);
            break;
        case EDIT_MENU_MULTIPLY:
            m_sequence->multiply_pattern(data1);
            break;
        case EDIT_MENU_REVERSE:
            m_sequence->reverse_pattern();
            break;
        case EDIT_MENU_CLOSE:
            close();
            break;

        case EDIT_MENU_RECORD:
            // m_menu_record_state = !m_menu_record_state;
            m_perform->get_master_midi_bus()->set_sequence_input(m_menu_record_state ? NULL : m_sequence);
            break;
        case EDIT_MENU_RECORD_QUANTIZED:
            m_sequence->get_quantized_rec(m_menu_record_quantized.get_active());
            break;
        case EDIT_MENU_RECORD_THRU:
            m_sequence->set_thru(m_menu_record_through.get_active());
            break;

        case EDIT_MENU_PLAY:
            m_sequence->set_playing(!m_menu_playing_state);
            break;
        case EDIT_MENU_RESUME:
            m_sequence->set_resume(m_menu_playback_resume.get_active());
            break;
    }
}

bool
EditWindow::timer_callback()
{
    if (!m_perform->is_active(m_seqnum)) {
        close();
        return false;
    }

    if (m_bg_seqnum != -1 && !m_perform->is_active(m_bg_seqnum)) {
        set_background_sequence(-1, NULL);
    }

    bool rec = m_sequence->get_recording();
    if (m_menu_record_state != rec) {
        m_menu_record_state = rec;
        if (m_menu_record_state) {
            m_menu_record.get_style_context()->add_class("recording");
        } else {
            m_menu_record.get_style_context()->remove_class("recording");
        }
    }

    bool playing = m_sequence->get_playing();
    if (m_menu_playing_state != playing) {
        m_menu_playing_state = playing;
        if (m_menu_playing_state) {
            m_menu_playback.get_style_context()->add_class("playing");
        } else {
            m_menu_playback.get_style_context()->remove_class("playing");
        }
    }

    update_midibus_name();

    auto adj = m_hscrollbar.get_adjustment();
    adj->set_lower(0);
    adj->set_upper(m_sequence->get_length() + c_ppqn);
    adj->set_page_size(m_pianoroll.get_width() * m_pianoroll.get_zoom());
    adj->set_step_increment(c_ppqn / 4 * m_pianoroll.get_zoom());
    adj->set_page_increment(c_ppqn * m_sequence->get_bpm() * 4.0 / m_sequence->get_bw() * m_pianoroll.get_zoom());
    if (adj->get_value() + m_pianoroll.get_width() * m_pianoroll.get_zoom() > adj->get_upper()) adj->set_value(adj->get_upper());

    m_timeroll.set_hscroll(adj->get_value());
    m_eventroll.set_hscroll(adj->get_value());
    m_pianoroll.set_hscroll(adj->get_value());
    m_dataroll.set_hscroll(adj->get_value());

    if (m_sequence->is_dirty_edit()) {
        m_pianoroll.queue_draw_background();
        m_eventroll.queue_draw_background();
        m_dataroll.queue_draw_background();
    }

    m_timeroll.queue_draw();
    m_eventroll.queue_draw();
    m_pianoroll.queue_draw();
    m_dataroll.queue_draw();

    if (m_sequence->m_have_undo && !m_menu_edit_undo.get_sensitive()) m_menu_edit_undo.set_sensitive(true);
    else if (!m_sequence->m_have_undo && m_menu_edit_undo.get_sensitive()) m_menu_edit_undo.set_sensitive(false);
    if (m_sequence->m_have_redo && !m_menu_edit_redo.get_sensitive()) m_menu_edit_redo.set_sensitive(true);
    else if (!m_sequence->m_have_redo && m_menu_edit_redo.get_sensitive()) m_menu_edit_redo.set_sensitive(false);

    return true;
}


bool
EditWindow::scroll_callback(GdkEventScroll* event)
{
    guint modifiers = gtk_accelerator_get_default_mod_mask ();

    if ((event->state & modifiers) == GDK_CONTROL_MASK)
    {
        double zoom = m_pianoroll.get_zoom();
        if (event->direction == GDK_SCROLL_DOWN)
        {
            m_timeroll.set_zoom(zoom * 2);
            m_eventroll.set_zoom(zoom * 2);
            m_pianoroll.set_zoom(zoom * 2);
            m_dataroll.set_zoom(zoom * 2);
        }
        else if (event->direction == GDK_SCROLL_UP)
        {
            m_timeroll.set_zoom(zoom / 2);
            m_eventroll.set_zoom(zoom / 2);
            m_pianoroll.set_zoom(zoom / 2);
            m_dataroll.set_zoom(zoom / 2);

        }
        update_hscrollbar_visibility();
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
    clear_focus();
}

void
EditWindow::clear_focus()
{
    m_toolbar_bpm.select_region(0, 0);
    m_toolbar_bw.select_region(0, 0);
    m_toolbar_measures.select_region(0, 0);
    m_pianokeys_scroller.set_can_focus(true);
    m_pianokeys_scroller.grab_focus();
    m_pianokeys_scroller.set_can_focus(false);
}


void
EditWindow::update_midibus_name()
{

    if (m_sequence->get_midi_bus() != m_midibus || m_sequence->get_midi_channel() != m_midichannel)
    {
        m_midibus = m_sequence->get_midi_bus();
        m_midichannel = m_sequence->get_midi_channel();

        mastermidibus *mmb =  m_perform->get_master_midi_bus();
        string bus = mmb->get_midi_out_bus_name(m_midibus);

        string channel = to_string(m_midichannel + 1);
        int instrument = global_user_midi_bus_definitions[m_midibus].instrument[m_midichannel];
        if (instrument >= 0 && instrument < c_maxBuses)
        {
            channel = channel + " " + global_user_instrument_definitions[instrument].instrument;
        }

        m_toolbar_bus.set_text(bus + ": " + channel);

    }

}

void EditWindow::create_midibus_menu()
{

    m_toolbar_bus_menu.set_valign(Gtk::ALIGN_START);
    m_toolbar_bus_menu.set_halign(Gtk::ALIGN_END);

    mastermidibus *masterbus = m_perform->get_master_midi_bus();
    for ( int i=0; i< masterbus->get_num_out_buses(); i++ ){
        Menu *menu_channels = new Menu();

        MenuItem * menu_item_bus = new MenuItem(masterbus->get_midi_out_bus_name(i));
        menu_item_bus->set_submenu(*menu_channels);
        m_toolbar_bus_menu.append(*menu_item_bus);


        for( int j=0; j<16; j++ ){
            std::string name = to_string(j + 1);
            int instrument = global_user_midi_bus_definitions[i].instrument[j];
            if ( instrument >= 0 && instrument < c_maxBuses )
            {
                name = name + " " + global_user_instrument_definitions[instrument].instrument;
            }

            MenuItem * menu_item_channel = new MenuItem(name);
            menu_item_channel->signal_activate().connect([&,i,j]{
                m_sequence->set_midi_bus(i);
                m_sequence->set_midi_channel(j);
                set_data_type(m_status, m_cc); // update event dropdown tooltip
                m_pianokeys.queue_draw();
            });
            menu_channels->append(*menu_item_channel);
        }

    }

    m_toolbar_bus_menu.show_all();
    m_toolbar_bus_dropdown.set_popup(m_toolbar_bus_menu);

}


void
EditWindow::create_event_menu()
{

    m_event_menu.set_valign(Gtk::ALIGN_START);
    m_event_menu.set_halign(Gtk::ALIGN_START);

    m_menu_item_noteon.set_label("Note On");
    m_menu_item_noteon.signal_toggled().connect([&]{set_data_type(EVENT_NOTE_ON, 0);});
    m_event_menu.append(m_menu_item_noteon);

    m_menu_item_noteoff.set_label("Note Off");
    m_menu_item_noteoff.signal_toggled().connect([&]{set_data_type(EVENT_NOTE_OFF, 0);});
    m_event_menu.append(m_menu_item_noteoff);

    m_menu_item_aftertouch.set_label("Aftertouch");
    m_menu_item_aftertouch.signal_toggled().connect([&]{set_data_type(EVENT_AFTERTOUCH, 0);});
    m_event_menu.append(m_menu_item_aftertouch);

    m_menu_item_program.set_label("Program Change");
    m_menu_item_program.signal_toggled().connect([&]{set_data_type(EVENT_PROGRAM_CHANGE, 0);});
    m_event_menu.append(m_menu_item_program);

    m_menu_item_pressure.set_label("Channel Pressure");
    m_menu_item_pressure.signal_toggled().connect([&]{set_data_type(EVENT_CHANNEL_PRESSURE, 0);});
    m_event_menu.append(m_menu_item_pressure);

    m_menu_item_pitch.set_label("Pitch Wheel");
    m_menu_item_pitch.signal_toggled().connect([&]{set_data_type(EVENT_PITCH_WHEEL, 0);});
    m_event_menu.append(m_menu_item_pitch);

    m_menu_item_control.set_label("Control Change");
    m_event_menu.append(m_menu_item_control);
    m_menu_item_control.set_submenu(m_submenu_control);

    for (int i=0; i<128; i++) {
        m_menu_items_control[i] = new CheckMenuItem();
        m_menu_items_control[i]->signal_toggled().connect([&, i]{
            set_data_type(EVENT_CONTROL_CHANGE, i);
        });
        m_submenu_control.append(*m_menu_items_control[i]);
    }

    m_menu_item_alt_control.set_label("Alt Control Change");
    m_event_menu.append(m_menu_item_alt_control);
    m_menu_item_alt_control.set_submenu(m_submenu_alt_control);

    m_menu_items_alt_control[0] = new CheckMenuItem();
    m_menu_items_alt_control[0]->set_label("None");
    m_menu_items_alt_control[0]->signal_toggled().connect([&]{
        set_data_type(0, 0, true);
        set_data_type(m_status, m_cc);
        m_menu_item_alt_control.set_label("Alt Control Change");
        m_menu_item_toggle_alt_control.set_sensitive(false);
        m_sequence->set_alt_cc(-1);
    });
    m_submenu_alt_control.append(*m_menu_items_alt_control[0]);
    for (int i=0; i<128; i++) {
        m_menu_items_alt_control[i+1] = new CheckMenuItem();
        m_menu_items_alt_control[i+1]->signal_toggled().connect([&, i]{
            set_data_type(EVENT_CONTROL_CHANGE, i, true);
        });
        m_submenu_alt_control.append(*m_menu_items_alt_control[i+1]);
    }

    m_menu_item_toggle_alt_control.set_label("Toggle Alt Control");
    m_event_menu.append(m_menu_item_toggle_alt_control);
    m_menu_item_toggle_alt_control.signal_activate().connect([&]{
        if (m_alt_status != 0) {
            if (m_alt_control_view) {
                set_data_type(m_status, m_cc);
            } else {
                set_data_type(m_alt_status, m_alt_cc, true);
            }
        }
    });
    m_menu_item_toggle_alt_control.add_accelerator("activate", m_accelgroup, 't', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_menu_item_toggle_alt_control.set_sensitive(false);

    m_event_menu.show_all();
    m_event_dropdown.set_popup(m_event_menu);
    m_event_dropdown.signal_clicked().connect([&]{update_event_menu();});

}

void
EditWindow::update_event_menu()
{

    m_menu_item_noteon.set_active(false);
    m_menu_item_noteoff.set_active(false);
    m_menu_item_aftertouch.set_active(false);
    m_menu_item_program.set_active(false);
    m_menu_item_pitch.set_active(false);
    m_menu_item_pressure.set_active(false);

    m_menu_item_noteon.get_style_context()->remove_class("checked");
    m_menu_item_noteoff.get_style_context()->remove_class("checked");
    m_menu_item_aftertouch.get_style_context()->remove_class("checked");
    m_menu_item_program.get_style_context()->remove_class("checked");
    m_menu_item_pitch.get_style_context()->remove_class("checked");
    m_menu_item_pressure.get_style_context()->remove_class("checked");

    int midi_bus = m_sequence->get_midi_bus();
    int midi_ch = m_sequence->get_midi_channel();
    for (int i=0; i<128; i++) {
        m_menu_items_control[i]->set_active(false);
        m_menu_items_control[i]->get_style_context()->remove_class("checked");
        m_menu_items_alt_control[i+1]->set_active(false);
        m_menu_items_alt_control[i+1]->get_style_context()->remove_class("checked");

        string ccname = to_string(i);
        int instrument = global_user_midi_bus_definitions[midi_bus].instrument[midi_ch];
        if (instrument > -1 && instrument < c_max_instruments)
        {
            if (global_user_instrument_definitions[instrument].controllers_active[i])
                ccname = to_string(i) + " " + global_user_instrument_definitions[instrument].controllers[i];
        } else {
            ccname = c_controller_names[i];
        }
        m_menu_items_control[i]->set_label(ccname);
        m_menu_items_alt_control[i+1]->set_label(ccname);
    }

    unsigned char status, cc;
    m_sequence->reset_draw_list();
    while (m_sequence->get_next_event( &status, &cc ) == true)
    {
        switch (status) {
            case EVENT_NOTE_OFF:
                m_menu_item_noteoff.get_style_context()->add_class("checked");
                break;
            case EVENT_NOTE_ON:
                m_menu_item_noteon.get_style_context()->add_class("checked");
                break;
            case EVENT_AFTERTOUCH:
                m_menu_item_aftertouch.get_style_context()->add_class("checked");
                break;
            case EVENT_CONTROL_CHANGE:
                m_menu_items_control[cc]->get_style_context()->add_class("checked");
                m_menu_items_alt_control[cc+1]->get_style_context()->add_class("checked");
                break;
            case EVENT_PITCH_WHEEL:
                m_menu_item_pitch.get_style_context()->add_class("checked");
                break;
            case EVENT_PROGRAM_CHANGE:
                m_menu_item_program.get_style_context()->add_class("checked");
                break;
            case EVENT_CHANNEL_PRESSURE:
                m_menu_item_pressure.get_style_context()->add_class("checked");
                break;
        }
    }
}

void
EditWindow::set_data_type(unsigned char status, unsigned char control, bool alt)
{

    if (alt) {
        m_alt_status = status;
        m_alt_cc = control;
        m_alt_control_view = true;
        m_menu_item_alt_control.set_label("Alt Control Change (" + to_string(control) + ")");
        m_menu_item_toggle_alt_control.set_sensitive(true);
        m_sequence->set_alt_cc(control);
    } else {
        m_status = status;
        m_cc = control;
        m_alt_control_view = false;
    }

    m_eventroll.set_data_type(status, control);
    m_dataroll.set_data_type(status, control, alt);

    m_event_dropdown.set_tooltip_text("");

    string label;
    switch (status) {
        case EVENT_NOTE_OFF:
            label = "Note Off";
            break;
        case EVENT_NOTE_ON:
            label = "Note On";
            break;
        case EVENT_AFTERTOUCH:
            label = "Aftertouch";
            break;
        case EVENT_CONTROL_CHANGE:
        {
            label = "CC " + to_string(control);
            int midi_bus = m_sequence->get_midi_bus();
            int midi_ch = m_sequence->get_midi_channel();
            string ccname = c_controller_names[control];
            int instrument = global_user_midi_bus_definitions[midi_bus].instrument[midi_ch];
            if (instrument > -1 && instrument < c_max_instruments)
            {
                if (global_user_instrument_definitions[instrument].controllers_active[control])
                    ccname = global_user_instrument_definitions[instrument].controllers[control];
            }
            m_event_dropdown.set_tooltip_text(ccname);
            break;
        }
        case EVENT_PITCH_WHEEL:
            label = "Pitch Wheel";
            break;
        case EVENT_PROGRAM_CHANGE:
            label = "Program";
            break;
        case EVENT_CHANNEL_PRESSURE:
            label = "Pressure";
            break;
    }

    m_event_dropdown_label.set_label(label);
}

void
EditWindow::update_background_menu()
{
    int off_i = ((int) m_seqnum / c_seqs_in_set) * c_seqs_in_set;
    bool check = false;
    m_menu_items_bgseq[c_seqs_in_set]->set_active(false);
    m_menu_items_bgseq[c_seqs_in_set]->get_style_context()->remove_class("checked");
    for (int i=0; i < c_seqs_in_set; i++) {
        m_menu_items_bgseq[i]->set_active(false);
        m_menu_items_bgseq[i]->get_style_context()->remove_class("checked");
        m_menu_items_bgseq[i]->hide();
        if (m_perform->is_active(i + off_i) && i + off_i != m_seqnum) {
            m_menu_items_bgseq[i]->set_label(m_perform->get_sequence(i + off_i)->get_name());
            if (i + off_i == m_bg_seqnum) {
                m_menu_items_bgseq[i]->get_style_context()->add_class("checked");
                check = true;
            }
            m_menu_items_bgseq[i]->show();
        }
    }
    if (!check) m_menu_items_bgseq[c_seqs_in_set]->get_style_context()->add_class("checked");

}

void
EditWindow::set_background_sequence(int i, sequence * seq)
{
    m_bg_seqnum = i;
    m_pianoroll.m_bg_sequence = seq;
    m_pianoroll.queue_draw_background();
}



void
EditWindow::update_hscrollbar_visibility() {
    auto adj = m_hscrollbar.get_adjustment();
    if (m_pianoroll.get_width() * m_pianoroll.get_zoom() >= adj->get_upper()) m_hscrollbar.get_style_context()->remove_class("show");
    else m_hscrollbar.get_style_context()->add_class("show");
}

void
EditWindow::update_window_title()
{
    std::string title = string(PACKAGE) + " - " + m_toolbar_name.get_text();
    set_title(title.c_str());
}
