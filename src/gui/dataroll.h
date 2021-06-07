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


#ifndef SEQ192_DATAROLL
#define SEQ192_DATAROLL

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

        // hscroll
        int                 m_hscroll;
        void set_hscroll(int s){m_hscroll = s;};

        // zoom: ticks per pixel
        int                 m_zoom;
        void set_zoom(int zoom);


        void update_width();

        bool on_scroll_event(GdkEventScroll* event);


    friend class EditWindow;
    friend class PianoRoll;

};

#endif
