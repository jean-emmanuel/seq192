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

#include "seqedit.h" 
#include "sequence.h"
#include "midibus.h"
#include "controllers.h"
#include "event.h"
#include "options.h"


#include "play.xpm"
#include "q_rec.xpm"
#include "rec.xpm"
#include "thru.xpm"
#include "bus.xpm"
#include "midi.xpm"
#include "snap.xpm"
#include "zoom.xpm"
#include "length.xpm"
#include "scale.xpm"
#include "key.xpm"   
#include "down.xpm"
#include "note_length.xpm"
#include "undo.xpm"
#include "redo.xpm"
#include "quanize.xpm"
#include "menu_empty.xpm"
#include "menu_full.xpm"
#include "sequences.xpm"
#include "tools.xpm"
#include "seq-editor.xpm"

// tooltip helper, for old vs new gtk...
#if GTK_MINOR_VERSION >= 12
#   define add_tooltip( obj, text ) obj->set_tooltip_text( text);
#else
#   define add_tooltip( obj, text ) m_tooltips->set_tip( *obj, text );
#endif

int seqedit::m_initial_zoom = 2;
int seqedit::m_initial_snap = c_ppqn / 4;
int seqedit::m_initial_note_length = c_ppqn / 4;
int seqedit::m_initial_scale = 0;
int seqedit::m_initial_key = 0;
int seqedit::m_initial_sequence = -1;


// Actions
const int select_all_notes      = 1;
const int select_all_events     = 2;
const int select_inverse_notes  = 3;
const int select_inverse_events = 4;

const int quantize_notes    = 5;
const int quantize_events   = 6;

const int tighten_events   =  8;
const int tighten_notes    =  9;

const int transpose     = 10;
const int transpose_h   = 12;

/* connects to a menu item, tells the performance 
   to launch the timer thread */
void 
seqedit::menu_action_quantise( void )
{
}



seqedit::seqedit( sequence *a_seq, 
		  perform *a_perf,  
		  // mainwid *a_mainwid, 
		  int a_pos  )
{
    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq_editor_xpm));

    /* set the performance */
    m_seq = a_seq;

    m_zoom        =  m_initial_zoom;
    m_snap        =  m_initial_snap;
    m_note_length = m_initial_note_length;
    m_scale       = m_initial_scale;
    m_key         =   m_initial_key;
    m_sequence    = m_initial_sequence;

    m_mainperf = a_perf;
    // m_mainwid = a_mainwid;
    m_pos = a_pos;

    /* main window */
    std::string title = "seq24 - ";
    title.append(m_seq->get_name());
    set_title(title);
    set_size_request(700, 500);

    m_seq->set_editing( true );

    /* scroll bars */
    m_vadjust = manage( new Adjustment(55,0, c_num_keys,           1,1,1 ));
    m_hadjust = manage( new Adjustment(0, 0, 1,  1,1,1 ));    
    m_vscroll_new   =  manage(new VScrollbar( *m_vadjust ));
    m_hscroll_new   =  manage(new HScrollbar( *m_hadjust ));

    /* get some new objects */
    m_seqkeys_wid  = manage( new seqkeys(  m_seq,
                                           m_vadjust ));

    m_seqtime_wid  = manage( new seqtime(  m_seq,
                                           m_zoom,
                                           m_hadjust ));
    
    m_seqdata_wid  = manage( new seqdata(  m_seq,
                                           m_zoom,
                                           m_hadjust));
    
    m_seqevent_wid = manage( new seqevent( m_seq,
                                           m_zoom,
                                           m_snap,
                                           m_seqdata_wid,
                                           m_hadjust));
    
    m_seqroll_wid  = manage( new seqroll(  m_mainperf,
                                           m_seq,
                                           m_zoom,
                                           m_snap,
                                           m_seqdata_wid,
                                           m_seqevent_wid,
                                           m_seqkeys_wid,
                                           m_pos,
                                           m_hadjust,
                                           m_vadjust ));
    

    
    /* menus */
    m_menubar   =  manage( new MenuBar());
    m_menu_tools = manage( new Menu() );
    m_menu_zoom =  manage( new Menu());
    m_menu_snap =   manage( new Menu());
    m_menu_note_length = manage( new Menu());
    m_menu_length = manage( new Menu());
    m_menu_bpm = manage( new Menu() );
    m_menu_bw = manage( new Menu() );
    m_menu_rec_vol = manage( new Menu() );

    m_menu_midich = NULL;
    m_menu_midibus = NULL;
    m_menu_sequences = NULL;

    m_menu_key = manage( new Menu());
    m_menu_scale = manage( new Menu());
    m_menu_data = NULL;

    create_menus(); 

    /* tooltips */
    m_tooltips = manage( new Tooltips( ) );

 
    /* init table, viewports and scroll bars */
    m_table     = manage( new Table( 7, 4, false));
    m_vbox      = manage( new VBox( false, 2 ));
    m_hbox      = manage( new HBox( false, 2 ));
    m_hbox2     = manage( new HBox( false, 2 ));
    m_hbox3     = manage( new HBox( false, 2 ));
    HBox *dhbox = manage( new HBox( false, 2 ));

    m_vbox->set_border_width( 2 );

    /* fill table */
    m_table->attach( *m_seqkeys_wid,    0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL );

    m_table->attach( *m_seqtime_wid, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK );
    m_table->attach( *m_seqroll_wid , 1, 2, 1, 2,
    	      Gtk::FILL |  Gtk::SHRINK,  
    	      Gtk::FILL |  Gtk::SHRINK );

    m_table->attach( *m_seqevent_wid, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK );
    m_table->attach( *m_seqdata_wid, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK );
    m_table->attach( *dhbox, 1, 2, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK, 0, 2 );

    m_table->attach( *m_vscroll_new, 2, 3, 1, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND  );
    m_table->attach( *m_hscroll_new, 1, 2, 5, 6, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK  );

    /* no expand, just fit the widgets */
    /* m_vbox->pack_start(*m_menubar, false, false, 0); */
    m_vbox->pack_start(*m_hbox,  false, false, 0);
    m_vbox->pack_start(*m_hbox2, false, false, 0);
    m_vbox->pack_start(*m_hbox3, false, false, 0);

    /* exapand, cause rollview expands */
    m_vbox->pack_start(*m_table, true, true, 0);




    /* data button */
    m_button_data = manage( new Button( " Event " ));
    m_button_data->signal_clicked().connect(
            mem_fun( *this, &seqedit::popup_event_menu));

    m_entry_data = manage( new Entry( ));
    m_entry_data->set_size_request(40,-1);
    m_entry_data->set_editable( false );

    dhbox->pack_start( *m_button_data, false, false );
    dhbox->pack_start( *m_entry_data, true, true );

    /* play, rec, thru */
    m_toggle_play = manage( new ToggleButton() );
    m_toggle_play->add(  *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( play_xpm ))));
    m_toggle_play->signal_clicked().connect(
            mem_fun( *this, &seqedit::play_change_callback));
    add_tooltip( m_toggle_play, "Sequence dumps data to midi bus." );
    
    m_toggle_record = manage( new ToggleButton(  ));
    m_toggle_record->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( rec_xpm ))));
    m_toggle_record->signal_clicked().connect(
            mem_fun( *this, &seqedit::record_change_callback));
    add_tooltip( m_toggle_record, "Records incoming midi data." );

    m_toggle_q_rec = manage( new ToggleButton(  ));
    m_toggle_q_rec->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( q_rec_xpm ))));
    m_toggle_q_rec->signal_clicked().connect(
            mem_fun( *this, &seqedit::q_rec_change_callback));
    add_tooltip( m_toggle_q_rec, "Quantized Record." );

    m_button_rec_vol = manage( new Button());
    m_button_rec_vol->add( *manage( new Label("Vol")));
    m_button_rec_vol->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu), m_menu_rec_vol  ));
    add_tooltip( m_button_rec_vol, "Select recording volume" );

    m_toggle_thru = manage( new ToggleButton(  ));
    m_toggle_thru->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( thru_xpm ))));
    m_toggle_thru->signal_clicked().connect(
            mem_fun( *this, &seqedit::thru_change_callback));
    add_tooltip( m_toggle_thru, "Incoming midi data passes "
            "thru to sequences midi bus and channel." );

    m_toggle_play->set_active( m_seq->get_playing());
    m_toggle_record->set_active( m_seq->get_recording());
    m_toggle_thru->set_active( m_seq->get_thru());
 
    dhbox->pack_end( *m_button_rec_vol, false, false, 4);
    dhbox->pack_end( *m_toggle_q_rec, false, false, 4);
    dhbox->pack_end( *m_toggle_record, false, false, 4);
    dhbox->pack_end( *m_toggle_thru, false, false, 4);
    dhbox->pack_end( *m_toggle_play, false, false, 4);
    dhbox->pack_end( *(manage(new VSeparator( ))), false, false, 4);

    fill_top_bar();


    /* add table */
    this->add( *m_vbox );
    /* show everything */
    show_all();

    /* sets scroll bar to the middle */
    //gfloat middle = m_vscroll->get_adjustment()->get_upper() / 3;
    //m_vscroll->get_adjustment()->set_value(middle);
    m_seqroll_wid->set_ignore_redraw(true);

    set_zoom( m_zoom );
    set_snap( m_snap );
    set_note_length( m_note_length );
    set_background_sequence( m_sequence );


    set_bpm( m_seq->get_bpm() );
    set_bw( m_seq->get_bw() );
    set_measures( get_measures() );

    set_midi_channel( m_seq->get_midi_channel() );
    set_midi_bus( m_seq->get_midi_bus() );
    set_data_type( EVENT_NOTE_ON );

    set_scale( m_scale );
    set_key( m_key );

    m_seqroll_wid->set_ignore_redraw(false);
    add_events(Gdk::SCROLL_MASK);
}


