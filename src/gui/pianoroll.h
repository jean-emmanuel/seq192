#ifndef SEQ24_PIANOROLL
#define SEQ24_PIANOROLL

#include <gtkmm.h>

#include "../core/perform.h"
#include "pianokeys.h"
#include "dataroll.h"

using namespace Gtk;

class rect
{
 public:
    int x, y, height, width;
};

class PianoRoll : public DrawingArea {

    public:

        PianoRoll(perform * p, sequence * seq, PianoKeys * pianokeys, DataRoll * dataroll);
        ~PianoRoll();

        sigc::signal<bool(GdkEventScroll*)> signal_scroll;

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;
        PianoKeys          *m_pianokeys;
        DataRoll           *m_dataroll;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // zoom: ticks per pixel
        int                 m_zoom;
        int                 m_note_length;
        int                 m_snap;


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

        int  get_zoom() {return m_zoom;};
        void set_zoom(int zoom);
        void set_snap(int snap);
        void set_note_length(int note_length);
        void set_adding(bool adding);
        void start_paste();

        // coords to notes & ticks
        void convert_xy( int a_x, int a_y, long *ticks, int *note);
        // notes & ticks to coords
        void convert_tn( long a_ticks, int a_note, int *x, int *y);

        // apply y snap
        void snap_y( int *y );
        // apply x snap
        void snap_x( int *x );

        void xy_to_rect( int x1,  int y1,
                         int x2,  int y2,
                         int *x,  int *y,
                         int *w,  int *h );

        void convert_tn_box_to_rect( long tick_s, long tick_f,
                                     int note_h, int note_l,
                                     int *x, int *y,
                                     int *w, int *h );


        bool on_motion_notify_event(GdkEventMotion* event);
        bool on_leave_notify_event(GdkEventCrossing* event);
        bool on_enter_notify_event(GdkEventCrossing* event);

        bool on_expose_event(GdkEventExpose* event);
        bool on_button_press_event(GdkEventButton* event);
        bool on_button_release_event(GdkEventButton* event);
        bool on_key_press_event(GdkEventKey* event);
        bool on_focus_in_event(GdkEventFocus*);
        bool on_focus_out_event(GdkEventFocus*);
        bool on_scroll_event(GdkEventScroll* event);


        friend class EditWindow;

};

#endif
