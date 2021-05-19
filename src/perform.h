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

#ifndef SEQ24_PERFORM
#define SEQ24_PERFORM

class perform;

#include "globals.h"
#include "event.h"
#include "midibus.h"
#include "midifile.h"
#include "sequence.h"
#include "oscserver.h"
#include <unistd.h>
#include <pthread.h>


/* if we have jack, include the jack headers */
#ifdef JACK_SUPPORT
#include <jack/jack.h>
#include <jack/transport.h>
#endif


/* class contains sequences that make up a live set */
class perform
{
 private:
    //andy mute group
    bool m_mute_group[c_gmute_tracks];
    bool m_tracks_mute_state[c_seqs_in_set];
    bool m_mode_group;
    bool m_mode_group_learn;
    int m_mute_group_selected;
    //andy playing screen
    int m_playing_screen;


    /* vector of sequences */
    sequence *m_seqs[c_max_sequence];

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

    int thread_trigger_width_ms;

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


#ifdef JACK_SUPPORT

    jack_client_t *m_jack_client;
    jack_nframes_t m_jack_frame_current,
                   m_jack_frame_last;
    jack_position_t m_jack_pos;
    jack_transport_state_t m_jack_transport_state;
    jack_transport_state_t m_jack_transport_state_last;
    double m_jack_tick;

#endif

    bool m_jack_running;

    void inner_start();
    void inner_stop();

 public:
    bool is_running();
    bool is_learn_mode() const { return m_mode_group_learn; }

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

    void launch_input_thread();
    void launch_output_thread();
    void init_jack();
    void deinit_jack();

    void add_sequence( sequence *a_seq, int a_perf );
    void delete_sequence( int a_num );
    bool is_sequence_in_edit( int a_num );

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
    bool is_dirty_perf (int a_sequence);
    bool is_dirty_names (int a_sequence);

    void new_sequence( int a_sequence );

    /* plays all notes to Curent tick */
    void play( long a_tick );
    void set_orig_ticks( long a_tick  );

    sequence * get_sequence( int a_sequence );

    void reset_sequences();

    void set_bpm(int a_bpm);
    int  get_bpm( );

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
        SEQ_SSET,
        SEQ_PANIC,
        SEQ_SSEQ,
        SEQ_SSEQ_AND_PLAY,
        SEQ_SSEQ_QUEUED,
        SEQ_STATUS,
        SEQ_STATUS_EXT,

        SEQ_MODE_SOLO,
        SEQ_MODE_ON,
        SEQ_MODE_OFF,
        SEQ_MODE_TOGGLE
    };

    std::map<std::string, int> osc_commands = {
        {"/play",               SEQ_PLAY},
        {"/stop",               SEQ_STOP},
        {"/screenset",          SEQ_SSET},
        {"/panic",              SEQ_PANIC},
        {"/sequence",           SEQ_SSEQ},
        {"/sequence_and_play",  SEQ_SSEQ_AND_PLAY},
        {"/sequence_queued",    SEQ_SSEQ_QUEUED},
        {"/status",             SEQ_STATUS},
        {"/status/extended",    SEQ_STATUS_EXT}
    };

    std::map<std::string, int> osc_seq_modes = {
        {"solo",                SEQ_MODE_SOLO},
        {"on",                  SEQ_MODE_ON},
        {"off",                 SEQ_MODE_OFF},
        {"toggle",              SEQ_MODE_TOGGLE}
    };


    friend class mainwid;
    friend class midifile;
    friend class optionsfile;
    friend class options;

#ifdef JACK_SUPPORT

    friend int jack_sync_callback(jack_transport_state_t state,
                              jack_position_t *pos, void *arg);
    friend void jack_shutdown(void *arg);
    friend void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes,
                                       jack_position_t *pos, int new_pos, void *arg);
#endif
};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);



#ifdef JACK_SUPPORT

int jack_sync_callback(jack_transport_state_t state,
					   jack_position_t *pos, void *arg);
void print_jack_pos( jack_position_t* jack_pos );
void jack_shutdown(void *arg);
void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes,
                            jack_position_t *pos, int new_pos, void *arg);
#endif


#endif
