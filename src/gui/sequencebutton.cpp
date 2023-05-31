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


#include "sequencebutton.h"
#include "editwindow.h"
#include "../core/globals.h"

SequenceButton::SequenceButton(perform * p, MainWindow * m, int seqpos)
{
    m_perform = p;
    m_mainwindow = m;
    m_seqpos = seqpos;
    m_click = false;
    m_middle_click = false;
    m_drag_start = false;
    m_last_seqnum = -1;

    m_last_marker_pos = 0;
    m_next_marker_pos = 0;

    set_last_sequence_number();
    set_can_focus(true);

    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

    add_events( Gdk::BUTTON_PRESS_MASK |
        Gdk::BUTTON_RELEASE_MASK |
        Gdk::ENTER_NOTIFY_MASK |
        Gdk::LEAVE_NOTIFY_MASK
    );
}

SequenceButton::~SequenceButton()
{

}

int
SequenceButton::get_sequence_number() {
    return m_seqpos + m_perform->get_screenset() * c_seqs_in_set;
}

int
SequenceButton::get_last_sequence_number() {
    return m_last_seqnum;
}

void
SequenceButton::set_last_sequence_number() {
    m_last_seqnum = get_sequence_number();
}

sequence *
SequenceButton::get_sequence() {
    int seqnum = get_sequence_number();
    if (m_perform->is_active(seqnum)) {
        return m_perform->get_sequence(seqnum);
    } else {
        return NULL;
    }
}

bool
SequenceButton::set_active(bool active) {
    bool changed = active != m_active;
    m_active = active;
    return changed;
}

