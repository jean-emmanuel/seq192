#ifndef SEQ24_SEQUENCEBUTTON
#define SEQ24_SEQUENCEBUTTON

#include <gtkmm.h>

#include "../core/globals.h"
#include "../core/perform.h"

#include "styles.h"
#include "mainwindow.h"

using namespace Gtk;

enum context_menu_action
{
    MENU_NEW = 0,
    MENU_EDIT,
    MENU_CUT,
    MENU_COPY,
    MENU_EXPORT,
    MENU_PASTE,
    MENU_MIDI_BUS
};

class MainWindow;
class SequenceButton : public DrawingArea {

    public:

        SequenceButton(perform * p, MainWindow * m, int seqnum);
        ~SequenceButton();

        bool get_clear() {return m_clear;};
        void set_clear() {m_clear = true;};

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
        bool on_button_release_event(GdkEventButton* event);
        bool on_button_press_event(GdkEventButton* event);
        bool on_leave_notify_event(GdkEventCrossing* event);
        bool on_enter_notify_event(GdkEventCrossing* event);

    private:

        perform * m_perform;
        MainWindow * m_mainwindow;
        int m_seqnum;
        int m_last_seqnum;
        bool m_clear;
        bool m_click;
        bool m_drag_start;
        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        void draw_background();
        int get_sequence_number();
        int get_last_sequence_number();
        sequence * get_sequence();
        void menu_callback(context_menu_action action, int data1, int data2);

        int m_rect_x;
        int m_rect_y;
        int m_rect_w;
        int m_rect_h;

        friend class MainWindow;

};

#endif
