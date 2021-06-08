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

        void set_data_type( unsigned char a_status, unsigned char a_control  );


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

        unsigned char m_status;
        unsigned char m_cc;

        int m_drop_x;
        int m_drop_y;
        int m_current_x;
        int m_current_y;
        bool m_dragging;
        bool m_drag_handle;

        void convert_x( int a_x, long *a_tick );
        void xy_to_rect( int a_x1,  int a_y1, int a_x2,  int a_y2, int *a_x,  int *a_y, int *a_w,  int *a_h );

        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event(GdkEventButton* event);
        bool on_motion_notify_event(GdkEventMotion* event);
        bool on_scroll_event(GdkEventScroll* event);


    friend class EditWindow;
    friend class PianoRoll;

};

#endif