void
SequenceButton::draw_background()
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    sequence * seq = get_sequence();
    if (seq != NULL) {

        color color;
        bool playing = seq->get_playing();

        // background
        color = playing ? c_sequence_background_on : c_sequence_background;
        cr->set_source_rgb(color.r, color.g, color.b);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        // font
        Pango::FontDescription font;
        int text_width;
        int text_height;
        font.set_family(c_font);
        font.set_size(c_sequence_fontsize * Pango::SCALE);
        font.set_weight(Pango::WEIGHT_NORMAL);

        // text color
        color = playing ? c_sequence_text_on : c_sequence_text;
        if (get_sequence()->get_recording()) {
            color = c_sequence_text_record;
        }

        // time signature
        string signature = to_string(seq->get_bpm()) + "/" + to_string(seq->get_bw());
        auto siglayout = create_pango_layout(signature);
        int sigwidth;
        font.set_size(c_sequence_signature_size * Pango::SCALE);
        siglayout->set_font_description(font);
        siglayout->get_pixel_size(sigwidth, text_height);
        if (seq->is_sync_reference()) {
            cr->set_source_rgba(c_sequence_syncref_bg.r, c_sequence_syncref_bg.g, c_sequence_syncref_bg.b, 0.5);
            cr->rectangle(width - c_sequence_padding - sigwidth - 1, c_sequence_padding - 1, sigwidth + 2, text_height + 2);
            cr->fill();
            cr->set_source_rgba(c_sequence_syncref.r, c_sequence_syncref.g, c_sequence_syncref.b, 1.0);
        } else {
            cr->set_source_rgba(color.r, color.g, color.b, 0.6);
        }
        cr->move_to(width - c_sequence_padding - sigwidth, c_sequence_padding);
        siglayout->show_in_cairo_context(cr);

        // sequence name
        auto name = create_pango_layout(seq->get_name());
        font.set_size(c_sequence_fontsize * Pango::SCALE);
        name->set_font_description(font);
        name->get_pixel_size(text_width, text_height);
        name->set_width((width - c_sequence_padding * 2 - sigwidth) * Pango::SCALE);
        name->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->set_source_rgb(color.r, color.g, color.b);
        cr->move_to(c_sequence_padding, c_sequence_padding);
        name->show_in_cairo_context(cr);

        // queued ?
        bool queued = get_sequence()->is_queued();
        int queued_width = 0;
        if (queued)
        {
            color = playing ? c_sequence_text_on : c_sequence_text;
            cr->set_source_rgb(color.r, color.g, color.b);
            auto queued = create_pango_layout("⌛");
            queued->set_font_description(font);
            queued->get_pixel_size(queued_width, text_height);
            cr->move_to(width - c_sequence_padding - queued_width, c_sequence_padding + text_height + 1);
            queued->show_in_cairo_context(cr);
        }

        // bus & channel name
        color = playing ? c_sequence_text_on : c_sequence_text;
        int bus = seq->get_midi_bus();
        int chan = seq->get_midi_channel();
        string busname = global_user_midi_bus_definitions[bus].alias;
        if (busname.empty()) busname = "Bus " + to_string(bus + 1);
        if (!global_user_instrument_definitions[bus * 16 + chan].instrument.empty()) {
             busname += ": " + global_user_instrument_definitions[bus * 16 + chan].instrument;
        } else {
            busname += ": Ch " + to_string(chan + 1);
        }

        auto channame = create_pango_layout(busname);
        channame->set_font_description(font);
        channame->get_pixel_size(text_width, text_height);
        channame->set_width((width - c_sequence_padding * 2 - queued_width) * Pango::SCALE);
        channame->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->set_source_rgba(color.r, color.g, color.b, 0.6);
        cr->move_to(c_sequence_padding, c_sequence_padding + text_height);
        channame->show_in_cairo_context(cr);


        // instrument color
        string instrument_color = global_user_instrument_definitions[bus * 16 + chan].color;
        if (!global_user_instrument_definitions[bus * 16 + chan].color.empty()) {
            color = global_user_instrument_colors[bus * 16 + chan];
            cr->set_source_rgba(color.r, color.g, color.b, 0.8);
        } else {
            // use default text color
            cr->set_source_rgba(color.r, color.g, color.b, 0.3);
        }
        cr->rectangle(0, c_sequence_padding * 2 + text_height * 2 - 2,width, 2);
        cr->fill();


        // events zone
        cr->set_source_rgba(color.r, color.g, color.b, playing ? 0.15 : 0.1);
        int rect_x = 0;
        int rect_y = c_sequence_padding * 2 + text_height * 2;
        int rect_w = width ;
        int rect_h = height - c_sequence_padding * 2 - text_height * 2;
        cr->set_line_width(1.0);
        cr->rectangle(rect_x , rect_y , rect_w, rect_h);
        cr->fill();

        // events (notes)
        if (playing) color = c_sequence_text_on;
        cr->set_source_rgba(color.r, color.g, color.b, playing ? 0.4 : 0.6);
        long tick_s;
        long tick_f;
        int note;
        bool selected;
        int velocity;
        draw_type dt;
        int length = seq->get_length( );
        int lowest_note = seq->get_lowest_note_event( );
        int highest_note = seq->get_highest_note_event( );
        double interval_height = highest_note - lowest_note;
        interval_height += 2;

        seq->reset_draw_list();
        while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note, &selected, &velocity )) != DRAW_FIN ) {

            int note_y = rect_h - (note + 1 - lowest_note) / interval_height * (rect_h - 4);
            int tick_s_x = tick_s * (rect_w - 3) / length + 2;
            int tick_f_x = tick_f * (rect_w - 3) / length + 2;

            if ( dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF )
                tick_f_x = tick_s_x + 1;
            if ( tick_f_x <= tick_s_x )
                tick_f_x = tick_s_x + 1;

            cr->move_to(rect_x + tick_s_x, rect_y + note_y - 1.5);
            cr->line_to(rect_x + tick_f_x, rect_y + note_y - 1.5);
        }

        cr->stroke();

        m_rect_x = rect_x;
        m_rect_y = rect_y;
        m_rect_w = rect_w;
        m_rect_h = rect_h;

    }

}

bool
SequenceButton::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        draw_background();
    }

    sequence * seq = get_sequence();
    if (seq != NULL) {

        // draw background
        cr->set_source(m_surface, 0.0, 0.0);
        cr->paint();

        // draw marker
        color color = seq->get_playing() ? c_sequence_marker_on : c_sequence_marker;
        cr->set_source_rgb(color.r, color.g, color.b);
        cr->set_line_width(1.0);
        cr->move_to(m_next_marker_pos - 0.5, m_rect_y + 1);
        cr->line_to(m_next_marker_pos - 0.5, m_rect_y + m_rect_h - 1);
        cr->stroke();

        m_last_marker_pos = m_next_marker_pos;
    }

    if (this == m_mainwindow->get_focus_sequence()) {
        cr->set_source_rgba(c_sequence_text.r, c_sequence_text.g, c_sequence_text.b, seq != NULL ? 0.5 : 0.25);
        cr->set_line_width(1.0);
        cr->rectangle(0, 0, width, height);
        cr->stroke();
    }

    return true;
}

