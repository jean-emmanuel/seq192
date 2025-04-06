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


#ifndef SEQ192_GLOBALS
#define SEQ192_GLOBALS

#include <string>

using namespace std;

const int c_thread_trigger_us = 1000;

/* 182 per screen */
const int c_mainwnd_rows = 13; //ORL
const int c_mainwnd_cols = 14; //ORL
const int c_seqs_in_set = c_mainwnd_rows * c_mainwnd_cols;
const int c_gmute_tracks = c_seqs_in_set * c_seqs_in_set;
const int c_max_sets = 32;
const int c_total_seqs = c_seqs_in_set * c_max_sets;

/* number of sequences */
/* 32 screen sets */
const int c_max_sequence =  c_mainwnd_rows *  c_mainwnd_cols * c_max_sets;


const int c_ppqn            = 192;  /* default - dosnt change */
const int c_ppwn            = c_ppqn * 4;  // whole note
const int c_ppen            = c_ppqn / 2;  // eighth note
const int c_ppsn            = c_ppqn / 4;  // 16th note
const int c_min_note_length = c_ppqn / 16 / 3;

const double c_bpm                       = 120.0;   /* default */
const double c_bpm_scale_factor          = 1000.0;  /* used in midifile for doubles */
const double c_bpm_minimum               = 1.0;
const double c_bpm_maximum               = 600.0;

const int c_maxBuses = 32;

/* sequences */
const int c_note_off_margin = 1;  // # ticks to shave off end of painted notes
const int c_num_keys = 128;
const int c_midi_notes = 256;
const string c_dummy( "Untitled" );

/* maximum size of sequence, default size */
const int c_maxbeats     = 0xFFFF;   /* max number of beats in a sequence */


/* midifile tags */
const unsigned long c_midibus =    0x24240001;
const unsigned long c_midich =     0x24240002;
const unsigned long c_midiclocks = 0x24240003;
const unsigned long c_triggers =   0x24240004;
const unsigned long c_notes =      0x24240005;
const unsigned long c_timesig =    0x24240006;
const unsigned long c_bpmtag =     0x24240007;
const unsigned long c_triggers_new =   0x24240008;
const unsigned long c_midictrl =   0x24240010;
const unsigned long c_mutegroups = 0x24240009; // not sure why we went to 10 above, this might need a different value
const unsigned long c_resume = 0x24240011;
const unsigned long c_alt_cc = 0x24240012;
const unsigned long c_chase = 0x24240013;
const unsigned long c_snap_tick = 0x24240014;
const unsigned long c_note_tick = 0x24240015;

const int c_default_snap_tick = c_ppqn / 4;
const int c_default_note_tick = c_ppqn / 4;


extern string global_client_name;

extern bool global_with_jack_transport;
extern char* global_oscport;

extern bool global_is_modified;
extern bool global_is_running;

extern string global_filename;
extern string last_used_dir;
extern bool is_pattern_playing;

const int c_max_instruments = 256;

const int c_max_undo_history = 30;

struct user_midi_bus_definition
{
    std::string alias;
    int instrument[16];
    int keymap[16];
    int portamento_max_time;
    bool portamento_log_scale;
};

struct user_instrument_definition
{
    string instrument;
    string color;
    bool controllers_active[128];
    string controllers[128];
};

struct user_keymap_definition
{
    string keymap;
    bool keys_active[128];
    string keys[128];
};


extern user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
extern user_instrument_definition global_user_instrument_definitions[c_max_instruments];
extern user_keymap_definition     global_user_keymap_definitions[c_max_instruments];

enum file_type_e
{
    E_MIDI_SEQ192_SESSION,
    E_MIDI_SEQ192_SEQUENCE,
    E_MIDI_SEQ192_SCREENSET
};

extern bool global_nsm_gui;

#endif
