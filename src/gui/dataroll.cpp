#include "dataroll.h"
#include "../core/globals.h"

DataRoll::DataRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_zoom = 2;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &DataRoll::on_draw));

    update_width();

}

DataRoll::~DataRoll()
{
}


bool
DataRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_source_rgb(1.0, 0.0, 1.0);
    cr->set_line_width(1.0);
    cr->move_to(0,0);
    cr->line_to(width,height);
    cr->stroke();
    cr->move_to(0,height);
    cr->line_to(width,0);
    cr->stroke();

    return true;
}


void
DataRoll::update_width()
{
    int width = m_sequence->get_length() / m_zoom;
    set_size_request(width, 200);

}
