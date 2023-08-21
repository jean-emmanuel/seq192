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
#include <map>

#ifdef USE_JACK
#include <jack/jack.h>
#include <jack/transport.h>
#endif


typedef std::map<int, sequence> SequenceMap;
struct state {
    string notepad[c_max_sets];
    SequenceMap sequence_map;
};

/* class contains sequences that make up a live set */
class perform
{
 private:

    /* vector of sequences */
    sequence *m_seqs[c_max_sequence];
    sequence m_clipboard;

    bool m_sequence_state[  c_max_sequence ];

    /* our midibus */
    mastermidibus m_master_bus;

    /* pthread info */
    pthread_t m_out_thread;
    pthread_t m_in_thread;
    bool m_out_thread_launched;
    bool m_in_thread_launched;

    bool m_running;
    bool m_stopping;
    bool m_inputing;
    bool m_outputing;
    bool m_looping;

    bool m_transport_stopping;

    long m_tick;
    long m_tick_offset;

    double m_swing_ratio;
    double m_swing_reference;

    void set_running( bool a_running );

    string m_screen_set_notepad[c_max_sets];

    int m_offset;
    int m_screen_set;
    int m_reference_sequence;

    condition_var m_running_lock;
    condition_var m_stopping_lock;

    #ifdef USE_JACK
    jack_client_t *m_jack_client;
    bool m_jack_running;
    #endif

    void inner_start();
    void inner_stop();

    deque < state* >m_list_undo;
    deque < state* >m_list_redo;
    int m_undo_lock = 0;
    void undoable_lock(bool a_push_undo);
    void undoable_unlock();
    void push_undo();
    state * get_state();
    void set_state(state * s);


 public:
    bool is_running();

    perform();
    ~perform();

    void init();

    void clear_all();

    void start_playing();
    void stop_playing();
    void panic();

    void launch_input_thread();
    void launch_output_thread();

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

    void set_reference_sequence( int a_seqnum );
    sequence * get_reference_sequence();

    void start();
    void stop();

    #ifdef USE_JACK
    void init_jack();
    void deinit_jack();
    void start_jack();
    void stop_jack();
    void position_jack();
    #endif

    void off_sequences();
    void all_notes_off();

    bool is_active(int a_sequence);

    void new_sequence( int a_sequence );

    /* plays all notes to Curent tick */
    void play( long a_tick );
    void set_orig_ticks( long a_tick  );

    sequence * get_sequence( int a_sequence );

    void reset_sequences();

    void set_bpm(double a_bpm);
    double  get_bpm( );

    double get_swing();
    void set_swing(double swing);
    void set_swing_reference(double swing_reference);

    mastermidibus* get_master_midi_bus( );

    void output_func();
    void input_func();

    void save_playing_state();
    void restore_playing_state();

    void file_new();
    bool file_open(std::string filename);
    bool file_import(std::string filename);
    bool file_save();
    bool file_saveas(std::string filename);
    bool file_export(std::string filename);
    bool file_export_screenset(std::string filename);
    bool file_export_sequence(std::string filename, int seqnum);

    void pop_undo();
    void pop_redo();
    bool can_undo();
    bool can_redo();

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
        SEQ_SWING,
        SEQ_SWING_REFERENCE,
        SEQ_CURSOR,
        SEQ_SSET,
        SEQ_PANIC,
        SEQ_SSEQ,
        SEQ_SSEQ_AND_PLAY,
        SEQ_SSEQ_QUEUED,
        SEQ_STATUS,
        SEQ_STATUS_EXT,
        SEQ_SEQ_EDIT,

        SEQ_MODE_SOLO,
        SEQ_MODE_ON,
        SEQ_MODE_OFF,
        SEQ_MODE_TOGGLE,
        SEQ_MODE_RECORD,
        SEQ_MODE_RECORD_ON,
        SEQ_MODE_RECORD_OFF,
        SEQ_MODE_NEW,
        SEQ_MODE_RECORD_THROUGH,
        SEQ_MODE_SYNC,
        SEQ_MODE_COPY,
        SEQ_MODE_CUT,
        SEQ_MODE_PASTE,
        SEQ_MODE_DELETE,
        SEQ_MODE_CLEAR,

        SEQ_EDIT_MODE_BEATS,
        SEQ_EDIT_MODE_BUS_CHAN,
    };

    std::map<std::string, int> osc_commands = {
        {"/play",               SEQ_PLAY},
        {"/stop",               SEQ_STOP},
        {"/bpm",                SEQ_BPM},
        {"/swing",              SEQ_SWING},
        {"/swing/reference",    SEQ_SWING_REFERENCE},
        {"/cursor",             SEQ_CURSOR},
        {"/screenset",          SEQ_SSET},
        {"/panic",              SEQ_PANIC},
        {"/sequence",           SEQ_SSEQ},
        {"/sequence/trig",      SEQ_SSEQ_AND_PLAY},
        {"/sequence/queue",     SEQ_SSEQ_QUEUED},
        {"/sequence/edit",      SEQ_SEQ_EDIT},
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
        {"new",                 SEQ_MODE_NEW},
        {"through",             SEQ_MODE_RECORD_THROUGH},
        {"sync",                SEQ_MODE_SYNC},
        {"copy",                SEQ_MODE_COPY},
        {"cut",                 SEQ_MODE_CUT},
        {"paste",               SEQ_MODE_PASTE},
        {"delete",              SEQ_MODE_DELETE},
        {"clear",               SEQ_MODE_CLEAR}
    };

    std::map<std::string, int> osc_seq_edit_modes = {
        {"beats",               SEQ_EDIT_MODE_BEATS},
        {"bus_chan",            SEQ_EDIT_MODE_BUS_CHAN}
    };


    friend class mainwid;
    friend class midifile;
    friend class optionsfile;
    friend class options;

    #ifdef USE_JACK
    friend int jack_process_callback(jack_nframes_t nframes, void* arg);
    friend void jack_shutdown(void *arg);
    #endif
};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);


#ifdef USE_JACK
int jack_process_callback(jack_nframes_t nframes, void* arg);
void jack_shutdown(void *arg);
#endif

#endif
