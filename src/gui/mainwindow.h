#ifndef SEQ24_MAINWINDOW
#define SEQ24_MAINWINDOW

#include <gtkmm.h>

#include "../core/globals.h"

#include "sequencebutton.h"


using namespace Glib;
using namespace Gtk;

class MainWindow : public Window {

    public:

        MainWindow(perform * p);
        ~MainWindow();

    private:

        perform * m_perform;


        VBox m_main_vbox;
        VBox m_toolbar_vbox;

        MenuBar m_main_menu;
        MenuItem m_main_menu_file;

        ScrolledWindow m_scroll_wrapper;
        Grid m_sequence_grid;
        SequenceButton * m_sequences[c_max_sequence];

        bool timer_callback();

};

#endif
