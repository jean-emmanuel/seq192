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

#include <cctype>
#include <gtk/gtkversion.h>

#include "mainwnd.h"
#include "perform.h"
#include "midifile.h"
#include "perfedit.h"

#include "play2.xpm"
#include "stop.xpm"
#include "learn.xpm"
#include "learn2.xpm"
#include "perfedit.xpm"
#include "seq24.xpm"
#include "seq24_32.xpm"

bool is_pattern_playing = false;

// tooltip helper, for old vs new gtk...
#if GTK_MINOR_VERSION >= 12
#   define add_tooltip( obj, text ) obj->set_tooltip_text( text);
#else
#   define add_tooltip( obj, text ) m_tooltips->set_tip( *obj, text );
#endif

mainwnd::mainwnd(perform *a_p)
{
    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq24_32_xpm));

    /* set the performance */
    m_mainperf = a_p;
    
    /* register for notification */
    m_mainperf->m_notify.push_back( this );

    /* main window */
    update_window_title();

#if GTK_MINOR_VERSION < 12
    m_tooltips = manage( new Tooltips() );
#endif
    m_main_wid = manage( new mainwid( m_mainperf ));
    m_main_time = manage( new maintime( ));

    m_menubar = manage(new MenuBar());

    m_menu_file = manage(new Menu());
    m_menubar->items().push_front(MenuElem("_File", *m_menu_file));
    
    m_menu_view = manage( new Menu());
    m_menubar->items().push_back(MenuElem("_View", *m_menu_view));

    m_menu_help = manage( new Menu());
    m_menubar->items().push_back(MenuElem("_Help", *m_menu_help));
    
    /* file menu items */
    m_menu_file->items().push_back(MenuElem("_New",
                Gtk::AccelKey("<control>N"),
                mem_fun(*this, &mainwnd::file_new)));
    m_menu_file->items().push_back(MenuElem("_Open...",
                Gtk::AccelKey("<control>O"),
                mem_fun(*this, &mainwnd::file_open)));
    m_menu_file->items().push_back(MenuElem("_Save",
                Gtk::AccelKey("<control>S"),
                mem_fun(*this, &mainwnd::file_save)));
    m_menu_file->items().push_back(MenuElem("Save _as...",
                mem_fun(*this, &mainwnd::file_save_as)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("_Import...",
                mem_fun(*this, &mainwnd::file_import_dialog)));
    m_menu_file->items().push_back(MenuElem("O_ptions...",
                mem_fun(*this,&mainwnd::options_dialog)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("E_xit",
                Gtk::AccelKey("<control>Q"),
                mem_fun(*this, &mainwnd::file_exit)));

    /* view menu items */
    m_menu_view->items().push_back(MenuElem("_Song Editor...",
                Gtk::AccelKey("<control>E"),
                mem_fun(*this, &mainwnd::open_performance_edit)));
 
    /* help menu items */
    m_menu_help->items().push_back(MenuElem("_About...",
                mem_fun(*this, &mainwnd::about_dialog)));
 

    /* bottom line items */
    HBox *hbox = manage( new HBox( false, 2 ) );

    /* stop button */
    m_button_stop = manage( new Button( ));
    m_button_stop->add(*manage(new Image(
                    Gdk::Pixbuf::create_from_xpm_data( stop_xpm ))));
    m_button_stop->signal_clicked().connect(
            mem_fun(*this, &mainwnd::stop_playing));
    add_tooltip( m_button_stop, "Stop playing MIDI sequence" );
    hbox->pack_start(*m_button_stop, false, false);

    /* play button */
    m_button_play = manage(new Button() );
    m_button_play->add(*manage(new Image(
                    Gdk::Pixbuf::create_from_xpm_data( play2_xpm ))));
    m_button_play->signal_clicked().connect(
            mem_fun( *this, &mainwnd::start_playing));
    add_tooltip( m_button_play, "Play MIDI sequence" );
    hbox->pack_start(*m_button_play, false, false);

    /* song edit button */
    m_button_perfedit = manage( new Button( ));
    m_button_perfedit->add( *manage( new Image(
                    Gdk::Pixbuf::create_from_xpm_data( perfedit_xpm  ))));
    m_button_perfedit->signal_clicked().connect(
            mem_fun( *this, &mainwnd::open_performance_edit ));
    add_tooltip( m_button_perfedit, "Show or hide song editor window" );
    hbox->pack_end(*m_button_perfedit, false, false, 4);

    /* bpm spin button */
    m_adjust_bpm = manage(new Adjustment(m_mainperf->get_bpm(), 20, 500, 1));
    m_spinbutton_bpm = manage( new SpinButton( *m_adjust_bpm ));
    m_spinbutton_bpm->set_editable( false );
    m_adjust_bpm->signal_value_changed().connect(
            mem_fun(*this, &mainwnd::adj_callback_bpm ));
    add_tooltip( m_spinbutton_bpm, "Adjust beats per minute (BPM) value" );
    hbox->pack_start(*(manage( new Label( "  bpm " ))), false, false, 4);
    hbox->pack_start(*m_spinbutton_bpm, false, false );
  
    /* sequence set spin button */
    m_adjust_ss = manage( new Adjustment( 0, 0, c_max_sets - 1, 1 ));
    m_spinbutton_ss = manage( new SpinButton( *m_adjust_ss ));
    m_spinbutton_ss->set_editable( false );
    m_spinbutton_ss->set_wrap( true );
    m_adjust_ss->signal_value_changed().connect(
            mem_fun(*this, &mainwnd::adj_callback_ss ));
    add_tooltip( m_spinbutton_ss, "Select sreen set" );
    hbox->pack_end(*m_spinbutton_ss, false, false );
    hbox->pack_end(*(manage( new Label( "  set " ))), false, false, 4);
 
    /* screen set name edit line */
    m_entry_notes = manage( new Entry());
    m_entry_notes->signal_changed().connect(
            mem_fun(*this, &mainwnd::edit_callback_notepad));
    m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                m_mainperf->get_screenset())); 
    add_tooltip( m_entry_notes, "Enter screen set name" );
    hbox->pack_start( *m_entry_notes, true, true );


    /* top line items */
    HBox *hbox2 = manage( new HBox( false, 0 ) );
    hbox2->pack_start(*manage(new Image(
                    Gdk::Pixbuf::create_from_xpm_data(seq24_xpm))),
            false, false);

    // adjust placement...
    VBox *vbox_b = manage( new VBox() );
    HBox *hbox3 = manage( new HBox( false, 0 ) );
    vbox_b->pack_start( *hbox3, false, false );
    hbox2->pack_end( *vbox_b, false, false );
    hbox3->set_spacing( 10 );

    /* timeline */
    hbox3->pack_start( *m_main_time, false, false );

    /* group learn button */
    m_button_learn = manage( new Button( ));
    m_button_learn->set_focus_on_click( false );
    m_button_learn->set_flags( m_button_learn->get_flags() & ~Gtk::CAN_FOCUS );
    m_button_learn->set_image(*manage(new Image(
                    Gdk::Pixbuf::create_from_xpm_data( learn_xpm ))));
    m_button_learn->signal_clicked().connect(
            mem_fun(*this, &mainwnd::learn_toggle));
    add_tooltip( m_button_learn, "Mute Group Learn\n\n"
            "Click 'L' then press a mutegroup key to store the mute state of "
            "the sequences in that key.\n\n"
            "(see File/Options/Keyboard for available mutegroup keys "
            "and the corresponding hotkey for the 'L' button)" );
    hbox3->pack_end( *m_button_learn, false, false );

    Button w;
    hbox3->set_focus_child( w ); // clear the focus, don't want to trigger L via keys

    /* set up a vbox, put the menu in it, and add it */
    VBox *vbox = new VBox();
    vbox->set_border_width( 10 );
    vbox->pack_start(*hbox2, false, false );
    vbox->pack_start(*m_main_wid, true, true, 10 );
    vbox->pack_start(*hbox, false, false ); 
 

    VBox *ovbox = new VBox();
 
    ovbox->pack_start(*m_menubar, false, false );
    ovbox->pack_start( *vbox );

    /* add box */
    this->add (*ovbox);
  
    /* show everything */
    show_all();

    add_events( Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );

    m_timeout_connect = Glib::signal_timeout().connect(
            mem_fun(*this, &mainwnd::timer_callback), 25);
    
    m_modified = false;

    m_perf_edit = new perfedit( m_mainperf );
    m_options = NULL;
}

 
mainwnd::~mainwnd()
{
    if ( m_perf_edit != NULL )
        delete m_perf_edit;
    if ( m_options != NULL )
        delete m_options;
}