void
SequenceButton::update()
{

    sequence * seq = get_sequence();
    if (seq != NULL) {
        long tick = seq->get_last_tick();
        m_next_marker_pos = tick * (m_rect_w - 4) / seq->get_length() + 3;

        if (seq->is_dirty_main()) {
            draw_background();
            queue_draw();
        } else {
            if (m_next_marker_pos > m_last_marker_pos) {
                queue_draw_area(m_last_marker_pos - 1, m_rect_y, m_next_marker_pos - m_last_marker_pos + 1, m_rect_h);
            } else {
                queue_draw_area(m_last_marker_pos - 1, m_rect_y, 1, m_rect_h);
                queue_draw_area(m_next_marker_pos - 1, m_rect_y, 1, m_rect_h);
            }
        }
    }

}

bool
SequenceButton::on_button_press_event(GdkEventButton* event)
{
    if (event->button == 1) m_drag_start = true;
    m_mainwindow->clear_focus();
    m_click = true;
    if (event->button == 2) m_middle_click = true;

    return false;
}

bool
SequenceButton::on_enter_notify_event(GdkEventCrossing* event)
{
    if (!m_drag_start && !m_click) m_mainwindow->set_drag_destination(this);
    if (!m_mainwindow->m_sequence_keyboard_nav) m_mainwindow->set_focus_sequence(this);
    return true;
}

bool
SequenceButton::on_leave_notify_event(GdkEventCrossing* event)
{
    if (m_click && m_drag_start) {
        m_mainwindow->set_drag_source(this);
        set_opacity(0.5);
        m_drag_start = false;
    }
    if (!m_mainwindow->m_sequence_keyboard_nav) m_mainwindow->set_focus_sequence(NULL);
    m_click = false;
    return true;
}

bool
SequenceButton::on_button_release_event(GdkEventButton* event)
{
    if (event->button == 1) {
        m_drag_start = false;
        set_opacity(1.0);
    }
    if (m_click) {

        m_click = false;

        sequence * seq = get_sequence();

        if (event->button == 1 && seq != NULL) {
            guint modifiers = gtk_accelerator_get_default_mod_mask ();
            if ((event->state & modifiers) == GDK_SHIFT_MASK) {
                if (seq->is_sync_reference()) {
                    m_perform->set_reference_sequence(-1);
                } else {
                    m_perform->set_reference_sequence(get_sequence_number());
                }
            } else if ((event->state & modifiers) == GDK_CONTROL_MASK) {
                seq->toggle_queued(m_perform->get_reference_sequence());
            } else {
                seq->toggle_playing();
            }
            queue_draw();
        }

        else if (event->button == 2 && m_middle_click) {
            m_middle_click = false;
            if (m_perform->is_active(get_sequence_number())) {
                m_mainwindow->open_edit_window(get_sequence_number(), seq);
            }
        }

        else if (event->button == 3) {
            Menu * menu = manage(new Menu());
            menu->attach_to_widget(*this);

            if (seq != NULL) {
                MenuItem * menu_item1 = manage(new MenuItem("Edit"));
                ((AccelLabel*)menu_item1->get_child())->set_accel(GDK_KEY_E, Gdk::CONTROL_MASK);
                menu_item1->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_EDIT, 0, 0));
                menu->append(*menu_item1);


                MenuItem * menu_item1b = manage(new MenuItem("Rename"));
                ((AccelLabel*)menu_item1b->get_child())->set_accel(GDK_KEY_R, Gdk::CONTROL_MASK);
                menu_item1b->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_RENAME, 0, 0));
                menu->append(*menu_item1b);
            } else {
                MenuItem * menu_item2 = manage(new MenuItem("New"));
                ((AccelLabel*)menu_item2->get_child())->set_accel(GDK_KEY_B, Gdk::CONTROL_MASK);
                menu_item2->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_NEW, 0, 0));
                menu->append(*menu_item2);
            }


            MenuItem * sep1 = manage(new SeparatorMenuItem());
            menu->append(*sep1);

            if (seq != NULL) {
                MenuItem * menu_item3 = manage(new MenuItem("Cut"));
                ((AccelLabel*)menu_item3->get_child())->set_accel(GDK_KEY_X, Gdk::CONTROL_MASK);
                menu_item3->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_CUT, 0, 0));
                menu->append(*menu_item3);

                MenuItem * menu_item4 = manage(new MenuItem("Copy"));
                ((AccelLabel*)menu_item4->get_child())->set_accel(GDK_KEY_C, Gdk::CONTROL_MASK);
                menu_item4->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_COPY, 0, 0));
                menu->append(*menu_item4);

                MenuItem * menu_item5 = manage(new MenuItem("Export sequence"));
                menu_item5->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_EXPORT, 0, 0));
                menu->append(*menu_item5);

                MenuItem * menu_item6 = manage(new MenuItem("Delete"));
                ((AccelLabel*)menu_item6->get_child())->set_accel(GDK_KEY_Delete, (Gdk::ModifierType)0);
                menu_item6->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_DELETE, 0, 0));
                menu->append(*menu_item6);
            } else {
                MenuItem * menu_item6 = manage(new MenuItem("Paste"));
                ((AccelLabel*)menu_item6->get_child())->set_accel(GDK_KEY_V, Gdk::CONTROL_MASK);
                menu_item6->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_PASTE, 0, 0));
                menu->append(*menu_item6);
            }

            if (seq != NULL) {
                MenuItem * sep2 = manage(new SeparatorMenuItem());
                menu->append(*sep2);

                MenuItem * menu_item7 = manage(new MenuItem("Midi Bus"));
                menu->append(*menu_item7);

                Menu *menu_buses = manage(new Menu());
                menu_item7->set_submenu(*menu_buses);

                char b[4];

                mastermidibus *masterbus = m_perform->get_master_midi_bus();
                for ( int i=0; i< masterbus->get_num_out_buses(); i++ ){
                    Menu *menu_channels = manage(new Menu());

                    MenuItem * menu_item_bus = manage(new MenuItem(masterbus->get_midi_out_bus_name(i)));
                    menu_item_bus->set_submenu(*menu_channels);
                    menu_buses->append(*menu_item_bus);


                    for( int j=0; j<16; j++ ){
                        snprintf(b, sizeof(b), "%d", j + 1);
                        std::string name = string(b);
                        int instrument = global_user_midi_bus_definitions[i].instrument[j];
                        if ( instrument >= 0 && instrument < c_max_instruments )
                        {
                            name = name + " " + global_user_instrument_definitions[instrument].instrument;
                        }

                        MenuItem * menu_item_channel = manage(new MenuItem(name));
                        menu_item_channel->signal_activate().connect(sigc::bind(mem_fun(*this, &SequenceButton::menu_callback), MENU_MIDI_BUS, i, j));
                        menu_channels->append(*menu_item_channel);
                    }

                }

            }

            menu->show_all();
            menu->popup_at_pointer(NULL);

        }
    }
    return true;
}