void 
seqedit::create_menus( void )
{
    using namespace Menu_Helpers;

    char b[20];
    
    /* zoom */
    for (int i = c_min_zoom; i <= c_max_zoom; i*=2)
    {
        snprintf(b, sizeof(b), "1:%d", i);
        m_menu_zoom->items().push_back(MenuElem(b,
                    sigc::bind(mem_fun(*this, &seqedit::set_zoom), i )));
    }
      
    /* note snap */
    m_menu_snap->items().push_back(MenuElem("1",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 4  )));
    m_menu_snap->items().push_back(MenuElem("1/2",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 2  )));
    m_menu_snap->items().push_back(MenuElem("1/4",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 1  )));
    m_menu_snap->items().push_back(MenuElem("1/8",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 2  )));
    m_menu_snap->items().push_back(MenuElem("1/16",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 4  )));
    m_menu_snap->items().push_back(MenuElem("1/32",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 8  )));
    m_menu_snap->items().push_back(MenuElem("1/64",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 16 )));
    m_menu_snap->items().push_back(MenuElem("1/128",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 32 )));

    m_menu_snap->items().push_back(SeparatorElem());
    
    m_menu_snap->items().push_back(MenuElem("1/3",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 4  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/6",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 2  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/12",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn * 1  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/24",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 2  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/48",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 4  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/96",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 8  / 3 )));
    m_menu_snap->items().push_back(MenuElem("1/192",
                sigc::bind(mem_fun(*this, &seqedit::set_snap),
                    c_ppqn / 16 / 3 )));
    
    /* note note_length */
    m_menu_note_length->items().push_back(MenuElem("1",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 4  )));
    m_menu_note_length->items().push_back(MenuElem("1/2",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 2  )));
    m_menu_note_length->items().push_back(MenuElem("1/4",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 1  )));
    m_menu_note_length->items().push_back(MenuElem("1/8",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 2  )));
    m_menu_note_length->items().push_back(MenuElem("1/16",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 4  )));
    m_menu_note_length->items().push_back(MenuElem("1/32",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 8  )));
    m_menu_note_length->items().push_back(MenuElem("1/64",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 16 )));
    m_menu_note_length->items().push_back(MenuElem("1/128",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 32 )));
    m_menu_note_length->items().push_back(SeparatorElem());
    m_menu_note_length->items().push_back(MenuElem("1/3",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 4  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/6",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 2  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/12",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn * 1  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/24",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 2  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/48",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 4  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/96",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 8  / 3 )));
    m_menu_note_length->items().push_back(MenuElem("1/192",
                sigc::bind(mem_fun(*this, &seqedit::set_note_length),
                    c_ppqn / 16 / 3 )));
    
    /* Key */
    m_menu_key->items().push_back(MenuElem( c_key_text[0],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 0 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[1],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 1 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[2],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 2 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[3],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 3 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[4],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 4 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[5],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 5 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[6],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 6 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[7],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 7 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[8],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 8 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[9],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 9 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[10],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 10 )));
    m_menu_key->items().push_back(MenuElem( c_key_text[11],
                sigc::bind(mem_fun(*this, &seqedit::set_key), 11 )));
    
    /* bw */
    m_menu_bw->items().push_back(MenuElem("1",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 1  )));
    m_menu_bw->items().push_back(MenuElem("2",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 2  )));
    m_menu_bw->items().push_back(MenuElem("4",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 4  )));
    m_menu_bw->items().push_back(MenuElem("8",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 8  )));
    m_menu_bw->items().push_back(MenuElem("16",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 16 )));
    m_menu_bw->items().push_back(MenuElem("32",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 32 )));
    m_menu_bw->items().push_back(MenuElem("64",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 64 )));
    m_menu_bw->items().push_back(MenuElem("128",
                sigc::bind(mem_fun(*this, &seqedit::set_bw), 128 )));
  
    /* record volume */
    m_menu_rec_vol->items().push_back(MenuElem("Free",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 0)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 8",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 127)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 7",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 111)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 6",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 95)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 5",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 79)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 4",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 63)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 3",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 47)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 2",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 31)));
    m_menu_rec_vol->items().push_back(MenuElem("Fixed 1",
                sigc::bind(mem_fun(*this, &seqedit::set_rec_vol), 15)));
    
    /* music scale */
    m_menu_scale->items().push_back(MenuElem(c_scales_text[0],
                sigc::bind(mem_fun(*this, &seqedit::set_scale),
                    c_scale_off )));
    m_menu_scale->items().push_back(MenuElem(c_scales_text[1],
                sigc::bind(mem_fun(*this, &seqedit::set_scale),
                    c_scale_major )));
    m_menu_scale->items().push_back(MenuElem(c_scales_text[2],
                sigc::bind(mem_fun(*this, &seqedit::set_scale),
                    c_scale_minor )));
    
    /* midi channel menu */
    for( int i=0; i<16; i++ ){
        
        snprintf(b, sizeof(b), "%d", i + 1);
        
        /* length */
        m_menu_length->items().push_back(MenuElem(b,
                    sigc::bind(mem_fun(*this, &seqedit::set_measures), i+1 )));
        /* length */
        m_menu_bpm->items().push_back(MenuElem(b, 
                    sigc::bind(mem_fun(*this, &seqedit::set_bpm), i+1 )));
    }

    m_menu_length->items().push_back(MenuElem("32",
                sigc::bind(mem_fun(*this, &seqedit::set_measures), 32 )));
    m_menu_length->items().push_back(MenuElem("64",
                sigc::bind(mem_fun(*this, &seqedit::set_measures), 64 )));

  //m_menu_tools->items().push_back( SeparatorElem( )); 
}