// This is the GTK timer callback, used to draw our current time and bpm
// ondd_events( the main window
bool
mainwnd::timer_callback(  )
{
    long ticks = m_mainperf->get_tick();
	
    m_main_time->idle_progress( ticks );
    m_main_wid->update_markers( ticks );

    if ( m_adjust_bpm->get_value() != m_mainperf->get_bpm()){
        m_adjust_bpm->set_value( m_mainperf->get_bpm());
    }

    if ( m_adjust_ss->get_value() !=  m_mainperf->get_screenset() )
    {
        m_main_wid->set_screenset(m_mainperf->get_screenset());  
        m_adjust_ss->set_value( m_mainperf->get_screenset());	
        m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                    m_mainperf->get_screenset())); 
    }

    return true;
}


void 
mainwnd::open_performance_edit( void )
{
    if (m_perf_edit->is_visible())
        m_perf_edit->hide();
    else {
        m_perf_edit->init_before_show();
        m_perf_edit->show_all();
        m_modified = true;
    }
}


void 
mainwnd::options_dialog( void )
{
    if ( m_options != NULL )
        delete m_options;
    m_options = new options( *this,  m_mainperf ); 
    m_options->show_all(); 
}


void 
mainwnd::start_playing( void )
{
    m_mainperf->position_jack( false );  
    m_mainperf->start( false );
    m_mainperf->start_jack( );
    is_pattern_playing = true;
}


