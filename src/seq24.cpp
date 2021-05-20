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
    {"jack-transport",0, 0, 'j'},
    {"osc-port", 1,0,'p'},
    {0, 0, 0, 0}

};

Glib::ustring global_filename = "";
Glib::ustring last_used_dir ="/";
std::string config_filename = ".seq24rc";
std::string user_filename = ".seq24usr";

bool global_with_jack_transport = false;
interaction_method_e global_interactionmethod = e_seq24_interaction;

char* global_oscport;

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
    perform * p = new perform();

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

        if ( !options.parse( p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());
        }

        total_file = home + SLASH + user_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        userfile user( total_file );

        if ( !user.parse( p ) ){
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
                printf( "  -h, --help : show this message\n" );
                printf( "  -f, --file <filename> : load midi file on startup\n" );
                printf( "  -p, --osc-port <port> : osc input port (udp port number or unix socket path)\n" );
                printf( "  -j, --jack-transport : sync to jack transport\n" );
                printf( "\n\n" );

                return 0;
                break;

            case 'j':
                global_with_jack_transport = true;
                break;

            case 'f':
                global_filename = Glib::ustring(optarg);
               break;

            case 'p':
                global_oscport = optarg;
                break;

            default:
                break;
        }

    }


    p->init();

    p->launch_input_thread();
    p->launch_output_thread();
    p->init_jack();

    if (global_filename != "") {
        /* import that midi file */
        midifile *f = new midifile(global_filename);
        f->parse( p, 0 );
        delete f;
    }

    mainwnd seq24_window( p );

    kit.run(seq24_window);

    if ( getenv( HOME ) != NULL ){

        string home( getenv( HOME ));
        Glib::ustring total_file = home + SLASH + config_filename;
        printf( "Writing [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.write( p ) ){
            printf( "Error writing [%s]\n", total_file.c_str());
        }

    } else {

        printf( "Error calling getenv( \"%s\" )\n", HOME );
    }

    delete p;

    return 0;
}
