// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>

#include "package.h"

#include "lib/nsm.h"

#include "core/midifile.h"
#include "core/configfile.h"
#include "core/cachefile.h"
#include "core/perform.h"

#ifdef USE_GTK
#include "gui/mainwindow.h"
#include <gtkmm.h>
#endif

/* struct for command parsing */
static struct
option long_options[] = {

    {"file",     required_argument, 0, 'f'},
    {"config", 1,0,'c'},
    {"help",     0, 0, 'h'},
    {"osc-port", 1,0,'p'},
    {"jack-transport",0, 0, 'j'},
    {"no-gui",0, 0, 'n'},
    {"version",0, 0, 'v'},
    {0, 0, 0, 0}

};

string config_filename = "";
string global_filename = "";
string last_used_dir = getenv("HOME");

#ifndef USE_GTK
bool global_no_gui = true;
#else
bool global_no_gui = false;
#endif

bool global_with_jack_transport = false;

bool global_is_running = true;

char* global_oscport;

string global_client_name = PACKAGE;

user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];
user_keymap_definition     global_user_keymap_definitions[c_max_instruments];

#ifdef USE_GTK
color global_user_instrument_colors[c_max_instruments];
Glib::RefPtr<Gtk::Application> application;
#endif

// nsm
bool global_nsm_gui = false;
bool nsm_optional_gui_support = true;
nsm_client_t *nsm = 0;
bool nsm_wait = true;
string nsm_folder = "";
int
nsm_save_cb(char **,  void *userdata)
{
    perform *p = (perform *) userdata;
    if (p->file_save()) {
        return ERR_OK;
    } else {
        return ERR_GENERAL;
    }
}
void
nsm_hide_cb(void *userdata)
{
    #ifdef USE_GTK
    application->hold();
    #endif
    global_nsm_gui = false;
}
void
nsm_show_cb(void *userdata)
{
    global_nsm_gui = true;
}
int
nsm_open_cb(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata)
{
    nsm_wait = false;
    nsm_folder = name;
    global_client_name = client_id;
    // NSM API 1.1.0: check if server supports optional-gui
    nsm_optional_gui_support = strstr(nsm_get_session_manager_features(nsm), "optional-gui");
    mkdir(nsm_folder.c_str(), 0777);
    // make sure nsm server doesn't override cached visibility state
    nsm_send_is_shown(nsm);
    return ERR_OK;
}