void 
mainwnd::stop_playing( void )
{
    m_mainperf->stop_jack();
    m_mainperf->stop();
    m_main_wid->update_sequences_on_window();
    is_pattern_playing = false;
}

void
mainwnd::on_grouplearnchange(bool state)
{
    /* respond to learn mode change from m_mainperf */
    m_button_learn->set_image(*manage(new Image(
        Gdk::Pixbuf::create_from_xpm_data( state ? learn2_xpm : learn_xpm))));
}

void
mainwnd::learn_toggle()
{
    if (m_mainperf->is_group_learning())
    {
        m_mainperf->unset_mode_group_learn();
    }
    else
    {
        m_mainperf->set_mode_group_learn();
    }
}



/* callback function */
void mainwnd::file_new()
{
    if (is_save())
        new_file();
}


void mainwnd::new_file()
{
    m_mainperf->clear_all();

    m_main_wid->reset();
    m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(
                m_mainperf->get_screenset() ));

    global_filename = "";
    update_window_title();
    m_modified = false;
}


/* callback function */
void mainwnd::file_save()
{
    save_file();
}


/* callback function */
void mainwnd::file_save_as()
{
    Gtk::FileChooserDialog dialog("Save file as",
                      Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dialog.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);
    int result = dialog.run();
    
    switch (result) {
        case Gtk::RESPONSE_OK:
        {
            bool result = false;
            
            std::string fname = dialog.get_filename();
            Gtk::FileFilter* current_filter = dialog.get_filter();

            if ((current_filter != NULL) &&
                    (current_filter->get_name() == "MIDI files")) {

                // check for MIDI file extension; if missing, add .midi
                std::string suffix = fname.substr(
                        fname.find_last_of(".") + 1, std::string::npos);
                toLower(suffix);
                if ((suffix != "midi") && (suffix != "mid"))
                    fname = fname + ".midi";
            }

            if (Glib::file_test(fname, Glib::FILE_TEST_EXISTS)) {
                Gtk::MessageDialog warning(*this,
                        "File already exists!\n"
                        "Do you want to overwrite it?",
                        false,
                        Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);
                result = warning.run();

                if (result == Gtk::RESPONSE_NO)
                    return;
            }
            global_filename = fname;
            update_window_title();
            save_file();
            break;
        }

        default:
            break;
    }
}


void mainwnd::open_file(const Glib::ustring& fn)
{
    bool result;

    m_mainperf->clear_all();

    midifile f(fn);
    result = f.parse(m_mainperf, 0);
    m_modified = !result;

    if (!result) {
        Gtk::MessageDialog errdialog(*this,
                "Error reading file: " + fn, false,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        errdialog.run();
        return;
    }

    last_used_dir = fn.substr(0, fn.rfind("/") + 1);
    global_filename = fn;
    update_window_title();

    m_main_wid->reset();
    m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                m_mainperf->get_screenset())); 
    m_adjust_bpm->set_value( m_mainperf->get_bpm());
}


