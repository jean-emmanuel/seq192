//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq24 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq24; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef SEQ24_OPTIONS
#define SEQ24_OPTIONS

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h> 
#include <gtkmm/fileselection.h>
#include <gtkmm/dialog.h>
#include <gtkmm/arrow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/notebook.h>
#include <gtkmm/tooltips.h>

#include <sigc++/bind.h>

#include "globals.h"
#include "perform.h"

using namespace Gtk;



class options : public Gtk::Dialog
{

 private:

    perform *m_perf;

    Button  *m_button_ok;
    Label* interaction_method_label;
    Label* interaction_method_desc_label;


    Table   *m_table;
    
    Notebook *m_notebook;

    enum button {
        e_jack_transport,
        e_jack_master,
        e_jack_master_cond,
        e_jack_start_mode_live,
        e_jack_start_mode_song,

        e_jack_connect,
        e_jack_disconnect
    };

    void clock_callback_off( int a_bus, RadioButton *a_button );
    void clock_callback_on ( int a_bus, RadioButton *a_button );
    void clock_callback_mod( int a_bus, RadioButton *a_button );

    void clock_mod_callback( Adjustment *adj );
    void interaction_method_callback( Adjustment *adj );
    
    void input_callback( int a_bus, Button *a_button );

    void transport_callback( button a_type, Button *a_button );
    
public:
    
    options( Gtk::Window &parent, perform *a_p );
};

#endif