int
main (int argc, char *argv[])
{

    for (int i=0; i<c_maxBuses; i++)
    {
        for (int j=0; j<16; j++) {
            global_user_midi_bus_definitions[i].instrument[j] = -1;
            global_user_midi_bus_definitions[i].keymap[j] = -1;
        }
    }

    for (int i=0; i<c_max_instruments; i++)
    {
        for (int j=0; j<128; j++)
            global_user_instrument_definitions[i].controllers_active[j] = false;
    }

    for (int i=0; i<c_max_instruments; i++)
    {
        for (int j=0; j<128; j++)
            global_user_keymap_definitions[i].keys_active[j] = false;
    }

    /* parse parameters */
    int c;
    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:f:c:hjnv", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {

            case '?':
            case 'h':

                printf("\n");
                printf("%s - live MIDI sequencer (version %s)", PACKAGE, VERSION);

                #if !defined USE_JACK && !defined USE_GTK
                printf("(compiled without jack support and gtk support)");
                #elif !defined USE_JACK
                printf("(compiled without jack support)");
                #elif !defined USE_GTK
                printf("(compiled without gtk support)");
                #endif

                printf("\n\nUsage: %s [options]\n\n", PACKAGE);
                printf("Options:\n");
                printf("  -h, --help              show available options\n");
                printf("  -f, --file <filename>   load midi file on startup\n");
                printf("  -c, --config <filename> load config file on startup\n");
                printf("  -p, --osc-port <port>   osc input port (udp port number or unix socket path)\n");
                #ifdef USE_JACK
                printf("  -j, --jack-transport    sync to jack transport\n");
                #endif
                #ifdef USE_GTK
                printf("  -n, --no-gui            enable headless mode\n");
                #endif
                printf("  -v, --version           show version and exit\n");
                printf("\n");

                return 0;
                break;

            case 'j':
                global_with_jack_transport = true;
                break;

            case 'n':
                global_no_gui = true;
                break;

            case 'f':
                global_filename = string(optarg);
                break;

            case 'c':
                config_filename = string(optarg);
                break;

            case 'p':
                global_oscport = optarg;
                break;

            case 'v':
                printf("%s %s\n", PACKAGE, VERSION);
                return EXIT_SUCCESS;
                break;

            default:
                break;
        }

    }

    // nsm
    const char *nsm_url = getenv( "NSM_URL" );
    if (nsm_url) {
        nsm = nsm_new();
        nsm_set_open_callback(nsm, nsm_open_cb, 0);
        if (nsm_init(nsm, nsm_url) == 0) {
            if (!global_no_gui) nsm_send_announce(nsm, PACKAGE, ":optional-gui:dirty:", argv[0]);
            else nsm_send_announce(nsm, PACKAGE, ":dirty:", argv[0]);
        }
        int timeout = 0;
        while (nsm_wait) {
            nsm_check_wait(nsm, 500);
            timeout += 1;
            if (timeout > 200) exit(1);
        }
    }

    /* the main performance object */
    perform * p = new perform();

    // read config file
    string config_path = getenv("XDG_CONFIG_HOME") == NULL ? string(getenv("HOME")) + "/.config" : getenv("XDG_CONFIG_HOME");
    config_path += string("/") + PACKAGE;
    mkdir(config_path.c_str(), 0777);
    if (nsm) config_path = nsm_folder;
    string file_path = config_filename == "" ? (config_path + "/config.json") : config_filename;
    std::ifstream infile(file_path);
    if (!infile.good()) {
        std::fstream fs;
        fs.open(file_path, std::ios::out);
        fs << "{}" << endl;
        fs.close();
    }
    ConfigFile config(file_path);
    config.parse();

    // read/touch cache file
    string cache_path = getenv("XDG_CACHE_HOME") == NULL ? string(getenv("HOME")) + "/.cache" : getenv("XDG_CACHE_HOME");
    cache_path += string("/") + PACKAGE;
    mkdir(cache_path.c_str(), 0777);
    if (nsm) cache_path = nsm_folder;
    file_path = cache_path + "/cache.json";
    infile = std::ifstream(file_path);
    if (!infile.good()) {
        std::fstream fs;
        fs.open(file_path, std::ios::out);
        fs << "{}" << endl;
        fs.close();
    }
    CacheFile cache(file_path);
    cache.parse();

    p->init();

    p->launch_input_thread();
    p->launch_output_thread();

    #ifdef USE_JACK
    p->init_jack();
    #endif

    if (nsm) {
        global_filename = nsm_folder + "/session.midi";
        // write session file if it doesn't exist
        std::ifstream infile(global_filename);
        if (!infile.good()) {
            midifile f(global_filename);
            f.write(p, -1, -1);
        }
        nsm_set_save_callback(nsm, nsm_save_cb, (void*) p);
    }

    if (global_filename != "") {
        midifile *f = new midifile(global_filename);
        f->parse(p, 0);
        delete f;
    }

    if (global_oscport != 0 && strstr(global_oscport, "/")) {
        // liblo needs a gracefull ctrl+c handler to release unix socket
        signal(SIGINT, [](int param){global_is_running = false;});
    }

    int status = 0;
    if (global_no_gui) {
        while (global_is_running) {
            if (nsm) nsm_check_nowait(nsm);
            usleep(10000);
        }
    } else {
        #ifdef USE_GTK
        application = Gtk::Application::create();
        MainWindow window(p, application);

        if (nsm) {
            // setup optional-gui
            if (nsm_optional_gui_support) {
                nsm_set_show_callback(nsm, nsm_show_cb, 0);
                nsm_set_hide_callback(nsm, nsm_hide_cb, 0);
                if (!global_nsm_gui) nsm_hide_cb(0);
                else nsm_send_is_shown(nsm);
            } else {
                global_nsm_gui = true;
            }
            // enable nsm in window
            window.nsm_set_client(nsm, nsm_optional_gui_support);
            // bind quit signal
            signal(SIGTERM, [](int param){
                global_is_running = false;
                application->quit();
            });
        }

        // bind ctrl+c signal
        signal(SIGINT, [](int param){
            global_is_running = false;
            application->quit();
        });


        status = application->run(window);
        #endif
    }

    // write cache file
    cache.write();

    delete p;

    if (nsm) {
        nsm_free(nsm);
        nsm = NULL;
    }

    return status;
}