void
seqedit::popup_tool_menu( void )
{

    using namespace Menu_Helpers;

    m_menu_tools = manage( new Menu());

    /* tools */
    Menu *holder;
    Menu *holder2;
    
    holder = manage( new Menu());
    holder->items().push_back( MenuElem( "All Notes",
                sigc::bind(mem_fun(*this, &seqedit::do_action),
                    select_all_notes, 0)));

    holder->items().push_back( MenuElem( "Inverse Notes",
                sigc::bind(mem_fun(*this, &seqedit::do_action),
                    select_inverse_notes, 0)));
   
    if ( m_editing_status !=  EVENT_NOTE_ON &&
         m_editing_status !=  EVENT_NOTE_OFF ){

        holder->items().push_back( SeparatorElem( )); 
        holder->items().push_back( MenuElem( "All Events",
                    sigc::bind(mem_fun(*this, &seqedit::do_action),
                        select_all_events, 0)));

        holder->items().push_back( MenuElem( "Inverse Events",
                    sigc::bind(mem_fun(*this, &seqedit::do_action),
                        select_inverse_events, 0)));
    }
    
    m_menu_tools->items().push_back( MenuElem( "Select", *holder ));
    
    holder = manage( new Menu());
    holder->items().push_back( MenuElem( "Quantize Selected Notes",
                sigc::bind(mem_fun(*this, &seqedit::do_action),
                    quantize_notes, 0 )));

    holder->items().push_back( MenuElem( "Tighten Selected Notes",
                sigc::bind(mem_fun(*this, &seqedit::do_action),
                    tighten_notes,0 )));

    if ( m_editing_status !=  EVENT_NOTE_ON &&
         m_editing_status !=  EVENT_NOTE_OFF ){
        
        holder->items().push_back( SeparatorElem( )); 
        holder->items().push_back( MenuElem( "Quantize Selected Events",
                    sigc::bind(mem_fun(*this, &seqedit::do_action),
                        quantize_events,0 )));

        holder->items().push_back( MenuElem( "Tighten Selected Events",
                    sigc::bind(mem_fun(*this, &seqedit::do_action),
                        tighten_events,0 )));
        
    }
    m_menu_tools->items().push_back( MenuElem( "Modify Time", *holder ));


    holder = manage( new Menu());

    char num[11];

    holder2 = manage( new Menu());
    for ( int i=-12; i<=12; ++i) {

        if (i != 0){
            snprintf(num, sizeof(num), "%+d [%s]", i, c_interval_text[abs(i)]);
            holder2->items().push_front( MenuElem( num,
                        sigc::bind(mem_fun(*this,&seqedit::do_action),
                            transpose, i )));
        }
    }

    holder->items().push_back( MenuElem( "Transpose Selected", *holder2));

    holder2 = manage( new Menu());
    for ( int i=-7; i<=7; ++i) {

        if (i != 0){
            snprintf(num, sizeof(num), "%+d [%s]", (i<0) ? i-1 : i+1, c_chord_text[abs(i)]);
            holder2->items().push_front( MenuElem( num,
                        sigc::bind(mem_fun(*this,&seqedit::do_action),
                            transpose_h, i )));
        }
    }

    if ( m_scale != 0 ){
        holder->items().push_back( MenuElem(
                    "Harmonic Transpose Selected", *holder2));
    }

    m_menu_tools->items().push_back( MenuElem( "Modify Pitch", *holder ));
    m_menu_tools->popup(0,0);
}


