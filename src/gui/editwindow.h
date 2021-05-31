#ifndef SEQ24_EDITWINDOW
#define SEQ24_EDITWINDOW

#include <gtkmm.h>

#include "../core/globals.h"

#include "styles.h"
#include "sequencebutton.h"

using namespace Gtk;

class EditWindow : public Window {

    public:

        EditWindow(perform * p, sequence * seq);
        ~EditWindow();

    private:

        perform            *m_perform;
        sequence           *m_sequence;



};

#endif
