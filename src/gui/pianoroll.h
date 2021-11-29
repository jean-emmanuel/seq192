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


#ifndef SEQ192_TIMEROLL
#define SEQ192_TIMEROLL

#include <gtkmm.h>

#include "../core/perform.h"
#include "pianokeys.h"

using namespace Gtk;

class rect
{
 public:
    int x, y, height, width;
};

class coords
{
 public:
    int x1, y1, x2, y2;
};

class PianoRoll : public DrawingArea {

    public:

        PianoRoll(perform * p, sequence * seq, PianoKeys * pianokeys);
        ~PianoRoll();

        sigc::signal<bool(GdkEventScroll*)> signal_scroll;
        sigc::signal<void(string name)> signal_focus;

        void queue_draw_background();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;
        sequence           *m_bg_sequence;
        PianoKeys          *m_pianokeys;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;
        bool                m_draw_background_queued;
        void draw_background();

        // hscroll
        int                 m_hscroll;
        void set_hscroll(int s);

        // zoom: ticks per pixel
        double              m_zoom;
        int                 m_note_length;
        int                 m_snap;
        bool                m_snap_active;
        bool                m_snap_bypass;


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
        coords       m_selection;
        coords       m_edition;

        // where the dragging started
        int m_drop_x;
        int m_drop_y;
        int m_move_delta_x;
        int m_move_delta_y;
        int m_current_x;
        int m_current_y;
        int m_last_x;

        double get_zoom() {return m_zoom;};
        void set_zoom(double zoom);
        int  get_snap() {return m_snap;};
        void set_snap(int snap);
        void set_snap_active(bool a){m_snap_active = a;};
        void set_snap_bypass(bool bypass);
        int get_note_length() {return m_note_length;};
        void set_note_length(int note_length);
        void set_adding(bool adding);
        void start_paste();
        void update_selection();

        // coords to notes & ticks
        void convert_xy( int a_x, int a_y, long *ticks, int *note);
        // notes & ticks to coords
        void convert_tn( long a_ticks, int a_note, int *x, int *y);

        // apply y snap
        void snap_y(int *y);
        // apply x snap
        void snap_x(int *x, bool grow);

        void xy_to_rect(int x1,  int y1, int x2,  int y2, int *x,  int *y, int *w,  int *h );

        void convert_tn_box_to_rect(long tick_s, long tick_f, int note_h, int note_l, int *x, int *y, int *w, int *h );
        void convert_tn_box_to_coords(long tick_s, long tick_f, int note_h, int note_l, int *x1, int *y1,  int *x2, int *y2 );

        bool on_motion_notify_event(GdkEventMotion* event);
        bool on_leave_notify_event(GdkEventCrossing* event);
        bool on_enter_notify_event(GdkEventCrossing* event);

        bool on_expose_event(GdkEventExpose* event);
        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event(GdkEventButton* event);
        bool on_scroll_event(GdkEventScroll* event);


        friend class EditWindow;

};

#endif