void
seqedit::do_action( int a_action, int a_var )
{
    switch (a_action) {

        case select_all_notes:
            m_seq->select_events(EVENT_NOTE_ON, 0);
            m_seq->select_events(EVENT_NOTE_OFF, 0);
            break;

        case select_all_events:
            m_seq->select_events(m_editing_status, m_editing_cc);
            break;

        case select_inverse_notes:
            m_seq->select_events(EVENT_NOTE_ON, 0, true);
            m_seq->select_events(EVENT_NOTE_OFF, 0, true);
            break;

        case select_inverse_events:
            m_seq->select_events(m_editing_status, m_editing_cc, true);
            break;
            // !!! m_seq->push_undo();

        case quantize_notes:
            m_seq->push_undo();
            m_seq->quanize_events(EVENT_NOTE_ON, 0, m_snap, 1 , true);
            break;

        case quantize_events:
            m_seq->push_undo();
            m_seq->quanize_events(m_editing_status, m_editing_cc, m_snap, 1);
            break;

        case tighten_notes:
            m_seq->push_undo();
            m_seq->quanize_events(EVENT_NOTE_ON, 0, m_snap, 2 , true);
            break;

        case tighten_events:
            m_seq->push_undo();
            m_seq->quanize_events(m_editing_status, m_editing_cc, m_snap, 2);
            break;

        case transpose:
            m_seq->push_undo();
            m_seq->transpose_notes(a_var, 0); 
            break; 

        case transpose_h:
            m_seq->push_undo();
            m_seq->transpose_notes(a_var, m_scale); 
            break;

        default:
            break;
    }

    m_seqroll_wid->redraw();
    m_seqtime_wid->redraw();
    m_seqdata_wid->redraw();  
    m_seqevent_wid->redraw();
}


void  
seqedit::fill_top_bar( void )
{
     /* name */
    m_entry_name = manage( new Entry(  ));
    m_entry_name->set_max_length(26);
    m_entry_name->set_width_chars(26);
    m_entry_name->set_text( m_seq->get_name());
    m_entry_name->select_region(0,0);
    m_entry_name->set_position(0);
    m_entry_name->signal_changed().connect(
            mem_fun( *this, &seqedit::name_change_callback));

    m_hbox->pack_start( *m_entry_name, true, true );
    m_hbox->pack_start( *(manage(new VSeparator( ))), false, false, 4);

    /* beats per measure */ 
    m_button_bpm = manage( new Button());
    m_button_bpm->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( down_xpm  ))));
    m_button_bpm->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_bpm  ));
    add_tooltip( m_button_bpm, "Time Signature. Beats per Measure" );
    m_entry_bpm = manage( new Entry());
    m_entry_bpm->set_width_chars(2);
    // ORL bpm can be set by a text type by the user
    m_entry_bpm->set_editable( true );
    m_entry_bpm->signal_changed().connect(
            mem_fun(*this, &seqedit::bpm_change_callback));
    // end of the mod

    m_hbox->pack_start( *m_button_bpm , false, false );
    m_hbox->pack_start( *m_entry_bpm , false, false );
 
    m_hbox->pack_start( *(manage(new Label( "/" ))), false, false, 4);

    /* beat width */
    m_button_bw = manage( new Button());
    m_button_bw->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( down_xpm  ))));
    m_button_bw->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_bw  ));
    add_tooltip( m_button_bw, "Time Signature. Length of Beat" );
    m_entry_bw = manage( new Entry());
    m_entry_bw->set_width_chars(2);
    m_entry_bw->set_editable( false );

    m_hbox->pack_start( *m_button_bw , false, false );
    m_hbox->pack_start( *m_entry_bw , false, false );

    /* length */
    m_button_length = manage( new Button());
    m_button_length->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( length_xpm  ))));
    m_button_length->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_length  ));
    add_tooltip( m_button_length, "Sequence length in Bars." );
    m_entry_length = manage( new Entry());
    m_entry_length->set_width_chars(3);
    // ORL number of measures may be set by a text typed by the user
    m_entry_length->set_editable( true );
    m_entry_length->signal_changed().connect(
            mem_fun(*this, &seqedit::measures_change_callback));
    // end of mod

    m_hbox->pack_start( *m_button_length , false, false );
    m_hbox->pack_start( *m_entry_length , false, false );


    m_hbox->pack_start( *(manage(new VSeparator( ))), false, false, 4);
    
    /* midi bus */
    m_button_bus = manage( new Button());
    m_button_bus->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( bus_xpm  ))));
    m_button_bus->signal_clicked().connect(
            mem_fun( *this, &seqedit::popup_midibus_menu));
    add_tooltip( m_button_bus, "Select Output Bus." );

    m_entry_bus = manage( new Entry());
    m_entry_bus->set_max_length(60);
    m_entry_bus->set_width_chars(60);
    m_entry_bus->set_editable( false );

    m_hbox->pack_start( *m_button_bus , false, false );
    m_hbox->pack_start( *m_entry_bus , true, true );

    /* midi channel */
    m_button_channel = manage( new Button());
    m_button_channel->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( midi_xpm  ))));
    m_button_channel->signal_clicked().connect(
            mem_fun( *this, &seqedit::popup_midich_menu ));
    add_tooltip( m_button_channel, "Select Midi channel." );
    m_entry_channel = manage( new Entry());
    m_entry_channel->set_width_chars(2);
    m_entry_channel->set_editable( false );

    m_hbox->pack_start( *m_button_channel , false, false );
    m_hbox->pack_start( *m_entry_channel , false, false );


    /* undo */
    m_button_undo = manage( new Button());
    m_button_undo->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( undo_xpm  ))));
    m_button_undo->signal_clicked().connect(
            mem_fun( *this, &seqedit::undo_callback));
    add_tooltip( m_button_undo, "Undo." );
 
    m_hbox2->pack_start( *m_button_undo , false, false );
    
    /* redo */
    m_button_redo = manage( new Button());
    m_button_redo->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( redo_xpm  ))));
    m_button_redo->signal_clicked().connect(
            mem_fun( *this, &seqedit::redo_callback));
    add_tooltip( m_button_redo, "Redo." );
 
    m_hbox2->pack_start( *m_button_redo , false, false );
    
    /* quantize shortcut */
    m_button_quanize = manage( new Button());
    m_button_quanize->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( quanize_xpm  ))));
    m_button_quanize->signal_clicked().connect(
            sigc::bind(mem_fun(*this, &seqedit::do_action), quantize_notes, 0));
    add_tooltip( m_button_quanize, "Quantize Selection." );
 
    m_hbox2->pack_start( *m_button_quanize , false, false );

    m_hbox2->pack_start( *(manage(new VSeparator( ))), false, false, 4);

    /* tools button */
    m_button_tools = manage( new Button( ));
    m_button_tools->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( tools_xpm  ))));
    m_button_tools->signal_clicked().connect(
            mem_fun( *this, &seqedit::popup_tool_menu ));
    m_tooltips->set_tip(  *m_button_tools, "Tools." );

    m_hbox2->pack_start( *m_button_tools , false, false );
    m_hbox2->pack_start( *(manage(new VSeparator( ))), false, false, 4);
    
    /* snap */
    m_button_snap = manage( new Button());
    m_button_snap->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( snap_xpm  ))));
    m_button_snap->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_snap  ));
    add_tooltip( m_button_snap, "Grid snap." );
    m_entry_snap = manage( new Entry());
    m_entry_snap->set_width_chars(5);
    m_entry_snap->set_editable( false );
    
    m_hbox2->pack_start( *m_button_snap , false, false );
    m_hbox2->pack_start( *m_entry_snap , false, false );

    /* note_length */
    m_button_note_length = manage( new Button());
    m_button_note_length->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( note_length_xpm  ))));
    m_button_note_length->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_note_length  ));
    add_tooltip( m_button_note_length, "Note Length." );
    m_entry_note_length = manage( new Entry());
    m_entry_note_length->set_width_chars(5);
    m_entry_note_length->set_editable( false );
    
    m_hbox2->pack_start( *m_button_note_length , false, false );
    m_hbox2->pack_start( *m_entry_note_length , false, false );


    /* zoom */
    m_button_zoom = manage( new Button());
    m_button_zoom->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( zoom_xpm  ))));
    m_button_zoom->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_zoom  ));
    add_tooltip( m_button_zoom, "Zoom. Pixels to Ticks" );
    m_entry_zoom = manage( new Entry());
    m_entry_zoom->set_width_chars(4);
    m_entry_zoom->set_editable( false );

    m_hbox2->pack_start( *m_button_zoom , false, false );
    m_hbox2->pack_start( *m_entry_zoom , false, false );

  
    m_hbox2->pack_start( *(manage(new VSeparator( ))), false, false, 4);

    /* key */
    m_button_key = manage( new Button());
    m_button_key->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( key_xpm  ))));
    m_button_key->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_key  ));
    add_tooltip( m_button_key, "Key of Sequence" );
    m_entry_key = manage( new Entry());
    m_entry_key->set_width_chars(5);
    m_entry_key->set_editable( false );
    
    m_hbox2->pack_start( *m_button_key , false, false );
    m_hbox2->pack_start( *m_entry_key , false, false );

    /* music scale */
    m_button_scale = manage( new Button());
    m_button_scale->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( scale_xpm  ))));
    m_button_scale->signal_clicked().connect(
            sigc::bind<Menu *>( mem_fun( *this, &seqedit::popup_menu),
                m_menu_scale  ));
    add_tooltip( m_button_scale, "Musical Scale" );
    m_entry_scale = manage( new Entry());
    m_entry_scale->set_width_chars(5);
    m_entry_scale->set_editable( false );

    m_hbox2->pack_start( *m_button_scale , false, false );
    m_hbox2->pack_start( *m_entry_scale , false, false );

    m_hbox2->pack_start( *(manage(new VSeparator( ))), false, false, 4);

    /* background sequence */
    m_button_sequence = manage( new Button());
    m_button_sequence->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( sequences_xpm  ))));
    m_button_sequence->signal_clicked().connect(
            mem_fun( *this, &seqedit::popup_sequence_menu));
    add_tooltip( m_button_sequence, "Background Sequence" );
    m_entry_sequence = manage( new Entry());
    m_entry_sequence->set_width_chars(14);
    m_entry_sequence->set_editable( false );

    m_hbox2->pack_start( *m_button_sequence , false, false );
    m_hbox2->pack_start( *m_entry_sequence , true, true );



