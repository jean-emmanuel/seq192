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

#include "options.h"
#include <sstream>

// tooltip helper, for old vs new gtk...
#if GTK_MINOR_VERSION >= 12
#   define add_tooltip( obj, text ) obj->set_tooltip_text( text);
#else
#   define add_tooltip( obj, text ) tooltips->set_tip( *obj, text );
#endif

const int c_status = 0;
const int c_status_inv = 1;
const int c_d1 = 2;
const int c_d2 = 3;
const int c_d3 = 4;
enum
{
    e_keylabelsonsequence = 9999
};


// GTK text edit widget for getting keyboard button values (for binding keys)
// put cursor in text box, hit a key, something like  'a' (42)  appears...
// each keypress replaces the previous text.
// also supports keyevent and keygroup maps in the perform class
class KeyBindEntry : public Entry
{
public:
    enum type { location, events, groups };

    KeyBindEntry(   type t,
                    unsigned int* location_to_write = NULL,
                    perform* p = NULL,
                    long s = 0 ) :  Entry(),
                                    m_key( location_to_write ),
                                    m_type( t ),
                                    m_perf( p ),
                                    m_slot( s )
    {
        switch (m_type)
        {
            case location: if (m_key) set( *m_key ); break;
            case events: set( m_perf->lookup_keyevent_key( m_slot ) ); break;
            case groups: set( m_perf->lookup_keygroup_key( m_slot ) ); break;
        }
    }
    
    void set( unsigned int val )
    {
        char buf[256] = "";
        char* special = gdk_keyval_name( val );
        if (special)
            sprintf( &buf[strlen(buf)], "%s", special );
        else
            sprintf( &buf[strlen(buf)], "'%c'", (char)val );
        set_text( buf );
        int width = strlen(buf)-1;
        set_width_chars( 1 <= width ? width : 1 );
    }

    virtual bool on_key_press_event(GdkEventKey* event)
    {
        bool result = Entry::on_key_press_event( event );
        set( event->keyval );
        switch (m_type)
        {
            case location: if (m_key) *m_key = event->keyval; break;
            case events: m_perf->set_key_event( event->keyval, m_slot ); break;
            case groups: m_perf->set_key_group( event->keyval, m_slot ); break;
        }
        return result;
    }

    unsigned int* m_key;
    type m_type;
    perform* m_perf;
    long m_slot;
};


