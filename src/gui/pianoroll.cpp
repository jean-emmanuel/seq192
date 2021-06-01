#include "pianoroll.h"
#include "../core/globals.h"
#include "styles.h"

PianoRoll::PianoRoll(perform * p, sequence * seq, PianoKeys * pianokeys)
{
    m_perform = p;
    m_sequence = seq;
    m_pianokeys = pianokeys;

    m_zoom = 4;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &PianoRoll::on_draw));

    add_events(Gdk::POINTER_MOTION_MASK |
               Gdk::LEAVE_NOTIFY_MASK
    );


}

PianoRoll::~PianoRoll()
{
}


bool
PianoRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();


    // Horizontal lines

    cr->set_source_rgba(1.0, 1.0, 1.0, 0.1);
    cr->set_line_width(1.0);
    int i;
    for (i = 0; i < c_num_keys; i++) {
        cr->move_to(0, i * c_key_height - 0.5);
        cr->line_to(width, i * c_key_height - 0.5);
        cr->stroke();
    }

    // Vertical lines

    int measures_per_line = 1;

    int ticks_per_measure =  m_sequence->get_bpm() * (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_beat =  (4 * c_ppqn) / m_sequence->get_bw();
    int ticks_per_step = 6 * m_zoom;
    int ticks_per_m_line =  ticks_per_measure * measures_per_line;
    int start_tick = 0;
    int end_tick = width * m_zoom;

    for (int i=start_tick; i<end_tick; i += ticks_per_step)
    {
        int base_line = i / m_zoom;

        if ( i % ticks_per_m_line == 0 )
        {
            cr->set_source_rgba(1.0, 1.0, 1.0, 0.8);
        }
        else if (i % ticks_per_beat == 0 )
        {
            cr->set_source_rgba(1.0, 1.0, 1.0, 0.3);
        }
        else
        {
            cr->set_source_rgba(1.0, 1.0, 1.0, 0.1);
        }

        cr->move_to(base_line - 0.5, 0);
        cr->line_to(base_line - 0.5, height);
        cr->stroke();
    }



    // Events
    draw_type dt;
    long tick_s;
    long tick_f;
    int note;
    bool selected;
    int velocity;
    int note_x;
    int note_width;
    int note_y;
    int note_height = c_key_height - 3;

    m_sequence->reset_draw_marker();

    cr->set_source_rgba(c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue(), 0.75);

    while ((dt = m_sequence->get_next_note_event( &tick_s, &tick_f, &note, &selected, &velocity )) != DRAW_FIN)
    {

        note_x = tick_s / m_zoom;
        note_y = height - c_key_height * (note + 1) + 1;

        if (dt == DRAW_NORMAL_LINKED)
        {
            if (tick_f >= tick_s)
            {
                note_width = (tick_f - tick_s) / m_zoom;
                if ( note_width < 1 ) note_width = 1;
            }
            else
            {
                note_width = (m_sequence->get_length() - tick_s) / m_zoom;
            }
        }
        else
        {
            note_width = 16 / m_zoom;
        }

        cr->rectangle(note_x + 1, note_y, note_width - 2, note_height);

        if (tick_f < tick_s)
        {
            cr->rectangle(0, note_y, tick_f / m_zoom - 2, note_height);
        }

        cr->fill();
    }


    long tick = m_sequence->get_last_tick() / m_zoom;
    cr->set_line_width(1.0);
    cr->set_source_rgba(c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue(), 0.75);
    cr->move_to(tick - 0.5, 0);
    cr->line_to(tick - 0.5, height);
    cr->stroke();

    return true;
}

bool
PianoRoll::on_motion_notify_event(GdkEventMotion* event)
{

    m_pianokeys->on_motion_notify_event(event);

    return false;
}

bool
PianoRoll::on_leave_notify_event(GdkEventCrossing* event)
{
    m_pianokeys->on_leave_notify_event(event);
    return true;
}