/*callback function*/
void mainwnd::file_open()
{
    if (is_save())
        choose_file();
}


void mainwnd::choose_file()
{
    Gtk::FileChooserDialog dialog("Open MIDI file",
                      Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dialog.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);

    int result = dialog.run();
    
    switch(result) {
        case(Gtk::RESPONSE_OK):
            open_file(dialog.get_filename());

        default:
            break;
    }
}


bool mainwnd::save_file()
{
    bool result = false;

    if (global_filename == "") {
        file_save_as();
        return true;
    }

    midifile f(global_filename);
    result = f.write(m_mainperf);

    if (!result) {
        Gtk::MessageDialog errdialog(*this,
                "Error writing file.", false,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        errdialog.run();
    }
    m_modified = !result;
    return result;
}


int mainwnd::query_save_changes()
{
    Glib::ustring query_str;

    if (global_filename == "")
        query_str = "Unnamed file was changed.\nSave changes?";
    else
        query_str = "File '" + global_filename + "' was changed.\n"
                "Save changes?";

    Gtk::MessageDialog dialog(*this, query_str, false,
            Gtk::MESSAGE_QUESTION,
            Gtk::BUTTONS_NONE, true);

    dialog.add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
    dialog.add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    return dialog.run();
}


bool mainwnd::is_save()
{
    bool result = false;

    if (is_modified()) {
        int choice = query_save_changes();
        switch (choice) {
            case Gtk::RESPONSE_YES:
                if (save_file())
                    result = true;
                break;
            case Gtk::RESPONSE_NO:
                result = true;
                break;
            case Gtk::RESPONSE_CANCEL:
            default:
                break;
        }
    }
    else
        result = true;

    return result;
}


/* convert string to lower case letters */
void
mainwnd::toLower(basic_string<char>& s) {
    for (basic_string<char>::iterator p = s.begin();
            p != s.end(); p++) {
        *p = tolower(*p);
    }
}


void 
mainwnd::file_import_dialog( void )
{
    Gtk::FileChooserDialog dialog("Import MIDI file",
            Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dialog.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);

    ButtonBox *btnbox = dialog.get_action_area(); 
    HBox hbox( false, 2 );

    m_adjust_load_offset = manage( new Adjustment( 0, -(c_max_sets - 1),
                c_max_sets - 1, 1 ));
    m_spinbutton_load_offset = manage( new SpinButton( *m_adjust_load_offset ));
    m_spinbutton_load_offset->set_editable( false );
    m_spinbutton_load_offset->set_wrap( true );
    hbox.pack_end(*m_spinbutton_load_offset, false, false );
    hbox.pack_end(*(manage( new Label("Screen Set Offset"))), false, false, 4);

    btnbox->pack_start(hbox, false, false );  

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    dialog.show_all_children();

    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
       case(Gtk::RESPONSE_OK):
       {
           try{
               midifile f( dialog.get_filename() );
               f.parse( m_mainperf, (int) m_adjust_load_offset->get_value() );
           }
           catch(...){
               Gtk::MessageDialog errdialog(*this, 
                       "Error reading file.", false,
                       Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                errdialog.run();
           }

           global_filename = std::string(dialog.get_filename());
           update_window_title();
           m_modified = true;
           
           m_main_wid->reset();
           m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                       m_mainperf->get_screenset() )); 
           m_adjust_bpm->set_value( m_mainperf->get_bpm() );

           break;
       }

       case(Gtk::RESPONSE_CANCEL):
           break;

       default:
           break;

   }
}

/*callback function*/
void mainwnd::file_exit()
{
    if (is_save()) {
        if (is_pattern_playing)
            stop_playing();
        hide();
    }
}


bool
mainwnd::on_delete_event(GdkEventAny *a_e)
{
    bool result = is_save();
    if (result && is_pattern_playing)
            stop_playing();

    return !result;
}