options::options (Gtk::Window & parent, perform * a_p):
    Gtk::Dialog ("Options", parent, true, true)
{
    m_perf = a_p;
    VBox *vbox = NULL;
    
    HBox *hbox = manage (new HBox ());
    get_vbox ()->pack_start (*hbox, false, false);

    get_action_area ()->set_border_width (2);
    hbox->set_border_width (6);

    m_button_ok = manage (new Button ("Ok"));
    get_action_area ()->pack_end (*m_button_ok, false, false);
    m_button_ok->signal_clicked ().connect (mem_fun (*this, &options::hide));


    m_notebook = manage (new Notebook ());
    hbox->pack_start (*m_notebook);

    // Clock  Buses
    int buses = m_perf->get_master_midi_bus ()->get_num_out_buses ();
    //Notebook *clock_notebook = manage( new Notebook());
    //clock_notebook->set_scrollable(true);

    vbox = manage(new VBox());
    m_notebook->pages().push_back(Notebook_Helpers::TabElem(*vbox,
                "MIDI Clock"));

    CheckButton *check;
    Label *label;
    
    Gtk::Tooltips * tooltips = manage (new Tooltips ());

    for (int i = 0; i < buses; i++)
    {  
        HBox *hbox2 = manage (new HBox ());
        label = manage( new Label(m_perf->get_master_midi_bus ()->
                                            get_midi_out_bus_name (i), 0));

        hbox2->pack_start (*label, false, false);
        
        
        Gtk::RadioButton * rb_off = manage (new RadioButton ("Off"));
        add_tooltip( rb_off, "Midi Clock will be disabled.");
        
        Gtk::RadioButton * rb_on = manage (new RadioButton ("On (Pos)"));
        add_tooltip( rb_on,
                "Midi Clock will be sent. Midi Song Position and Midi Continue will be sent if starting greater than tick 0 in song mode, otherwise Midi Start is sent.");

        Gtk::RadioButton * rb_mod = manage (new RadioButton ("On (Mod)"));
        add_tooltip( rb_mod, "Midi Clock will be sent.  Midi Start will be sent and clocking will begin once the song position has reached the modulo of the specified Size. (Used for gear that doesn't respond to Song Position)");

        Gtk::RadioButton::Group group = rb_off->get_group ();
        rb_on->set_group (group);
        rb_mod->set_group (group);

        rb_off->signal_toggled().connect (sigc::bind(mem_fun (*this, &options::clock_callback_off), i, rb_off ));
        rb_on->signal_toggled ().connect (sigc::bind(mem_fun (*this, &options::clock_callback_on),  i, rb_on  ));
        rb_mod->signal_toggled().connect (sigc::bind(mem_fun (*this, &options::clock_callback_mod), i, rb_mod ));
        
        hbox2->pack_end (*rb_mod, false, false ); 
        hbox2->pack_end (*rb_on, false, false);
        hbox2->pack_end (*rb_off, false, false);

        vbox->pack_start( *hbox2, false, false );
       
        switch ( m_perf->get_master_midi_bus ()->get_clock (i))
        {
            case e_clock_off: rb_off->set_active(1); break;
            case e_clock_pos: rb_on->set_active(1); break;
            case e_clock_mod: rb_mod->set_active(1); break;
        }
                              
        // SET DEFAULT STATES check->set_active (m_perf->get_master_midi_bus ()->get_clock (i));
    }

    Adjustment *clock_mod_adj = new Adjustment( midibus::get_clock_mod(), 1, 16 << 10, 1 );
    SpinButton *clock_mod_spin = new SpinButton( *clock_mod_adj );

    HBox *hbox2 = manage (new HBox ());
    
    //m_spinbutton_bpm->set_editable( false );
    hbox2->pack_start(*(manage( new Label( "Clock Start Modulo (1/16 Notes)"))), false, false, 4);
    hbox2->pack_start(*clock_mod_spin, false, false );

    vbox->pack_start( *hbox2, false, false );
    
    clock_mod_adj->signal_value_changed().connect( sigc::bind(mem_fun(*this,&options::clock_mod_callback),clock_mod_adj));

    // add controls for input method
    {
        Adjustment *adj = new Adjustment( global_interactionmethod, 0, e_number_of_interactions-1, 1 );
        SpinButton *spin = new SpinButton( *adj );

        HBox *hbox2 = manage (new HBox ());
        HBox *hbox3 = manage (new HBox ());

        //m_spinbutton_bpm->set_editable( false );
        interaction_method_label = new Label("Input Method");
        hbox2->pack_start(*(manage( interaction_method_label )), false, false, 4);
        hbox2->pack_start(*spin, false, false );

        vbox->pack_start( *hbox2, false, false );

        interaction_method_desc_label = new Label(" ----- ");
        hbox3->pack_start(*(manage( interaction_method_desc_label )), false, false, 4);
        vbox->pack_start(*hbox3, false, false );

        adj->signal_value_changed().connect( sigc::bind(mem_fun(*this,&options::interaction_method_callback),adj));

        // force it to refresh.
        interaction_method_callback( adj );
    }

    // Input Buses
    buses = m_perf->get_master_midi_bus ()->get_num_in_buses ();

    vbox = manage (new VBox ());
    m_notebook->pages ().
        push_back (Notebook_Helpers::TabElem (*vbox, "MIDI Input"));

    for (int i = 0; i < buses; i++)
    {

        check =
            manage (new
                    CheckButton (m_perf->get_master_midi_bus ()->
                        get_midi_in_bus_name (i), 0));
        check->signal_toggled ().
            connect (bind (mem_fun (*this, &options::input_callback), i, check));
        check->set_active (m_perf->get_master_midi_bus ()->get_input (i));

        vbox->pack_start (*check, false, false);
    }

    // KeyBoard keybinding setup (editor for .seq24rc keybindings.
    vbox = manage (new VBox ());
    m_notebook->pages ().push_back (Notebook_Helpers::TabElem (*vbox, "Keyboard"));
    {
        Label* label;
        KeyBindEntry* entry;
        HBox *hbox;

        #define AddKey(text, integer) \
            label = manage (new Label( text )); \
            hbox->pack_start (*label, false, false, 4); \
            entry = manage (new KeyBindEntry( KeyBindEntry::location, &integer )); \
            hbox->pack_start (*entry, false, false, 4);
        #define AddKeyL(text) \
            label = manage (new Label( text )); \
            hbox->pack_start (*label, false, false, 4);
        #define AddKeyM(text, type, slot) \
            label = manage (new Label( text )); \
            hbox->pack_start (*label, false, false, 4); \
            entry = manage (new KeyBindEntry( type, NULL, m_perf, slot )); \
            hbox->pack_start (*entry, false, false, 4);

        hbox = manage (new HBox ());
        check = manage (new CheckButton ("show key labels on sequences", 0));
        check->signal_toggled ().
            connect (bind (mem_fun (*this, &options::input_callback), (int)e_keylabelsonsequence, check));
        check->set_active (m_perf->m_show_ui_sequence_key);
        vbox->pack_start (*check, false, false);

        hbox = manage (new HBox ());
        AddKey( "stop:", m_perf->m_key_stop );
        AddKey( "start:", m_perf->m_key_start );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "bpm dn:", m_perf->m_key_bpm_dn );
        AddKey( "bpm up:", m_perf->m_key_bpm_up );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "snpsht1:", m_perf->m_key_snapshot_1 );
        AddKey( "snpsht2:", m_perf->m_key_snapshot_2 );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "replace:", m_perf->m_key_replace );
        AddKey( "queue:", m_perf->m_key_queue );
        AddKey( "keep queue:", m_perf->m_key_keep_queue );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "scrnset dn:", m_perf->m_key_screenset_dn );
        AddKey( "scrnset up:", m_perf->m_key_screenset_up );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "set plying scrnset:", m_perf->m_key_set_playing_screenset );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKeyL( "sequence toggle keys >>" );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        std::map<unsigned int, long>::const_iterator it;
        int x = 0;
        for (int ss = 0; ss < 32; ++ss)
        {
            int s = (ss%8) * 4 + ss/8; // count this way... 0,4,8,16...
            //unsigned int keycode = m_perf->lookup_keyevent_key( s );
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", s);
            AddKeyM( buf, KeyBindEntry::events, s );
            ++x;
            if (x == 8)
            {
                vbox->pack_start (*hbox, false, false);
                hbox = manage (new HBox ());
                x = 0;
            }
        }
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKeyL( "mute-group slots >>" );
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        x = 0;
        for (int s = 0; s < 128; ++s)
        {
            //unsigned int keycode = m_perf->lookup_keygroup_key( s );
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", s);
            AddKeyM( buf, KeyBindEntry::groups, s );
            ++x;
            if (x == 8)
            {
                vbox->pack_start (*hbox, false, false);
                hbox = manage (new HBox ());
                x = 0;
            }
        }
        vbox->pack_start (*hbox, false, false);

        hbox = manage (new HBox ());
        AddKey( "learn (while pressing a mute-group key):", m_perf->m_key_group_learn );
        AddKey( "disable:", m_perf->m_key_group_off );
        AddKey( "enable:", m_perf->m_key_group_on );
        vbox->pack_start (*hbox, false, false);

        #undef AddKeyL
        #undef AddKey
    }

    // Jack
