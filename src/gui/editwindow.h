#ifndef SEQ24_EDITWINDOW
#define SEQ24_EDITWINDOW

#include <gtkmm.h>

#include "../core/globals.h"

#include "styles.h"
#include "mainwindow.h"
#include "sequencebutton.h"
#include "pianoroll.h"
#include "dataroll.h"
#include "pianokeys.h"

using namespace Gtk;

class MainWindow;
class EditWindow : public Window {

    public:

        EditWindow(perform * p, MainWindow * m, int seqnum, sequence * seq);
        ~EditWindow();

    private:

        perform            *m_perform;
        sequence           *m_sequence;
        MainWindow         *m_mainwindow;
        int                 m_seqnum;

        VBox                m_vbox;
        Grid                m_grid;
        HBox                m_hbox;
        VBox                m_left_vbox;
        VBox                m_right_vbox;
        ScrolledWindow      m_hscroll_wrapper;
        VBox                m_hscroll_vbox;
        PianoKeys           m_pianokeys;
        DataRoll            m_dataroll;
        ScrolledWindow      m_pianokeys_scroller;
        ScrolledWindow      m_pianoroll_scroller;
        PianoRoll           m_pianoroll;
        Scrollbar           m_vscrollbar;
        Label               m_dummy1;
        Label               m_dummy2;


        void update_size();
        bool on_delete_event(GdkEventAny *event);
        bool timer_callback();

        bool on_hscroll_event(GdkEventScroll* event);

};

#endif