void
SequenceButton::menu_callback(context_menu_action action, int data1, int data2)
{
    switch (action) {
        case MENU_NEW:
            m_perform->new_sequence(get_sequence_number());
            // no break -> edit
        case MENU_EDIT:
            if (get_sequence() != NULL) m_mainwindow->open_edit_window(get_sequence_number(), get_sequence());
            break;
        case MENU_RENAME:
        {
            sequence * seq = get_sequence();
            if (seq != NULL) {
                Dialog dialog("Rename sequence");
                Entry entry;
                entry.set_text(seq->get_name());
                entry.set_editable(true);
                entry.set_activates_default(true);
                entry.grab_focus();
                entry.show();
                dialog.get_content_area()->pack_start(entry, true, true);
                dialog.add_button("_Ok", Gtk::RESPONSE_OK);
                dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
                dialog.set_default_response(Gtk::RESPONSE_OK);
                if (dialog.run() == Gtk::RESPONSE_OK)
                {
                    string s = entry.get_text();
                    seq->set_name(s);
                    if (m_mainwindow->m_editwindows[get_sequence_number()] != NULL) {
                        m_mainwindow->m_editwindows[get_sequence_number()]->update_name();
                    }

                }
            }
            break;
        }
        case MENU_CUT:
            m_perform->cut_sequence(get_sequence_number());
            break;
        case MENU_COPY:
            m_perform->copy_sequence(get_sequence_number());
            break;
        case MENU_EXPORT:
            m_mainwindow->menu_callback(MAIN_MENU_EXPORT_SEQUENCE, get_sequence_number(), -1);
            break;
        case MENU_DELETE:
            m_perform->delete_sequence(get_sequence_number());
            break;
        case MENU_PASTE:
            m_perform->paste_sequence(get_sequence_number());
            break;
        case MENU_MIDI_BUS:
            sequence * seq = get_sequence();
            if (seq != NULL) {
                seq->set_midi_bus(data1);
                seq->set_midi_channel(data2);
            }
            break;
    }
}
