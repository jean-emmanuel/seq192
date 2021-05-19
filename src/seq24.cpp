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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "font.h"
#include "mainwnd.h"
#include "midifile.h"
#include "optionsfile.h"
#include "perform.h"
#include "userfile.h"

/* struct for command parsing */
static struct
option long_options[] = {

    {"file",     required_argument, 0, 'f'},
    {"help",     0, 0, 'h'},
    {"showmidi",     0, 0, 's'},
    {"show_keys",     0, 0, 'k' },
    {"stats",     0, 0, 'S' },
    {"ignore",required_argument, 0, 'i'},
    {"interaction_method",required_argument, 0, 'x'},
    {"jack_transport",0, 0, 'j'},
    {"show_keys", 0,0,'k'},
    {"osc_port", 1,0,'o'},
    {"key_size", 1,0,'o'},
    {0, 0, 0, 0}

};

bool global_showmidi = false;
bool global_device_ignore = false;
int global_device_ignore_num = 0;
bool global_stats = false;
Glib::ustring global_filename = "";
Glib::ustring last_used_dir ="/";
std::string config_filename = ".seq24rc";
std::string user_filename = ".seq24usr";
bool global_print_keys = false;
interaction_method_e global_interactionmethod = e_seq24_interaction;

bool global_with_jack_transport = false;

int global_oscport = 0;

user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];

font *p_font_renderer;

#define HOME "HOME"
#define SLASH "/"

int
main (int argc, char *argv[])
{
    for ( int i=0; i<c_maxBuses; i++ )
    {
        for ( int j=0; j<16; j++ )
            global_user_midi_bus_definitions[i].instrument[j] = -1;
    }

    for ( int i=0; i<c_max_instruments; i++ )
    {
        for ( int j=0; j<128; j++ )
            global_user_instrument_definitions[i].controllers_active[j] = false;
    }

    /* the main performance object */
    perform p;

    /* all GTK applications must have a gtk_main(). Control ends here
       and waits for an event to occur (like a key press or mouse event). */
    Gtk::Main kit(argc, argv);

    p_font_renderer = new font();


    if ( getenv( HOME ) != NULL ){

        Glib::ustring home( getenv( HOME ));
        last_used_dir = home;
        Glib::ustring total_file = home + SLASH + config_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());
        }

        total_file = home + SLASH + user_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        userfile user( total_file );

        if ( !user.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());
        }

    } else {

        printf( "Error calling getenv( \"%s\" )\n", HOME );
    }



    /* parse parameters */
    int c;


    while (1){

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:f:v", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c){

            case '?':
            case 'h':

                printf( "usage: seq24 [options]\n\n" );
                printf( "options:\n" );
                printf( "    --help : show this message\n" );
                printf( "    --file <filename> : load midi file on startup\n" );
                printf( "    --showmidi : dumps incoming midi to screen\n" );
                printf( "    --show_keys : prints pressed key value\n" );
                printf( "    --interaction_method <number>: see .seq24rc for methods to use\n" );
                printf( "    --jack_transport : seq24 will sync to jack transport\n" );
                printf( "    --osc_port : osc input port\n" );
                printf( "\n\n\n" );

                return 0;
                break;

            case 'S':
                global_stats = true;
                break;

            case 's':
                global_showmidi = true;
                break;

            case 'k':
                global_print_keys = true;
                break;

            case 'j':
                global_with_jack_transport = true;
                break;

            case 'f':
                global_filename = Glib::ustring(optarg);
               break;

            case 'i':
                /* ignore alsa device */
                global_device_ignore = true;
                global_device_ignore_num = atoi( optarg );
                break;

            case 'x':
                global_interactionmethod = (interaction_method_e)atoi( optarg );
                break;

            case 'o':
                global_oscport = atoi( optarg );
                break;

            default:
                break;
        }

    } /* end while */


    p.init();

    p.launch_input_thread();
    p.launch_output_thread();
    p.init_jack();

    if (global_filename != "") {
        /* import that midi file */
        midifile *f = new midifile(global_filename);
        f->parse( &p, 0 );
        delete f;
    }

    mainwnd seq24_window( &p );

    kit.run(seq24_window);

    p.deinit_jack();

    if (global_oscport != 0) {
        p.oscserver->stop();
    }

    if ( getenv( HOME ) != NULL ){

        string home( getenv( HOME ));
        Glib::ustring total_file = home + SLASH + config_filename;
        printf( "Writing [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.write( &p ) ){
            printf( "Error writing [%s]\n", total_file.c_str());
        }

    } else {

        printf( "Error calling getenv( \"%s\" )\n", HOME );
    }

    return 0;
}
