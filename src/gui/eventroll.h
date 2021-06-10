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


#ifndef SEQ192_EVENTROLL
#define SEQ192_EVENTOLL

#include <gtkmm.h>

#include "../core/perform.h"
#include "pianoroll.h"

using namespace Gtk;


class EventRoll : public DrawingArea {

    public:

        EventRoll(perform * p, sequence * seq);
        ~EventRoll();

        sigc::signal<bool(GdkEventScroll*)> signal_scroll;
        sigc::signal<void(string name)> signal_focus;

        void set_data_type(unsigned char a_status, unsigned char a_control);


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
        int                 m_snap;
        bool                m_snap_active;
        void set_snap_active(bool a){m_snap_active = a;};

        /* what is the data window currently editing ? */
        unsigned char m_status;
        unsigned char m_cc;

        // edit mode (right click pressed)
        bool                m_adding;


        // when highlighting a bunch of events
        bool m_selecting;
        bool m_moving;
        bool m_moving_init;
        bool m_growing;
        bool m_painting;
        bool m_paste;
        bool m_is_drag_pasting;
        bool m_is_drag_pasting_start;
        bool m_justselected_one;
        int m_move_snap_offset_x;

        rect         m_selected;

        // where the dragging started
        int m_drop_x;
        int m_drop_y;
        int m_move_delta_x;
        int m_move_delta_y;
        int m_current_x;
        int m_current_y;

        void set_zoom(int zoom);
        void set_snap(int snap);

        void set_adding(bool adding);


        void convert_x( int a_x, long *a_ticks );
        void convert_t( long a_ticks, int *a_x );

        void snap_y( int *a_y );
        void snap_x( int *a_x );

        void x_to_w( int a_x1, int a_x2, int *a_x, int *a_w  );

        void drop_event(long a_tick);
        void start_paste();

        void update_width();

        bool on_motion_notify_event(GdkEventMotion* event);

        bool on_expose_event(GdkEventExpose* event);
        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event(GdkEventButton* event);
        bool on_scroll_event(GdkEventScroll* event);


    friend class EditWindow;
    friend class PianoRoll;

};

#endif
