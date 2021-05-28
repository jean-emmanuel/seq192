#ifndef SEQ24_MAINWINDOW
#define SEQ24_MAINWINDOW

#include <gtkmm.h>

#include "../core/globals.h"

#include "styles.h"
#include "sequencebutton.h"


using namespace Gtk;

enum main_menu_action
{
    MAIN_MENU_NEW = 0,
    MAIN_MENU_OPEN,
    MAIN_MENU_SAVE,
    MAIN_MENU_SAVEAS,
    MAIN_MENU_IMPORT,
    MAIN_MENU_EXPORT_SCREENSET,
    MAIN_MENU_EXPORT_SEQUENCE,
    MAIN_MENU_QUIT
};


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
        Menu m_main_submenu_file;
        MenuItem m_main_menu_file_new;
        MenuItem m_main_menu_file_open;
        MenuItem m_main_menu_file_save;
        MenuItem m_main_menu_file_saveas;
        MenuItem m_main_menu_file_import;
        MenuItem m_main_menu_file_export;
        MenuItem m_main_menu_file_quit;
        SeparatorMenuItem m_main_menu_separator1;
        SeparatorMenuItem m_main_menu_separator2;

        ScrolledWindow m_scroll_wrapper;
        Grid m_sequence_grid;
        SequenceButton * m_sequences[c_max_sequence];

        bool timer_callback();
        void menu_callback(main_menu_action action, int data1, int data2);

        void update_window_title();

        bool on_delete_event(GdkEventAny *event);

        bool unsaved_changes();
};

#endif