void 
mainwnd::about_dialog( void )
{
    Gtk::AboutDialog dialog;
    dialog.set_transient_for(*this);
    dialog.set_name(PACKAGE_NAME);
    dialog.set_version(VERSION);
    dialog.set_comments("Interactive MIDI Sequencer\n");

    dialog.set_copyright(
            "(C) 2002 - 2006 Rob C. Buse\n"
            "(C) 2008 - 2009 Seq24team");
    
    dialog.set_website(
            "http://www.filter24.org/seq24\n"
            "http://edge.launchpad.net/seq24");

    std::list<Glib::ustring> list_authors;
    list_authors.push_back("Rob C. Buse <rcb@filter24.org>");
    list_authors.push_back("Ivan Hernandez <ihernandez@kiusys.com>");
    list_authors.push_back("Guido Scholz <guido.scholz@bayernline.de>");
    list_authors.push_back("Jaakko Sipari <jaakko.sipari@gmail.com>");
    list_authors.push_back("Peter Leigh <pete.leigh@gmail.com>");
    list_authors.push_back("Anthony Green <green@redhat.com>");
    list_authors.push_back("Daniel Ellis <mail@danellis.co.uk>");
    list_authors.push_back("Kevin Meinert <kevin@subatomicglue.com>");
    dialog.set_authors(list_authors);

    std::list<Glib::ustring> list_documenters;
    list_documenters.push_back("Dana Olson <seq24@ubuntustudio.com>");
    dialog.set_documenters(list_documenters);

    dialog.show_all_children();
    dialog.run(); 
}


void 
mainwnd::adj_callback_ss( )
{
    m_mainperf->set_screenset( (int) m_adjust_ss->get_value()); 
    m_main_wid->set_screenset( m_mainperf->get_screenset());
    m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                m_mainperf->get_screenset()));
    m_modified = true;
}


void 
mainwnd::adj_callback_bpm( )
{
    m_mainperf->set_bpm( (int) m_adjust_bpm->get_value()); 
    m_modified = true;
}


bool
mainwnd::on_key_release_event(GdkEventKey* a_ev)
{
    if ( a_ev->keyval == m_mainperf->m_key_replace )
        m_mainperf->unset_sequence_control_status( c_status_replace );

    if (a_ev->keyval == m_mainperf->m_key_queue )
        m_mainperf->unset_sequence_control_status( c_status_queue );

    if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
            a_ev->keyval == m_mainperf->m_key_snapshot_2 )
        m_mainperf->unset_sequence_control_status( c_status_snapshot );
    if ( a_ev->keyval == m_mainperf->m_key_group_learn ){
        m_mainperf->unset_mode_group_learn();
    }

    return false;
}


void
mainwnd::edit_callback_notepad( )
{
    string text = m_entry_notes->get_text();
    m_mainperf->set_screen_set_notepad( m_mainperf->get_screenset(), 
				        &text ); 
    m_modified = true;
}


