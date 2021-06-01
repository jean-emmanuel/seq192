#include "pianokeys.h"
#include "../core/globals.h"
#include "styles.h"

PianoKeys::PianoKeys(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_zoom = 2;
    m_min_note = 48;
    m_max_note = 72;
    m_hint_key = -1;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &PianoKeys::on_draw));

    add_events(Gdk::POINTER_MOTION_MASK |
               Gdk::LEAVE_NOTIFY_MASK
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
        cr->set_line_width(1.0);
        cr->move_to(0, key_y + c_key_height - 0.5);
        cr->line_to(width-1, key_y + c_key_height - 0.5);
        cr->stroke();

        octave = i  / 12 - 1;
        if (key % 12 == 0) {
            std::string key_name = (std::string) c_key_text[key % 12] + std::to_string(octave);
            auto name = create_pango_layout(key_name);
            name->set_font_description(font);
            name->get_pixel_size(text_width, text_height);
            cr->move_to(width - text_width - c_key_padding, key_y + c_key_height / 2 - text_height / 2);
            name->show_in_cairo_context(cr);
        }

    }

    cr->set_source_rgb(c_key_white.r, c_key_white.g, c_key_white.b);
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
PianoKeys::on_motion_notify_event(GdkEventMotion* event)
{

    Gtk::Allocation allocation = get_allocation();
    const int height = allocation.get_height();

    hint_key((height - event->y) / c_key_height);

    // int y, note;
    //
    // y = (int) a_p0->y + m_scroll_offset_y;
    // convert_y( y,&note );
    //
    // set_hint_key( note );
    //
    // if ( m_keying ){
    //
    //     if ( note != m_keying_note ){
    //
	//     m_seq->play_note_off( m_keying_note );
	//     m_seq->play_note_on(  note );
	//     m_keying_note = note;
    //
	// }
    // }

    return false;
}

bool
PianoKeys::on_leave_notify_event(GdkEventCrossing* event)
{
    hint_key(-1);
    return true;
}
