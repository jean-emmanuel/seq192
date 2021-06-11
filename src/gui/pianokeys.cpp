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


#include "pianokeys.h"
#include "../core/globals.h"
#include "styles.h"

PianoKeys::PianoKeys(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_hint_key = -1;
    m_keying = false;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &PianoKeys::on_draw));

    add_events(Gdk::BUTTON_PRESS_MASK |
        	   Gdk::BUTTON_RELEASE_MASK |
        	   Gdk::LEAVE_NOTIFY_MASK |
        	   Gdk::POINTER_MOTION_MASK
    );
}

PianoKeys::~PianoKeys()
{
}


bool
PianoKeys::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{

    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();




    double key_y;
    int i, key;
    int octave;
    Pango::FontDescription font;
    int text_width;
    int text_height;

    font.set_family(c_font);
    font.set_size(c_key_fontsize * Pango::SCALE);
    font.set_weight(Pango::WEIGHT_BOLD);

    for (i = 0; i < c_num_keys; i++) {
        key = i % 12;
        key_y = height - c_key_height * (i + 1);

        if ( i == m_hint_key )
        {
            cr->set_source_rgba(c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue(), 0.75);
        }
        else if (key == 1 ||
                 key == 3 ||
                 key == 6 ||
                 key == 8 ||
                 key == 10)
        {
            cr->set_source_rgb(c_key_black.r, c_key_black.g, c_key_black.b);
        }
        else
        {
            cr->set_source_rgb(c_key_white.r, c_key_white.g, c_key_white.b);
        }

        cr->rectangle(0, key_y, width, c_key_height);
        cr->fill();

        cr->set_source_rgb(c_key_black.r, c_key_black.g, c_key_black.b);

        if (i != 0)
        {
            cr->set_line_width(1.0);
            cr->move_to(0, key_y + c_key_height - 0.5);
            cr->line_to(width-1, key_y + c_key_height - 0.5);
            cr->stroke();
        }

        octave = i  / 12 - 1;
        if (key % 12 == 0) {
            std::string key_name = "C" + std::to_string(octave);
            auto name = create_pango_layout(key_name);
            name->set_font_description(font);
            name->get_pixel_size(text_width, text_height);
            cr->move_to(width - text_width - c_key_padding, key_y + c_key_height / 2 - text_height / 2);
            name->show_in_cairo_context(cr);
        }

    }

    cr->set_source_rgb(c_key_black.r, c_key_black.g, c_key_black.b);
    cr->move_to(width - 0.5, 0);
    cr->line_to(width - 0.5, height);
    cr->stroke();

    return true;
}

void
PianoKeys::hint_key(int key)
{
    if (key != m_hint_key) {
        m_hint_key = key;
        queue_draw();
    }
}

bool
PianoKeys::on_button_press_event(GdkEventButton *event)
{
	if (event->button == 1) {
        Gtk::Allocation allocation = get_allocation();
        const int height = allocation.get_height();
        int note = (height - event->y) / c_key_height;
	    m_keying = true;
        m_sequence->play_note_on(note);
	    m_keying_note = note;
	}
    return true;
}


bool
PianoKeys::on_motion_notify_event(GdkEventMotion* event)
{

    Gtk::Allocation allocation = get_allocation();
    const int height = allocation.get_height();

    int note = (height - event->y) / c_key_height;

    hint_key(note);

    if (m_keying && note != m_keying_note) {
	    m_sequence->play_note_off(m_keying_note);
	    m_sequence->play_note_on(note);
	    m_keying_note = note;
    }

    return false;
}

bool
PianoKeys::on_button_release_event(GdkEventButton *event)
{
	if (event->button == 1 && m_keying) {
	    m_keying = false;
        m_sequence->play_note_off(m_keying_note);
	}
    return true;
}

bool
PianoKeys::on_leave_notify_event(GdkEventCrossing* event)
{
    hint_key(-1);
    if (m_keying) {
        m_keying = false;
        m_sequence->play_note_off(m_keying_note);
    }
    return true;
}