#if 0
    /* Select */
    m_radio_select = manage( new RadioButton( "Sel", true ));
    m_radio_select->signal_clicked().connect( sigc::bind(mem_fun( *this, &seqedit::mouse_action ), e_action_select ));
    m_hbox3->pack_start( *m_radio_select, false, false );
    
    /* Draw */
    m_radio_draw = manage( new RadioButton( "Draw" ));
    m_radio_draw->signal_clicked().connect( sigc::bind(mem_fun( *this, &seqedit::mouse_action ), e_action_draw ));
    m_hbox3->pack_start( *m_radio_draw, false, false );
     
    /* Grow */
    m_radio_grow = manage( new RadioButton( "Grow" ));
    m_radio_grow->signal_clicked().connect( sigc::bind(mem_fun( *this, &seqedit::mouse_action ), e_action_grow ));
    m_hbox3->pack_start( *m_radio_grow, false, false );
     
    /* Stretch */

    Gtk::RadioButton::Group g = m_radio_select->get_group();
    m_radio_draw->set_group(g);
    m_radio_grow->set_group(g);
#endif 
}

#if 0
void 
seqedit::mouse_action( mouse_action_e a_action )
{
    if ( a_action == e_action_select && m_radio_select->get_active())
        printf( "mouse_action() select [%d]\n", a_action );
    
    if ( a_action == e_action_draw && m_radio_draw->get_active())
        printf( "mouse_action() draw [%d]\n", a_action );

    if ( a_action == e_action_grow && m_radio_grow->get_active())
        printf( "mouse_action() grow [%d]\n", a_action );
}
#endif

void
seqedit::popup_menu(Menu *a_menu)
{
    a_menu->popup(0,0);
}



void
seqedit::popup_midibus_menu( void )
{
    using namespace Menu_Helpers;

    m_menu_midibus = manage( new Menu());

    /* midi buses */
    mastermidibus *masterbus = m_mainperf->get_master_midi_bus();
    for ( int i=0; i< masterbus->get_num_out_buses(); i++ ){
        m_menu_midibus->items().push_back(MenuElem(
                    masterbus->get_midi_out_bus_name(i),
                    sigc::bind(mem_fun(*this,&seqedit::set_midi_bus), i)));
    }

    m_menu_midibus->popup(0,0);
}

