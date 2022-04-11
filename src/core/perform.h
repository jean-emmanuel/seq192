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


#ifndef SEQ192_PERFORM
#define SEQ192_PERFORM

class perform;

#include "globals.h"
#include "event.h"
#include "midibus.h"
#include "midifile.h"
#include "sequence.h"
#include "osc.h"
#include <unistd.h>
#include <pthread.h>

#include <jack/jack.h>
#include <jack/transport.h>


/* class contains sequences that make up a live set */
class perform
{
 private:

    /* vector of sequences */
    sequence *m_seqs[c_max_sequence];
    sequence m_clipboard;

    bool m_seqs_active[ c_max_sequence ];

    bool m_was_active_main[ c_max_sequence ];
    bool m_was_active_edit[ c_max_sequence ];
    bool m_was_active_perf[ c_max_sequence ];
    bool m_was_active_names[ c_max_sequence ];

    bool m_sequence_state[  c_max_sequence ];

    /* our midibus */
    mastermidibus m_master_bus;

    /* pthread info */
    pthread_t m_out_thread;
    pthread_t m_in_thread;
    bool m_out_thread_launched;
    bool m_in_thread_launched;

    bool m_running;
    bool m_inputing;
    bool m_outputing;
    bool m_looping;

    bool m_transport_stopping;

    long m_tick;

    void set_running( bool a_running );

    string m_screen_set_notepad[c_max_sets];

    int m_offset;
    int m_control_status;
    int m_screen_set;

    condition_var m_condition_var;

    // do not access these directly, use set/lookup below
    std::map<unsigned int,long> key_events;
    std::map<unsigned int,long> key_groups;
    std::map<long,unsigned int> key_events_rev; // reverse lookup, keep this in sync!!
    std::map<long,unsigned int> key_groups_rev; // reverse lookup, keep this in sync!!


    jack_client_t *m_jack_client;
    bool m_jack_running;

    void inner_start();
    void inner_stop();

 public:
    bool is_running();

    unsigned int m_key_bpm_up;
    unsigned int m_key_bpm_dn;

    unsigned int m_key_replace;
    unsigned int m_key_queue;
    unsigned int m_key_keep_queue;
    unsigned int m_key_snapshot_1;
    unsigned int m_key_snapshot_2;

    unsigned int m_key_screenset_up;
    unsigned int m_key_screenset_dn;
    unsigned int m_key_set_playing_screenset;

    unsigned int m_key_group_on;
    unsigned int m_key_group_off;
    unsigned int m_key_group_learn;

    unsigned int m_key_screenset[c_max_sets];

    unsigned int m_key_start;
    unsigned int m_key_stop;

    perform();
    ~perform();

    void init();

    void clear_all();

    void start_playing();
    void stop_playing();
    void panic();

    void launch_input_thread();
    void launch_output_thread();
    void init_jack();
    void deinit_jack();

    void add_sequence( sequence *a_seq, int a_perf );
    void delete_sequence( int a_num );
    void copy_sequence( int a_num );
    void paste_sequence( int a_num );
    void cut_sequence( int a_num );
    void move_sequence( int a_from, int a_to );

    long get_tick( ) { return m_tick; };

    void print();

    void set_screen_set_notepad( int a_screen_set, string *a_note );
    string *get_screen_set_notepad( int a_screen_set );

    void set_screenset( int a_ss );
    int get_screenset();

    void start();
    void stop();

    void start_jack();
    void stop_jack();
    void position_jack();

    void off_sequences();
    void all_notes_off();

    void set_active(int a_sequence, bool a_active);
    void set_was_active( int a_sequence );
    bool is_active(int a_sequence);
    bool is_dirty_main (int a_sequence);
    bool is_dirty_edit (int a_sequence);

    void new_sequence( int a_sequence );

    /* plays all notes to Curent tick */
    void play( long a_tick );
    void set_orig_ticks( long a_tick  );

    sequence * get_sequence( int a_sequence );

    void reset_sequences();

    void set_bpm(double a_bpm);
    double  get_bpm( );

    mastermidibus* get_master_midi_bus( );

    void output_func();
    void input_func();

    void save_playing_state();
    void restore_playing_state();

    OSCServer *oscserver;
    static int osc_callback(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

    int osc_selected_seqs[c_mainwnd_rows * c_mainwnd_cols];
    void osc_status( char* address, const char* path );
    enum OSC_COMMANDS {
        OSC_ZERO = 0,
        SEQ_PLAY,
        SEQ_STOP,
        SEQ_BPM,
        SEQ_SSET,
        SEQ_PANIC,
        SEQ_SSEQ,
        SEQ_SSEQ_AND_PLAY,
        SEQ_SSEQ_QUEUED,
        SEQ_STATUS,
        SEQ_STATUS_EXT,
        SEQ_CLEAR,              // Test clear notes in clip

        SEQ_MODE_SOLO,
        SEQ_MODE_ON,
        SEQ_MODE_OFF,
        SEQ_MODE_TOGGLE,
        SEQ_MODE_RECORD,
        SEQ_MODE_RECORD_ON,
        SEQ_MODE_RECORD_OFF
    };

    std::map<std::string, int> osc_commands = {
        {"/play",               SEQ_PLAY},
        {"/stop",               SEQ_STOP},
        {"/bpm",                SEQ_BPM},
        {"/screenset",          SEQ_SSET},
        {"/panic",              SEQ_PANIC},
        {"/sequence",           SEQ_SSEQ},
        {"/sequence/trig",      SEQ_SSEQ_AND_PLAY},
        {"/sequence/queue",     SEQ_SSEQ_QUEUED},
        {"/status",             SEQ_STATUS},
        {"/status/extended",    SEQ_STATUS_EXT}
    };

    std::map<std::string, int> osc_seq_modes = {
        {"solo",                SEQ_MODE_SOLO},
        {"on",                  SEQ_MODE_ON},
        {"off",                 SEQ_MODE_OFF},
        {"toggle",              SEQ_MODE_TOGGLE},
        {"record",              SEQ_MODE_RECORD},
        {"record_on",           SEQ_MODE_RECORD_ON},
        {"record_off",          SEQ_MODE_RECORD_OFF},
        {"clear",               SEQ_CLEAR}
    };


    friend class mainwid;
    friend class midifile;
    friend class optionsfile;
    friend class options;

    friend int jack_process_callback(jack_nframes_t nframes, void* arg);
    friend void jack_shutdown(void *arg);
};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);


int jack_process_callback(jack_nframes_t nframes, void* arg);
void jack_shutdown(void *arg);

#endif
