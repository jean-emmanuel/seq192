#ifndef SEQ24_EDITWINDOW
#define SEQ24_EDITWINDOW

#include <gtkmm.h>

#include "../core/globals.h"

#include "styles.h"
#include "mainwindow.h"
#include "sequencebutton.h"
#include "pianoroll.h"

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
        ScrolledWindow      m_scroll_wrapper;
        PianoRoll           m_pianoroll;



        bool on_delete_event(GdkEventAny *event);

};

#endif
