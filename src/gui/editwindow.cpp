#include "editwindow.h"
#include "../core/globals.h"

EditWindow::EditWindow(perform * p, sequence * seq)
{
    m_perform = p;
    m_sequence = seq;

    show_all();
}

EditWindow::~EditWindow()
{
}