#ifdef JACK_SUPPORT
    VBox *vbox2 = manage (new VBox ());
    vbox2->set_border_width (4);
    m_notebook->pages().push_back(Notebook_Helpers::TabElem(*vbox2,
                "Jack Sync"));

    check = manage (new CheckButton ("Jack Transport"));
    check->set_active (global_with_jack_transport);
    add_tooltip( check, "Enable sync with JACK Transport.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_transport,
                 check));
    vbox2->pack_start (*check, false, false);

    check = manage (new CheckButton ("Transport Master"));
    check->set_active (global_with_jack_master);
    add_tooltip( check, "Seq24 will attempt to serve as JACK Master.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_master,
                 check));

    vbox2->pack_start (*check, false, false);

    check = manage (new CheckButton ("Master Conditional"));
    check->set_active (global_with_jack_master_cond);
    add_tooltip( check,
            "Seq24 will fail to be master if there is already a master set.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_master_cond,
                 check));

    vbox2->pack_start (*check, false, false);


    Gtk::RadioButton * rb_live = manage (new RadioButton ("Live Mode"));
    add_tooltip( rb_live,
            "Playback will be in live mode.  Use this to allow muting and unmuting of loops.");

    Gtk::RadioButton * rb_perform = manage (new RadioButton ("Song Mode"));
    add_tooltip( rb_perform, "Playback will use the song editors data.");

    Gtk::RadioButton::Group group = rb_live->get_group ();
    rb_perform->set_group (group);

    if (global_jack_start_mode)
    {
        rb_perform->set_active (true);
    }
    else
    {
        rb_live->set_active (true);
    }

    rb_perform->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback),
                 e_jack_start_mode_song, rb_perform));

    vbox2->pack_start (*rb_live, false, false);
    vbox2->pack_start (*rb_perform, false, false);

    Gtk::Button * button = manage (new Button ("Connect"));
    add_tooltip( button, "Connect to Jack.");
    button->signal_clicked().connect(bind(mem_fun(*this,
                    &options::transport_callback), e_jack_connect, button));
    vbox2->pack_start (*button, false, false);

    button = manage (new Button ("Disconnect"));
    add_tooltip( button, "Disconnect Jack.");
    button->signal_clicked().connect(bind(mem_fun(*this,
                    &options::transport_callback), e_jack_disconnect, button));
    vbox2->pack_start (*button, false, false);
