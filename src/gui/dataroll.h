#ifndef SEQ24_DATAROLL
#define SEQ24_DATAROLL

#include <gtkmm.h>

#include "../core/perform.h"

using namespace Gtk;

class DataRoll : public DrawingArea {

    public:

        DataRoll(perform * p, sequence * seq);
        ~DataRoll();

        sigc::signal<bool(GdkEventScroll*)> signal_scroll;
        

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // zoom: ticks per pixel
        int                 m_zoom;
        void set_zoom(int zoom);


        void update_width();

        bool on_scroll_event(GdkEventScroll* event);


    friend class EditWindow;
    friend class PianoRoll;

};

#endif
