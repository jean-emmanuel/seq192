#ifndef SEQ24_PIANOROLL
#define SEQ24_PIANOROLL

#include <gtkmm.h>

#include "../core/perform.h"

using namespace Gtk;

class TimeRoll : public DrawingArea {

    public:

        TimeRoll(perform * p, sequence * seq);
        ~TimeRoll();

        sigc::signal<bool(GdkEventScroll*)> signal_scroll;

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // hscroll
        int                 m_hscroll;
        void set_hscroll(int s){m_hscroll = s;};

        // zoom: ticks per pixel
        int                 m_zoom;


        void set_zoom(int zoom);

        bool on_scroll_event(GdkEventScroll* event);

        friend class EditWindow;

};

#endif
