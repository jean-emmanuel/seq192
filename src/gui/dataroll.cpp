#include "../core/globals.h"

#include "dataroll.h"
#include "styles.h"

DataRoll::DataRoll(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

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
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    cr->set_source_rgb(1.0, 0.0, 1.0);
    cr->set_line_width(1.0);
    cr->move_to(0,0);
    cr->line_to(width / m_zoom ,height);
    cr->stroke();
    cr->move_to(0,height);
    cr->line_to(width / m_zoom,0);
    cr->stroke();

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
    // guint modifiers = gtk_accelerator_get_default_mod_mask ();
    //
    // if ((event->state & modifiers) == GDK_CONTROL_MASK)
    // {
    //     if (event->direction == GDK_SCROLL_DOWN)
    //     {
    //         if (m_zoom * 2 <= c_max_zoom) {
    //             set_zoom(m_zoom * 2);
    //         }
    //     }
    //     else if (event->direction == GDK_SCROLL_UP)
    //     {
    //         if (m_zoom / 2 >= c_min_zoom) {
    //             set_zoom(m_zoom / 2);
    //
    //         }
    //     }
    //     return true;
    // }

    return false;
}