void
seqedit::popup_midich_menu( void )
{
    using namespace Menu_Helpers;

    m_menu_midich = manage( new Menu());

    int midi_bus = m_seq->get_midi_bus();

    char b[16];
    
    /* midi channel menu */
    for( int i=0; i<16; i++ ){
        
        snprintf( b, sizeof(b), "%d", i+1 );
        std::string name = string(b);
        int instrument = global_user_midi_bus_definitions[midi_bus].instrument[i]; 
        if ( instrument >= 0 && instrument < c_maxBuses )
        {
            name = name + (string(" (") + 
                           global_user_instrument_definitions[instrument].instrument + 
                           string(")") );
        }
        m_menu_midich->items().push_back(MenuElem(name, 
                    sigc::bind(mem_fun(*this,&seqedit::set_midi_channel), 
                                                       i )));
    }

    m_menu_midich->popup(0,0);
}


void
seqedit::popup_sequence_menu( void )
{
    using namespace Menu_Helpers;


    m_menu_sequences = manage( new Menu());

    m_menu_sequences->items().push_back(MenuElem("Off",
                sigc::bind(mem_fun(*this, &seqedit::set_background_sequence), -1)));
    m_menu_sequences->items().push_back( SeparatorElem( ));

    for ( int ss=0; ss<c_max_sets; ++ss ){

        Menu *menu_ss = NULL;  
        bool inserted = false;
        
        for ( int seq=0; seq<  c_seqs_in_set; seq++ ){

            int i = ss * c_seqs_in_set + seq;

            char name[30];
            
            if ( m_mainperf->is_active( i )){

                if ( !inserted ){
                    inserted = true;
                    snprintf(name, sizeof(name), "[%d]", ss);
                    menu_ss = manage( new Menu());
                    m_menu_sequences->items().push_back(MenuElem(name,*menu_ss));
                }
                
                sequence *seq = m_mainperf->get_sequence( i );                
                snprintf(name, sizeof(name),"[%d] %.13s", i, seq->get_name());

                menu_ss->items().push_back(MenuElem(name,
                            sigc::bind(mem_fun(*this, &seqedit::set_background_sequence), i)));
                
            }
        }
    }
    
    m_menu_sequences->popup(0,0);
}


void
seqedit::set_background_sequence( int a_seq )
{
    char name[30];

    m_initial_sequence = m_sequence = a_seq;

    if ( a_seq == -1 || !m_mainperf->is_active( a_seq )){
        m_entry_sequence->set_text("Off");
         m_seqroll_wid->set_background_sequence( false, 0 );
    }
    
    if ( m_mainperf->is_active( a_seq )){

        sequence *seq = m_mainperf->get_sequence( a_seq );                
        snprintf(name, sizeof(name),"[%d] %.13s", a_seq, seq->get_name());
        m_entry_sequence->set_text(name);

        m_seqroll_wid->set_background_sequence( true, a_seq );

    }
}


Gtk::Image*
seqedit::create_menu_image( bool a_state )
{
    if ( a_state )
        return manage( new Image(Gdk::Pixbuf::create_from_xpm_data( menu_full_xpm  )));
    else
        return manage( new Image(Gdk::Pixbuf::create_from_xpm_data( menu_empty_xpm  )));
}


void
seqedit::popup_event_menu( void )
{
    using namespace Menu_Helpers;

    /* temp */
    char b[20];

    bool note_on = false;
    bool note_off = false;
    bool aftertouch = false;
    bool program_change = false;
    bool channel_pressure = false;
    bool pitch_wheel = false;
    bool ccs[128];
    memset( ccs, false, sizeof(bool) * 128 );

    int midi_bus = m_seq->get_midi_bus();
    int midi_ch = m_seq->get_midi_channel();

    unsigned char status, cc;
    m_seq->reset_draw_marker();
    while ( m_seq->get_next_event( &status, &cc ) == true ){

        switch( status ){
            case EVENT_NOTE_OFF:
                note_off = true;
                break;	  
            case EVENT_NOTE_ON:
                note_on = true;
                break;
            case EVENT_AFTERTOUCH:
                aftertouch = true;
                break;
            case EVENT_CONTROL_CHANGE:
                ccs[cc] = true;
                break;
            case EVENT_PITCH_WHEEL:
                pitch_wheel = true;
                break;
                /* one data item */
            case EVENT_PROGRAM_CHANGE:
                program_change = true;
                break;
            case EVENT_CHANNEL_PRESSURE:
                channel_pressure = true;
                break;
        }
    }

    m_menu_data = manage( new Menu());

    m_menu_data->items().push_back( ImageMenuElem( "Note On Velocity",
                *create_menu_image( note_on ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_NOTE_ON, 0 )));

    m_menu_data->items().push_back( SeparatorElem( )); 

    m_menu_data->items().push_back( ImageMenuElem( "Note Off Velocity",
                *create_menu_image( note_off ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_NOTE_OFF, 0 )));

    m_menu_data->items().push_back( ImageMenuElem( "AfterTouch",
                *create_menu_image( aftertouch ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_AFTERTOUCH, 0 )));

    m_menu_data->items().push_back( ImageMenuElem( "Program Change",
                *create_menu_image( program_change ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_PROGRAM_CHANGE, 0 )));

    m_menu_data->items().push_back( ImageMenuElem( "Channel Pressure",
                *create_menu_image( channel_pressure ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_CHANNEL_PRESSURE, 0 )));

    m_menu_data->items().push_back( ImageMenuElem( "Pitch Wheel",
                *create_menu_image( pitch_wheel ),
                sigc::bind(mem_fun(*this, &seqedit::set_data_type),
                    (unsigned char) EVENT_PITCH_WHEEL , 0 )));

    m_menu_data->items().push_back( SeparatorElem( )); 

    /* create control change */
    for ( int i=0; i<8; i++ ){

        snprintf(b, sizeof(b), "Controls %d-%d", (i*16), (i*16) + 15);
        Menu *menu_cc = manage( new Menu() ); 

        for( int j=0; j<16; j++ ){
            string controller_name( c_controller_names[i*16+j] );
            int instrument = global_user_midi_bus_definitions[midi_bus].instrument[midi_ch];  
            if ( instrument > -1 && instrument < c_max_instruments )
            {
                if ( global_user_instrument_definitions[instrument].controllers_active[i*16+j] )
                    controller_name = global_user_instrument_definitions[instrument].controllers[i*16+j];
            }

            menu_cc->items().push_back( ImageMenuElem( controller_name,
                        *create_menu_image( ccs[i*16+j]),
                        sigc::bind(mem_fun(*this, &seqedit::set_data_type), 
                            (unsigned char) EVENT_CONTROL_CHANGE, i*16+j)));
        }
        m_menu_data->items().push_back( MenuElem( string(b), *menu_cc ));
    }

    m_menu_data->popup(0,0);
}


    //m_option_midich->set_history( m_seq->getMidiChannel() );
    //m_option_midibus->set_history( m_seq->getMidiBus()->getID() );