#endif

    /* show everything */
    show_all_children ();
}



void
options::clock_callback_off (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_off );
}

void
options::clock_callback_on (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_pos );
}

void
options::clock_callback_mod (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_mod );
}

void 
options::clock_mod_callback( Adjustment *adj )
{
    midibus::set_clock_mod((int)adj->get_value());
}
 
void
options::interaction_method_callback( Adjustment *adj )
{
    global_interactionmethod = (interaction_method_e)(int)adj->get_value();
    std::string text = "Interaction Method (";
    text += c_interaction_method_names[(int)adj->get_value()];
    text += "): ";
    interaction_method_label->set_text( text.c_str() );
    
    text = "     (";
    text += c_interaction_method_descs[(int)adj->get_value()];
    text += ")";
    interaction_method_desc_label->set_text( text.c_str() );
}


    void
options::input_callback (int a_bus, Button * i_button)
{
    CheckButton *a_button = (CheckButton *) i_button;
    bool input = a_button->get_active ();
    if (9999 == a_bus) {
        m_perf->m_show_ui_sequence_key = input;
        for (int i=0; i< c_max_sequence; i++ ){
            if (m_perf->get_sequence( i ))
                m_perf->get_sequence( i )->set_dirty();
        }
        return;
    }
    m_perf->get_master_midi_bus ()->set_input (a_bus, input);
}


    void
options::transport_callback (button a_type, Button * a_check)
{
    CheckButton *check = (CheckButton *) a_check;

    switch (a_type)
    {

        case e_jack_transport:
            {
                global_with_jack_transport = check->get_active ();
            }
            break;

        case e_jack_master:
            {
                global_with_jack_master = check->get_active ();
            }
            break;

        case e_jack_master_cond:
            {
                global_with_jack_master_cond = check->get_active ();
            }
            break;

        case e_jack_start_mode_song:
            {
                //printf( "toggle %d\n" ,  check->get_active() );
                global_jack_start_mode = check->get_active ();
            }
            break;

        case e_jack_connect:
            {
                m_perf->init_jack ();
            }
            break;


        case e_jack_disconnect:
            {
                m_perf->deinit_jack ();
            }
            break;

        default:
            break;
    }
}
