#ifndef SEQ24_PIANOKEYS
#define SEQ24_PIANOKEYS

#include <gtkmm.h>

#include "../core/perform.h"

using namespace Gtk;

class PianoKeys : public DrawingArea {

    public:

        PianoKeys(perform * p, sequence * seq);
        ~PianoKeys();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // zoom: ticks per pixel
        int                 m_zoom;

        int                 m_min_note;
        int                 m_max_note;
        int                 m_hint_key;

        void hint_key(int y);
        
        bool on_motion_notify_event(GdkEventMotion* event);
        bool on_leave_notify_event(GdkEventCrossing* event);

    friend class PianoRoll;
};

#endif