void 
seqedit::set_midi_channel( int a_midichannel  )
{
    char b[10];

    snprintf( b, sizeof(b), "%d", a_midichannel + 1);
    m_entry_channel->set_text(b);
    m_seq->set_midi_channel( a_midichannel );
    // m_mainwid->update_sequence_on_window( m_pos );
}


void 
seqedit::set_midi_bus( int a_midibus )
{
    m_seq->set_midi_bus( a_midibus );
	mastermidibus *mmb =  m_mainperf->get_master_midi_bus();
    m_entry_bus->set_text( mmb->get_midi_out_bus_name( a_midibus ));
    // m_mainwid->update_sequence_on_window( m_pos );
}


void 
seqedit::set_zoom( int a_zoom  )
{
    char b[10];

    snprintf(b, sizeof(b), "1:%d", a_zoom);
    m_entry_zoom->set_text(b);

    m_zoom = a_zoom;
    m_initial_zoom = a_zoom;
    m_seqroll_wid->set_zoom( m_zoom );
    m_seqtime_wid->set_zoom( m_zoom );
    m_seqdata_wid->set_zoom( m_zoom ); 
    m_seqevent_wid->set_zoom( m_zoom );
}


void 
seqedit::set_snap( int a_snap  )
{
    char b[10];

    snprintf(b, sizeof(b), "1/%d",   c_ppqn * 4 / a_snap);
    m_entry_snap->set_text(b);
    
    m_snap = a_snap;
    m_initial_snap = a_snap;
    m_seqroll_wid->set_snap( m_snap );
    m_seqevent_wid->set_snap( m_snap );
    m_seq->set_snap_tick(a_snap);
}


void 
seqedit::set_note_length( int a_note_length  )
{
    char b[10];

    snprintf(b, sizeof(b), "1/%d",   c_ppqn * 4 / a_note_length);
    m_entry_note_length->set_text(b);
    
    m_note_length = a_note_length;
    m_initial_note_length = a_note_length;
    m_seqroll_wid->set_note_length( m_note_length );
}


void 
seqedit::set_scale( int a_scale )
{
  m_entry_scale->set_text( c_scales_text[a_scale] );

  m_scale = m_initial_scale = a_scale;

  m_seqroll_wid->set_scale( m_scale );
  m_seqkeys_wid->set_scale( m_scale );


}

void 
seqedit::set_key( int a_note )
{ 
  m_entry_key->set_text( c_key_text[a_note] );

  m_key = m_initial_key = a_note;
  
  m_seqroll_wid->set_key( m_key );
  m_seqkeys_wid->set_key( m_key );

}


void
seqedit::apply_length( int a_bpm, int a_bw, int a_measures )
{
  m_seq->set_length( a_measures * a_bpm * ((c_ppqn * 4) / a_bw) );

  m_seqroll_wid->reset();
  m_seqtime_wid->reset();
  m_seqdata_wid->reset();  
  m_seqevent_wid->reset();
    
}


long
seqedit::get_measures( void )
{
    long units = ((m_seq->get_bpm() * (c_ppqn * 4)) /  m_seq->get_bw() );

    long measures = (m_seq->get_length() / units);

    if (m_seq->get_length() % units != 0)
        measures++;

    return measures;
}


void 
seqedit::set_measures( int a_length_measures  )
{
    char b[10];

    snprintf(b, sizeof(b), "%d", a_length_measures);
    m_entry_length->set_text(b);

    m_measures = a_length_measures;
    apply_length( m_seq->get_bpm(), m_seq->get_bw(), a_length_measures );
}

// ORL Number of measures (length of sequence) can be set by a text typed by user
void
seqedit::measures_change_callback(void)
{
    char b[4];

    int m_int_length = atoi( m_entry_length->get_text().c_str() );

    if( m_int_length < 0 ){

        fprintf( stderr, "Number of measures must be > 1\n" );
        snprintf(b, sizeof(b), "%ld", get_measures() );
        m_entry_length->set_text(b);

    } else if( m_int_length > 1024) {

        fprintf( stderr, "Number of measures must be < 1024\n" );
        snprintf(b, sizeof(b), "%ld", get_measures() );
        m_entry_length->set_text(b);
        // This max value may be changed. Actually, I'm not even sure that's it's really important to set a max value.

    } else if (m_int_length == 0) {
        // This else statement just allows to remove everything in the Entry before typing something new
    } else {

        m_measures = m_int_length;
        apply_length( m_seq->get_bpm(), m_seq->get_bw(), m_measures );

    }
}

void 
seqedit::set_bpm( int a_beats_per_measure )
{
    char b[4];

    snprintf(b, sizeof(b), "%d", a_beats_per_measure);
    m_entry_bpm->set_text(b);

    if ( a_beats_per_measure != m_seq->get_bpm() ){

        long length = get_measures();
        m_seq->set_bpm( a_beats_per_measure );
        apply_length( a_beats_per_measure, m_seq->get_bw(), length );
    }
}

// ORL BPM can be set by a text typed by user
void
seqedit::bpm_change_callback(void)
{
    char b[4];

    int m_int_bpm = atoi( m_entry_bpm->get_text().c_str() );

    if( m_int_bpm < 0 ){

        fprintf( stderr, "Number of beats in bar must be > 1\n" );
        snprintf(b, sizeof(b), "%ld", m_seq->get_bpm() );
        m_entry_bpm->set_text(b);

    } else if( m_int_bpm > 128) {

        fprintf( stderr, "Number of beats in bar must be < 128\n" );
        snprintf(b, sizeof(b), "%ld", m_seq->get_bpm() );
        m_entry_bpm->set_text(b);
        // This max value may be changed. Actually, going over 32 is probably senseless, but -due to the minimal division (128), this value will allow every composed bars within these decomposition

    } else if (m_int_bpm == 0) {
        // This else statement just allows to remove everything in the Entry before typing something new
    } else {

        long length = get_measures();
        m_seq->set_bpm( m_int_bpm );
        apply_length( m_int_bpm, m_seq->get_bw(), length );
       
    }
}

