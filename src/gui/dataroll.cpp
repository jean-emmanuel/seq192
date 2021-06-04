#include "../core/globals.h"

#include "dataroll.h"
#include "styles.h"

DataRoll::DataRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    m_hscroll = 0;
    m_zoom = c_default_zoom;

    // draw callback
    signal_draw().connect(sigc::mem_fun(*this, &DataRoll::on_draw));

    add_events(Gdk::SCROLL_MASK);


}

DataRoll::~DataRoll()
{
}


bool
DataRoll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Gtk::Allocation allocation = get_allocation();
    // const int width = allocation.get_width();
    // const int height = allocation.get_height();

    return true;
}


void
DataRoll::set_zoom(int zoom)
{
    if (zoom < c_min_zoom) zoom = c_min_zoom;
    else if (zoom > c_max_zoom) zoom = c_max_zoom;
    m_zoom = zoom;
    queue_draw();
}

bool
DataRoll::on_scroll_event(GdkEventScroll* event)
{

    if (signal_scroll.emit(event)) return true;

    return false;
}
