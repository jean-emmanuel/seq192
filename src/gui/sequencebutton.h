#ifndef SEQ24_SEQUENCEBUTTON
#define SEQ24_SEQUENCEBUTTON

#include <gtkmm.h>

#include "../core/globals.h"
#include "../core/perform.h"

#include "styles.h"

using namespace Cairo;
using namespace Gtk;

class SequenceButton : public DrawingArea {

    public:

        SequenceButton(perform * p, int seqnum);
        ~SequenceButton();

        bool get_clear() {return m_clear;};
        void set_clear() {m_clear = true;};

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

    private:

        perform * m_perform;
        int m_seqnum;
        bool m_clear;
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        void draw_background();
        sequence * get_sequence();

        int m_rect_x;
        int m_rect_y;
        int m_rect_w;
        int m_rect_h;


};

#endif
