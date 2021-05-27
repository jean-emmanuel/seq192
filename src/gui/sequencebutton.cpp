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

void
SequenceButton::draw_background()
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    int seqnum = m_seqnum + m_perform->get_screenset() * c_seqs_in_set;
    if (m_perform->is_active(seqnum)) {
        auto seq = m_perform->get_sequence(seqnum);

        const int margin = 4;
        const int fontsize = 8;

        // background
        if (seq->get_playing()) {
            cr->set_source_rgb(0.0, 0.0, 0.0);
        } else {
            cr->set_source_rgb(1.0, 1.0, 1.0);
        }
        cr->rectangle(0, 0, width, height);
        cr->fill();

        // text
        if (seq->get_playing()) {
            cr->set_source_rgb(1.0, 1.0, 1.0);
        } else {
            cr->set_source_rgb(0.0, 0.0, 0.0);
        }
        Pango::FontDescription font;
        int text_width;
        int text_height;

        font.set_family("sans");
        font.set_size(fontsize * Pango::SCALE);
        font.set_weight(Pango::WEIGHT_NORMAL);

        auto name = create_pango_layout(seq->get_name());
        name->set_font_description(font);
        name->get_pixel_size(text_width, text_height);
        name->set_width((width - margin * 2) * Pango::SCALE);
        name->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->move_to(margin, margin);
        name->show_in_cairo_context(cr);


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
        timesig->set_width((width - margin * 2) * Pango::SCALE);
        timesig->set_ellipsize(Pango::ELLIPSIZE_END);
        cr->move_to(margin, height - margin - text_height);
        timesig->show_in_cairo_context(cr);



        // sequence preview
        if (seq->get_playing()) {
            cr->set_source_rgb(1.0, 1.0, 1.0);
        } else {
            cr->set_source_rgb(0.0, 0.0, 0.0);
        }
        int rect_x = margin;
        int rect_y = margin * 2 + text_height;
        int rect_w = width - margin * 2;
        int rect_h = height - margin * 4 - text_height * 2;
        cr->set_line_width(1.0);
        cr->rectangle(rect_x + 0.5, rect_y - 0.5, rect_w - 1, rect_h);
        cr->stroke();


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

        if (seq->get_playing()) {
            cr->set_source_rgb(1.0, 1.0, 1.0);
        } else {
            cr->set_source_rgb(0.0, 0.0, 0.0);
        }
        cr->stroke();

        m_rect_x = rect_x;
        m_rect_y = rect_y;
        m_rect_w = rect_w;
        m_rect_h = rect_h;

        m_clear = false;

    }

}

bool
SequenceButton::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    if (width != m_surface->get_width() || height != m_surface->get_height()){
        m_surface = ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        draw_background();
    }

    int seqnum = m_seqnum + m_perform->get_screenset() * c_seqs_in_set;
    if (m_perform->is_active(seqnum)) {

        auto seq = m_perform->get_sequence(seqnum);

        // draw background
        cr->set_source(m_surface, 0.0, 0.0);
        cr->paint();

        // draw marker
        long tick = seq->get_last_tick();
        int tick_x = tick * (m_rect_w - 4) / seq->get_length();
        cr->set_line_width(1.0);
        cr->set_source_rgb(0.0, 0.0, 1.0);
        cr->move_to(m_rect_x + tick_x + 2.5, m_rect_y + 1);
        cr->line_to(m_rect_x + tick_x + 2.5, m_rect_y + m_rect_h - 2);
        cr->stroke();

    }






    return true;
}