void 
seqedit::set_bw( int a_beat_width  )
{
    char b[4];

    snprintf(b, sizeof(b), "%d", a_beat_width);
    m_entry_bw->set_text(b);

    if ( a_beat_width != m_seq->get_bw()){

        long length = get_measures();
        m_seq->set_bw( a_beat_width );
        apply_length( m_seq->get_bpm(), a_beat_width, length );
    }
}


void
seqedit::set_rec_vol( int a_rec_vol  )
{
    m_seq->set_rec_vol( a_rec_vol );
}


void 
seqedit::name_change_callback( void )
{
    m_seq->set_name( m_entry_name->get_text());
    // m_mainwid->update_sequence_on_window( m_pos );
}


void 
seqedit::play_change_callback( void )
{
    m_seq->set_playing( m_toggle_play->get_active() );
    // m_mainwid->update_sequence_on_window( m_pos );
}


void 
seqedit::record_change_callback( void )
{
    m_mainperf->get_master_midi_bus()->set_sequence_input( true, m_seq );
    m_seq->set_recording( m_toggle_record->get_active() );
}


void 
seqedit::q_rec_change_callback( void )
{
    m_seq->set_quanized_rec( m_toggle_q_rec->get_active() );
}


void 
seqedit::undo_callback( void )
{
	m_seq->pop_undo( );
 	
	m_seqroll_wid->redraw();
	m_seqtime_wid->redraw();
	m_seqdata_wid->redraw();  
	m_seqevent_wid->redraw();
}


void 
seqedit::redo_callback( void )
{
	m_seq->pop_redo( );
 	
	m_seqroll_wid->redraw();
	m_seqtime_wid->redraw();
	m_seqdata_wid->redraw();  
	m_seqevent_wid->redraw();
}


void 
seqedit::thru_change_callback( void )
{
    m_mainperf->get_master_midi_bus()->set_sequence_input( true, m_seq );
    m_seq->set_thru( m_toggle_thru->get_active() );
}


void 
seqedit::set_data_type( unsigned char a_status, unsigned char a_control  )
{
    m_editing_status = a_status;
    m_editing_cc = a_control;

    m_seqevent_wid->set_data_type( a_status, a_control );
    m_seqdata_wid->set_data_type( a_status, a_control );
    m_seqroll_wid->set_data_type( a_status, a_control );

    char text[100];
    char hex[20];
    char type[80];

    snprintf(hex, sizeof(hex), "[0x%02X]", a_status);  

    if ( a_status == EVENT_NOTE_OFF )         
        snprintf(type, sizeof(type),"Note Off" );
    else if ( a_status == EVENT_NOTE_ON )          
        snprintf(type, sizeof(type),"Note On" );  
    else if ( a_status == EVENT_AFTERTOUCH )       
        snprintf(type, sizeof(type), "Aftertouch" );       
    else if ( a_status == EVENT_CONTROL_CHANGE )
    {
        int midi_bus = m_seq->get_midi_bus();
        int midi_ch = m_seq->get_midi_channel();

        string controller_name( c_controller_names[a_control] );
        int instrument = global_user_midi_bus_definitions[midi_bus].instrument[midi_ch];  
        if ( instrument > -1 && instrument < c_max_instruments )
        {
            if ( global_user_instrument_definitions[instrument].controllers_active[a_control] )
                controller_name = global_user_instrument_definitions[instrument].controllers[a_control];
        }
        snprintf(type, sizeof(type), "Control Change - %s",
                controller_name.c_str() );  
    }

    else if ( a_status == EVENT_PROGRAM_CHANGE )   
        snprintf(type, sizeof(type), "Program Change");       
    else if ( a_status == EVENT_CHANNEL_PRESSURE ) 
        snprintf(type, sizeof(type), "Channel Pressure");     
    else if ( a_status == EVENT_PITCH_WHEEL )      
        snprintf(type, sizeof(type), "Pitch Wheel");          
    else
        snprintf(type, sizeof(type), "Unknown MIDI Event");          

    snprintf(text, sizeof(text), "%s %s", hex, type );

    m_entry_data->set_text( text );
}


void 
seqedit::on_realize()
{
    // we need to do the default realize
    Gtk::Window::on_realize();
    Glib::signal_timeout().connect(mem_fun(*this, &seqedit::timeout),
            c_redraw_ms);
}


bool
seqedit::timeout( void )
{

    if (m_seq->get_raise())
    {
        m_seq->set_raise(false);
        raise();
    }
    
    if (m_seq->is_dirty_edit() ){

	    m_seqroll_wid->redraw_events();
	    m_seqevent_wid->redraw();
	    m_seqdata_wid->redraw();
    }

    m_seqroll_wid->draw_progress_on_window();
    
    return true;
}


seqedit::~seqedit()
{
    //m_seq->set_editing( false );
}


bool
seqedit::on_delete_event(GdkEventAny *a_event)
{
    //printf( "seqedit::on_delete_event()\n" );
    m_seq->set_recording( false );
    m_mainperf->get_master_midi_bus()->set_sequence_input( false, NULL );
    m_seq->set_editing( false );

    delete this;
    
    return false;
}


bool
seqedit::on_scroll_event( GdkEventScroll* a_ev )
{
    //printf("seqedit::on_scroll_event(x=%f,y=%f,state=%d)\n", a_ev->x, a_ev->y, a_ev->state);

    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();

    if ((a_ev->state & modifiers) == GDK_CONTROL_MASK)
    {
        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            if (m_zoom*2 <= c_max_zoom)
                set_zoom(m_zoom*2);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            if (m_zoom/2 >= c_min_zoom)
                set_zoom(m_zoom/2);
        }
        return true;
    }
    else if ((a_ev->state & modifiers) == GDK_SHIFT_MASK)
    {
        double val = m_hadjust->get_value();
        double step = m_hadjust->get_step_increment();
        double upper = m_hadjust->get_upper();

        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            if (val + step < upper)
                m_hadjust->set_value(val + step);
            else
                m_hadjust->set_value(upper);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            m_hadjust->set_value(val - step);
        }
        return true;
    }

    return false;  // Not handled
}

bool
seqedit::on_key_press_event( GdkEventKey* a_ev )
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();

    if ((a_ev->state & modifiers) == GDK_CONTROL_MASK && a_ev->keyval == 'w')
        return on_delete_event((GdkEventAny*)a_ev);
    else
        return Gtk::Window::on_key_press_event(a_ev);
}

