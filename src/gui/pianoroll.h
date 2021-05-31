#ifndef SEQ24_PIANOROLL
#define SEQ24_PIANOROLL

#include <gtkmm.h>

#include "../core/perform.h"

using namespace Gtk;

class PianoRoll : public DrawingArea {

    public:

        PianoRoll(perform * p, sequence * seq);
        ~PianoRoll();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // zoom: ticks per pixel
        int                 m_zoom;


        void update_width();





};

#endif
