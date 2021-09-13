// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SEQ192_PIANOROLL
#define SEQ192_PIANOROLL

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
        double              m_zoom;


        void set_zoom(double zoom);

        bool on_scroll_event(GdkEventScroll* event);

        friend class EditWindow;

};

#endif
