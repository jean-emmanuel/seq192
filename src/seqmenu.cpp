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

#include "seqmenu.h"
#include "seqedit.h"
#include "font.h"


// Constructor

seqmenu::seqmenu( perform *a_p  )
{    
    using namespace Menu_Helpers;

    m_mainperf = a_p;
    m_menu = NULL;

    // init the clipboard, so that we don't get a crash
    // on paste with no previous copy...
    m_clipboard.set_master_midi_bus( a_p->get_master_midi_bus() );
}


    void
seqmenu::popup_menu( void )
{

    using namespace Menu_Helpers;

    if ( m_menu != NULL )
        delete m_menu;

    m_menu = manage( new Menu());

    if ( m_mainperf->is_active( m_current_seq )) {
        m_menu->items().push_back(MenuElem("Edit...",
                    mem_fun(*this, &seqmenu::seq_edit)));
    } else {
        m_menu->items().push_back(MenuElem("New",
                    mem_fun(*this, &seqmenu::seq_edit)));
    }



    m_menu->items().push_back(SeparatorElem());

    if ( m_mainperf->is_active( m_current_seq )) {
        m_menu->items().push_back(MenuElem("Cut", mem_fun(*this,&seqmenu::seq_cut)));
        m_menu->items().push_back(MenuElem("Copy", mem_fun(*this,&seqmenu::seq_copy)));
    } else {
        m_menu->items().push_back(MenuElem("Paste", mem_fun(*this,&seqmenu::seq_paste)));
    }

    m_menu->items().push_back(SeparatorElem());
    
    Menu *menu_song = manage( new Menu() );
    m_menu->items().push_back( MenuElem( "Song", *menu_song) );
    
    if ( m_mainperf->is_active( m_current_seq ))
    {
        menu_song->items().push_back(MenuElem("Clear Song Data", mem_fun(*this,&seqmenu::seq_clear_perf)));
    }
    
    menu_song->items().push_back(MenuElem("Mute All Tracks", mem_fun(*this,&seqmenu::mute_all_tracks)));
    
    if ( m_mainperf->is_active( m_current_seq )) {
        m_menu->items().push_back(SeparatorElem());
        Menu *menu_buses = manage( new Menu() );

        m_menu->items().push_back( MenuElem( "Midi Bus", *menu_buses) );

        /* midi buses */
        mastermidibus *masterbus = m_mainperf->get_master_midi_bus();
        for ( int i=0; i< masterbus->get_num_out_buses(); i++ ){

            Menu *menu_channels = manage( new Menu() );

            menu_buses->items().push_back(MenuElem( masterbus->get_midi_out_bus_name(i),
                        *menu_channels ));
            char b[4];

            /* midi channel menu */
            for( int j=0; j<16; j++ ){
                snprintf(b, sizeof(b), "%d", j + 1);
                std::string name = string(b);
                int instrument = global_user_midi_bus_definitions[i].instrument[j]; 
                if ( instrument >= 0 && instrument < c_maxBuses )
                {
                    name = name + (string(" (") + 
                            global_user_instrument_definitions[instrument].instrument + 
                            string(")") );
                }

                menu_channels->items().push_back(MenuElem(name, 
                            sigc::bind(mem_fun(*this,&seqmenu::set_bus_and_midi_channel), 
                                i, j )));
            }
        }        
    }

    m_menu->popup(0,0);

}

    void
seqmenu::set_bus_and_midi_channel( int a_bus, int a_ch )
{
    if ( m_mainperf->is_active( m_current_seq )) {
        m_mainperf->get_sequence( m_current_seq )->set_midi_bus( a_bus );
        m_mainperf->get_sequence( m_current_seq )->set_midi_channel( a_ch );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();
    }
}

void
seqmenu::mute_all_tracks( void )
{
    m_mainperf->mute_all_tracks();
}


// Menu callback, Lanches Editor Window
void 
seqmenu::seq_edit(){

    seqedit *seq_edit;

    if ( m_mainperf->is_active( m_current_seq )) {
        if ( !m_mainperf->get_sequence( m_current_seq )->get_editing())
        {
            seq_edit = new seqedit( m_mainperf->get_sequence( m_current_seq ), 
                    m_mainperf, 
                    m_current_seq
                    );
        }
        else {
            m_mainperf->get_sequence( m_current_seq )->set_raise(true);
        }
    }
    else {
        this->seq_new();
        seq_edit = new seqedit( m_mainperf->get_sequence( m_current_seq ), 
                m_mainperf, 
                m_current_seq
                );
    }    
}

// Makes a New sequence 
void 
seqmenu::seq_new(){

    if ( ! m_mainperf->is_active( m_current_seq )){

        m_mainperf->new_sequence( m_current_seq );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();

    }
}

// Copies selected to clipboard sequence */
void 
seqmenu::seq_copy(){

    if ( m_mainperf->is_active( m_current_seq ))
        m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));
}

// Deletes and Copies to Clipboard */
void 
seqmenu::seq_cut(){

    if ( m_mainperf->is_active( m_current_seq ) &&
            !m_mainperf->is_sequence_in_edit( m_current_seq ) ){

        m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));
        m_mainperf->delete_sequence( m_current_seq );
        redraw( m_current_seq );

    }
}

// Puts clipboard into location
void 
seqmenu::seq_paste(){

    if ( ! m_mainperf->is_active( m_current_seq )){

        m_mainperf->new_sequence( m_current_seq  );
        *(m_mainperf->get_sequence( m_current_seq )) = m_clipboard;

        m_mainperf->get_sequence( m_current_seq )->set_dirty();

    }
}


void 
seqmenu::seq_clear_perf(){

    if ( m_mainperf->is_active( m_current_seq )){

        m_mainperf->push_trigger_undo();
        
        m_mainperf->clear_sequence_triggers( m_current_seq  );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();

    }
}