bool
mainwnd::on_key_press_event(GdkEventKey* a_ev)
{
    // control and modifier key combinations matching
    // menu items have first priority
    /*if (*/Gtk::Window::on_key_press_event(a_ev);
        //return true;  // on win32, it'd always return true here (i.e. for SPACE bar)... ?

    /*else */if ( m_entry_notes->has_focus()) {
        m_entry_notes->event( (GdkEvent*) a_ev );
        return false;
    }
    else {
        if ( a_ev->type == GDK_KEY_PRESS ){

            if ( global_print_keys ){
                printf( "key_press[%d]\n", a_ev->keyval );
                fflush( stdout );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_bpm_dn ){
                m_mainperf->set_bpm( m_mainperf->get_bpm() - 1 );  
                m_adjust_bpm->set_value(  m_mainperf->get_bpm() );
            }
            
            if ( a_ev->keyval ==  m_mainperf->m_key_bpm_up ){
                m_mainperf->set_bpm( m_mainperf->get_bpm() + 1 );   
                m_adjust_bpm->set_value(  m_mainperf->get_bpm() );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_replace )
            {
                m_mainperf->set_sequence_control_status( c_status_replace );
            }
            
              if ((a_ev->keyval ==  m_mainperf->m_key_queue )
             || (a_ev->keyval == m_mainperf->m_key_keep_queue ))
            {
                m_mainperf->set_sequence_control_status( c_status_queue );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
                 a_ev->keyval == m_mainperf->m_key_snapshot_2 )
            {
                m_mainperf->set_sequence_control_status( c_status_snapshot );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_screenset_dn ){

                m_mainperf->set_screenset(  m_mainperf->get_screenset() - 1 );  
                m_main_wid->set_screenset(  m_mainperf->get_screenset() );  
                m_adjust_ss->set_value( m_mainperf->get_screenset()  );	
                m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()  )); 
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_screenset_up ){

                m_mainperf->set_screenset(  m_mainperf->get_screenset() + 1 );  
                m_main_wid->set_screenset(  m_mainperf->get_screenset() );  
                m_adjust_ss->set_value( m_mainperf->get_screenset()  );	
                m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()  )); 
            }
             if ( a_ev->keyval == m_mainperf->m_key_set_playing_screenset ){
                m_mainperf->set_playing_screenset();
            }
           
            if ( a_ev->keyval == m_mainperf->m_key_group_on ){
                m_mainperf->set_mode_group_mute();
            }
            if ( a_ev->keyval == m_mainperf->m_key_group_off ){
                m_mainperf->unset_mode_group_mute();
            }
           
            if ( a_ev->keyval == m_mainperf->m_key_group_learn ){
                m_mainperf->set_mode_group_learn();
            }

            // activate mute group key
            if( m_mainperf->get_key_groups()->count( a_ev->keyval ) != 0 )
            {
                m_mainperf->select_and_mute_group(
                        m_mainperf->lookup_keygroup_group(a_ev->keyval));
            }

            // mute group learn
            if (m_mainperf->is_learn_mode() &&
                a_ev->keyval != m_mainperf->m_key_group_learn)
            {
                if( m_mainperf->get_key_groups()->count( a_ev->keyval ) != 0 )
                {
                    std::ostringstream os;
                    os << "Key \""
                       << gdk_keyval_name(a_ev->keyval)
                       << "\" (code = "
                       << a_ev->keyval
                       << ") successfully mapped.";

                    Gtk::MessageDialog dialog(*this,
                           "MIDI mute group learn success", false,
                           Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                    dialog.set_secondary_text(os.str(), false);
                    dialog.run();
                    
                    // we miss the keyup msg for learn, force set it off
                    m_mainperf->unset_mode_group_learn();
                }
                else
                {
                    std::ostringstream os;
                    os << "Key \""
                        << gdk_keyval_name(a_ev->keyval)
                        << "\" (code = "
                        << a_ev->keyval
                        << ") is not one of the configured mute-group keys.\n"
                        << "To change this see File/Options menu or .seq24rc";

                    Gtk::MessageDialog dialog(*this,
                           "MIDI mute group learn failed", false,
                           Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);

                    dialog.set_secondary_text(os.str(), false);
                    dialog.run();
                    // we miss the keyup msg for learn, force set it
                    m_mainperf->unset_mode_group_learn();
                }
            }

            // the start/end key may be the same key (i.e. SPACE)
            // allow toggling when the same key is mapped to both
            // triggers (i.e. SPACEBAR)
            bool dont_toggle = m_mainperf->m_key_start
                != m_mainperf->m_key_stop;

            if ( a_ev->keyval == m_mainperf->m_key_start
                    && (dont_toggle || !is_pattern_playing))
            {
                start_playing();
            }
            else if ( a_ev->keyval == m_mainperf->m_key_stop
                    && (dont_toggle || is_pattern_playing))
            {
                stop_playing();
            }

            /* toggle sequence mute/unmute using keyboard keys... */
            if (m_mainperf->get_key_events()->count( a_ev->keyval) != 0)
            {
                sequence_key(m_mainperf->lookup_keyevent_seq( a_ev->keyval));
            }
        }
    }

    return false;
}


void 
mainwnd::sequence_key( int a_seq )
{
    int offset = m_mainperf->get_screenset() * c_mainwnd_rows * c_mainwnd_cols;
	
    if ( m_mainperf->is_active( a_seq + offset ) ){
		m_mainperf->sequence_playing_toggle( a_seq + offset );
    }
}


void
mainwnd::update_window_title()
{
    std::string title;

    if (global_filename == "")
        title = ( PACKAGE ) + string( " - [unnamed]" );
    else
        title =
            ( PACKAGE )
            + string( " - [" )
            + Glib::filename_to_utf8(global_filename)
            + string( "]" );
    
    set_title ( title.c_str());
}


bool mainwnd::is_modified()
{
    return m_modified;
}
