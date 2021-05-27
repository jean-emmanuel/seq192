#include "sequencebutton.h"

SequenceButton::SequenceButton(perform * p, int seqnum)
{
    m_perform = p;
    m_seqnum = seqnum;
    m_clear = true;

    Gtk::Allocation allocation = get_allocation();
    m_surface = ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

    // draw callback
    this->signal_draw().connect(sigc::mem_fun(*this, &SequenceButton::on_draw));
}

SequenceButton::~SequenceButton()
{

}

int
SequenceButton::get_sequence_number() {
    return m_seqnum + m_perform->get_screenset() * c_seqs_in_set;
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

        // background
        color = seq->get_playing() ? c_sequence_background_on : c_sequence_background;
        cr->set_source_rgb(color.r, color.g, color.b);
        cr->rectangle(0, 0, width, height);
        cr->fill();

        // text
        color = seq->get_playing() ? c_sequence_text_on : c_sequence_text;
        cr->set_source_rgb(color.r, color.g, color.b);
        Pango::FontDescription font;
        int text_width;
        int text_height;

        font.set_family("sans");
        font.set_size(c_sequence_fontsize * Pango::SCALE);
        font.set_weight(Pango::WEIGHT_NORMAL);

        auto name = create_pango_layout(seq->get_name());
        name->set_font_description(font);
        name->get_pixel_size(text_width, text_height);
        name->set_width((width - c_sequence_padding * 2) * Pango::SCALE);
        name->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->move_to(c_sequence_padding, c_sequence_padding);
        name->show_in_cairo_context(cr);

        // timesig
        color = seq->get_playing() ? c_sequence_text_faded_on : c_sequence_text_faded;
        cr->set_source_rgb(color.r, color.g, color.b);
        char str[20];
        sprintf( str,
            "%d-%d %ld/%ld",
            seq->get_midi_bus()+1,
            seq->get_midi_channel()+1,
            seq->get_bpm(), seq->get_bw()
        );
        auto timesig = create_pango_layout(str);
        timesig->set_font_description(font);
        timesig->get_pixel_size(text_width, text_height);
        timesig->set_width((width - c_sequence_padding * 2) * Pango::SCALE);
        timesig->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->move_to(c_sequence_padding, c_sequence_padding + text_height);
        timesig->show_in_cairo_context(cr);



        // sequence preview
        color = seq->get_playing() ? c_sequence_events_background_on : c_sequence_events_background;
        cr->set_source_rgb(color.r, color.g, color.b);
        int rect_x = 0;
        int rect_y = c_sequence_padding * 2 + text_height * 2;
        int rect_w = width ;
        int rect_h = height - c_sequence_padding * 2 - text_height * 2;
        cr->set_line_width(1.0);
        cr->rectangle(rect_x , rect_y , rect_w, rect_h);
        cr->fill();
        // cr->stroke();

        color = seq->get_playing() ? c_sequence_events_on : c_sequence_events;
        cr->set_source_rgb(color.r, color.g, color.b);
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

        seq->reset_draw_marker();
        while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note, &selected, &velocity )) != DRAW_FIN ) {

            int note_y = rect_h - (note + 1 - lowest_note) / interval_height * (rect_h - 3);
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

        m_clear = false;
        m_lastseqnum = get_sequence_number();

    }

}

bool
SequenceButton::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    bool newsurface = false;

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        draw_background();
        newsurface = true;
    }

    sequence * seq = get_sequence();
    if (seq != NULL) {

        // sequence changed or screenset change
        if (!newsurface && (
            seq->is_dirty_main() ||
            m_lastseqnum != get_sequence_number()))
        {
            draw_background();
        }

        // draw background
        cr->set_source(m_surface, 0.0, 0.0);
        cr->paint();

        // draw marker
        long tick = seq->get_last_tick();
        int tick_x = tick * (m_rect_w - 4) / seq->get_length();
        color color = seq->get_playing() ? c_sequence_marker_on : c_sequence_marker;
        cr->set_source_rgb(color.r, color.g, color.b);
        cr->set_line_width(1.0);
        cr->move_to(m_rect_x + tick_x + 2.5, m_rect_y + 1);
        cr->line_to(m_rect_x + tick_x + 2.5, m_rect_y + m_rect_h - 1);
        cr->stroke();

    }

    return true;
}
