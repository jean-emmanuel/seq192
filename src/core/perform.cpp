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

#include "perform.h"
#include "osc.h"
#include "midibus.h"
#include "event.h"
#include <stdio.h>
#include <time.h>
#include <sched.h>


bool global_is_modified = false;

perform::perform()
{
    for (int i=0; i< c_max_sequence; i++) {

        m_seqs[i] = NULL;
        m_seqs_active[i] = false;
    }

    m_running = false;
    m_looping = false;
    m_inputing = true;
    m_outputing = true;
    m_tick = 0;

    // m_key_start  = GDK_space;
    // m_key_stop   = GDK_Escape;

    m_screen_set = 0;

    m_jack_running = false;

    m_out_thread_launched = false;
    m_in_thread_launched = false;
}

void
perform::start_playing()
{
    inner_stop();

    if (!m_jack_running) {
        usleep(1000);
    }

    position_jack();
    start_jack();

    start();
}


void
perform::stop_playing()
{
    stop_jack();
    stop();
}

void
perform::panic()
{
    for (int i = 0; i < c_max_sequence; i++) {
        if (is_active(i)) {
            m_seqs[i]->set_playing(false);
            if (m_seqs[i]->get_queued()) {
                m_seqs[i]->toggle_queued();
            }
        }
    }
}


void perform::init()
{
    m_master_bus.init();
    m_clipboard.set_master_midi_bus(get_master_midi_bus());

    if (global_oscport != 0) {
        oscserver = new OSCServer(global_oscport);
        oscserver->start();
        oscserver->add_method(NULL, NULL, &perform::osc_callback, this);
    }

}

int perform::osc_callback(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data)
{

    perform *self = (perform *)user_data;

    // debug
    // int i;
    // printf("path: <%s>\n", path);
    // for (i = 0; i < argc; i++) {
    //     printf("arg %d '%c' ", i, types[i]);
    //     lo_arg_pp((lo_type)types[i], argv[i]);
    //     printf("\n");
    // }
    // printf("\n");
    // fflush(stdout);

    int command = self->osc_commands[(std::string) path];
    if (!command) return 0;

    switch (command) {
        case SEQ_PLAY:
            self->start_playing();
            break;
        case SEQ_STOP:
            self->stop_playing();
            break;
        case SEQ_SSET:
            self->set_screenset((int) argv[0]->i);
            break;
        case SEQ_PANIC:
            self->panic();
            break;
        case SEQ_SSEQ:
        case SEQ_SSEQ_AND_PLAY:
        case SEQ_SSEQ_QUEUED:
        {
            if (argc < 2 || types[0] != 's') return 0;

            // arg 0: mode
            int mode = self->osc_seq_modes[(std::string) &argv[0]->s];
            if (!mode) return 1;

            for (int i = 0; i < c_mainwnd_rows * c_mainwnd_cols; i++) {
                self->osc_selected_seqs[i] = 0;
            }
            // int selected_seqs[c_mainwnd_rows * c_mainwnd_cols];

            if (types[1] == 'i') {
                // arg 1: column number

                int col = argv[1]->i;
                if (col < 0 || col > c_mainwnd_cols) return 0;

                if (argc == 2) {
                    // select all rows in column
                    for (int i = 0; i < c_mainwnd_rows; i++) {
                        self->osc_selected_seqs[i + col * c_mainwnd_rows] = 1;
                    }
                } else {
                    // select some rows in column
                    for (int i = 2; i < argc; i++) {
                        if (types[i] == 'i') {
                            int row = argv[i]->i;
                            if (row < c_mainwnd_rows) {
                                self->osc_selected_seqs[row + col * c_mainwnd_rows] = 1;
                            }
                        }
                    }
                }

            } else if (types[1] == 's') {
                // arg 1...n: sequences names / osc pattern

                for (int i = 0; i < c_mainwnd_cols * c_mainwnd_rows; i++) {
                    int nseq = i + self->m_screen_set * c_mainwnd_cols * c_mainwnd_rows;
                    if (self->is_active(nseq)) {
                        for (int j = 1; j < argc; j++) {
                            if (types[j] == 's' && lo_pattern_match(self->m_seqs[nseq]->get_name(), &argv[j]->s)) {
                                self->osc_selected_seqs[i] = 1;
                                break;
                            }
                        }
                    }
                }

            }

            if (mode == SEQ_MODE_SOLO) {
                for (int i = 0; i < c_max_sequence; i++) {
                    if (self->is_active(i) && self->m_seqs[i]->get_playing()) {
                        if (command == SEQ_SSEQ_QUEUED) {
                            if (!self->m_seqs[i]->get_queued()) {
                                self->m_seqs[i]->toggle_queued();
                            }
                        } else {
                            self->m_seqs[i]->set_playing(false);
                        }
                    }
                }
            }

            for (int i = 0; i < c_mainwnd_rows * c_mainwnd_cols; i++) {
                if (self->osc_selected_seqs[i] == 1) {
                    int nseq = i + self->m_screen_set * c_mainwnd_cols * c_mainwnd_rows;
                    if (nseq < c_max_sequence && self->is_active(nseq)) {
                        switch (mode) {
                            case SEQ_MODE_SOLO:
                            case SEQ_MODE_ON:
                                if (command == SEQ_SSEQ_QUEUED) {
                                    if (!self->m_seqs[nseq]->get_playing() && !self->m_seqs[nseq]->get_queued()) {
                                        self->m_seqs[nseq]->toggle_queued();
                                    }
                                } else {
                                    self->m_seqs[nseq]->set_playing(true);
                                }
                                break;
                            case SEQ_MODE_OFF:
                                if (command == SEQ_SSEQ_QUEUED) {
                                    if (self->m_seqs[nseq]->get_playing() != self->m_seqs[nseq]->get_queued()) {
                                        // if playing and not queued or queued and not playing
                                        self->m_seqs[nseq]->toggle_queued();
                                    }
                                } else {
                                    self->m_seqs[nseq]->set_playing(false);
                                }
                                break;
                            case SEQ_MODE_TOGGLE:
                                if (command == SEQ_SSEQ_QUEUED) {
                                    self->m_seqs[nseq]->toggle_queued();
                                } else {
                                    self->m_seqs[nseq]->toggle_playing();
                                }
                                break;
                        }
                    }
                }
            }

            if (command == SEQ_SSEQ_AND_PLAY) {
                self->start_playing();
            }

            break;
        }
        case SEQ_STATUS:
        case SEQ_STATUS_EXT:
            char *address;
            if (argc == 1) {
                address = &argv[0]->s;
            } else {
                address = lo_address_get_url(lo_message_get_source(data));
            }
            self->osc_status(address, path);
            break;

    }


    return 0;
}


void perform::osc_status( char* address, const char* path)
{

    int command = osc_commands[(std::string) path];

    std::string json = "{";

    json += "\"playing\":" + std::to_string(m_running) + ",";
    json += "\"screenset\":" + std::to_string(m_screen_set) + ",";
    json += "\"screensetName\":\"" + (std::string)get_screen_set_notepad(m_screen_set)->c_str() + "\",";
    json += "\"tick\":\"" + std::to_string(get_tick()) + "\",";
    json += "\"bpm\":\"" + std::to_string(get_bpm()) + "\"";

    if (command == SEQ_STATUS_EXT) {

        json += ",\"sequences\":[";
        bool empty = true;
        for (int col = 0; col < c_mainwnd_cols; col++) {
            for (int row = 0; row < c_mainwnd_rows; row++) {
                int nseq = row + col * c_mainwnd_rows + m_screen_set * c_mainwnd_cols * c_mainwnd_rows;
                if (is_active(nseq)) {
                    empty = false;
                    json += "{";
                    json += "\"col\":" + std::to_string(col) + ",";
                    json += "\"row\":" + std::to_string(row) + ",";
                    json += "\"name\":\"" + (std::string)m_seqs[nseq]->get_name() + "\",";
                    json += "\"time\":\"" + std::to_string(m_seqs[nseq]->get_bpm()) + "/" + std::to_string(m_seqs[nseq]->get_bw()) + "\",";
                    json += "\"bars\":" + std::to_string((int)((double)m_seqs[nseq]->get_length() / c_ppqn / m_seqs[nseq]->get_bpm() * (m_seqs[nseq]->get_bw() / 4))) + ",";
                    json += "\"ticks\":" + std::to_string(m_seqs[nseq]->get_length()) + ",";
                    json += "\"queued\":" + std::to_string(m_seqs[nseq]->get_queued()) + ",";
                    json += "\"timesPlayed\":" + std::to_string(m_seqs[nseq]->get_times_played()) + ",";
                    json += "\"playing\":" + std::to_string(m_seqs[nseq]->get_playing());
                    json += "},";
                }
            }
        }

        if (!empty) json = json.substr(0, json.size() - 1);

        json += "]";

    }

    json += "}";

    oscserver->send_json(address, path, json.c_str());

}

void perform::init_jack()
{
    if ( global_with_jack_transport  && !m_jack_running)
    {
        m_jack_running = true;
        //printf ( "init_jack() m_jack_running[%d]\n", m_jack_running );

        do
        {
            // register as a jack client
            // only to retreive transport state changes and time

            m_jack_client = jack_client_open(PACKAGE, JackNullOption, NULL );

            if (m_jack_client == 0)
            {
                printf( "JACK server is not running.\n[JACK sync disabled]\n");
                m_jack_running = false;
                break;
            }

            jack_on_shutdown( m_jack_client, jack_shutdown,(void *) this );
            jack_set_process_callback(m_jack_client, jack_process_callback, (void *) this);

            if (jack_activate(m_jack_client))
            {
                printf("Cannot register as JACK client\n");
                m_jack_running = false;
                break;
            }
        }
        while (0);
    }

}


void perform::deinit_jack()
{
    if (global_with_jack_transport) {

        if (m_jack_running) {

            //printf ( "deinit_jack() m_jack_running[%d]\n", m_jack_running );

            m_jack_running = false;

            if (jack_client_close(m_jack_client)) {
                printf("Cannot close JACK client.\n");
            }

        }

        if (!m_jack_running){
            printf( "[JACK sync disabled]\n");
        }

    }
}


void perform::clear_all()
{
    reset_sequences();

    for (int i=0; i< c_max_sequence; i++ ){

        if ( is_active(i) )
            delete_sequence( i );
    }

    string e( "" );

    for (int i=0; i<c_max_sets; i++ ){
        set_screen_set_notepad( i, &e );
    }
}

perform::~perform()
{
    m_inputing = false;
    m_outputing = false;
    m_running = false;

    m_condition_var.signal();

    if (m_out_thread_launched )
        pthread_join( m_out_thread, NULL );

    if (m_in_thread_launched )
        pthread_join( m_in_thread, NULL );

    for (int i=0; i< c_max_sequence; i++ ){
        if ( is_active(i) ){
            delete m_seqs[i];
        }
    }

    deinit_jack();

    if (global_oscport != 0) {
        oscserver->stop();
        delete oscserver;
    }
}

void perform::add_sequence( sequence *a_seq, int a_perf )
{
    /* check for perferred */
    if ( a_perf < c_max_sequence &&
            is_active(a_perf) == false &&
            a_perf >= 0 ){

        m_seqs[a_perf] = a_seq;
        set_active(a_perf, true);
        //a_seq->set_tag( a_perf );

    } else {

        for (int i=a_perf; i< c_max_sequence; i++ ){

            if ( is_active(i) == false ){

                m_seqs[i] = a_seq;
                set_active(i,true);

                //a_seq->set_tag( i  );
                break;
            }
        }
    }
}


void perform::set_active( int a_sequence, bool a_active )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return;

    //printf ("set_active %d\n", a_active );

    if ( m_seqs_active[ a_sequence ] == true && a_active == false )
    {
        set_was_active(a_sequence);
    }

    m_seqs_active[ a_sequence ] = a_active;
}


void perform::set_was_active( int a_sequence )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return;

    //printf( "was_active true\n" );

    m_was_active_main[ a_sequence ] = true;
    m_was_active_edit[ a_sequence ] = true;
    m_was_active_perf[ a_sequence ] = true;
    m_was_active_names[ a_sequence ] = true;
}



bool perform::is_active( int a_sequence )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    return m_seqs_active[ a_sequence ];
}


bool perform::is_dirty_main (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_main();
    }

    bool was_active = m_was_active_main[ a_sequence ];
    m_was_active_main[ a_sequence ] = false;

    return was_active;
}


bool perform::is_dirty_edit (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_edit();
    }

    bool was_active = m_was_active_edit[ a_sequence ];
    m_was_active_edit[ a_sequence ] = false;

    return was_active;
}


sequence* perform::get_sequence( int a_sequence )
{
    return m_seqs[a_sequence];
}

mastermidibus* perform::get_master_midi_bus( )
{
    return &m_master_bus;
}


void perform::set_running( bool a_running )
{
    m_running = a_running;
}


bool perform::is_running()
{
    return m_running;
}

void perform::set_bpm(double a_bpm)
{
    if ( a_bpm < c_bpm_minimum ) a_bpm = c_bpm_minimum;
    if ( a_bpm > c_bpm_maximum ) a_bpm = c_bpm_maximum;
    if (a_bpm != get_bpm())
    {
        m_master_bus.set_bpm( a_bpm );
        global_is_modified = true;
    }
}


double  perform::get_bpm( )
{
    return  m_master_bus.get_bpm( );
}


void perform::delete_sequence( int a_num )
{
    set_active(a_num, false);

    if ( m_seqs[a_num] != NULL ){

        m_seqs[a_num]->set_playing( false );
        delete m_seqs[a_num];
        global_is_modified = true;
    }
}

void perform::copy_sequence( int a_num )
{
    if (is_active(a_num)) {
        m_clipboard = *get_sequence(a_num);
    }
}


void perform::cut_sequence( int a_num )
{
    if (is_active(a_num)) {
        m_clipboard = *get_sequence(a_num);
        delete_sequence(a_num);
    }
}

void perform::paste_sequence( int a_num )
{
    if (!is_active(a_num)) {
        new_sequence(a_num);
        *get_sequence(a_num) = m_clipboard;
    }
}

void perform::move_sequence( int a_from, int a_to )
{
    if (is_active(a_from) && !is_active(a_to)) {
        new_sequence(a_to);
        *get_sequence(a_to) = *get_sequence(a_from);
        delete_sequence(a_from);
    }
}

void perform::new_sequence( int a_sequence )
{
    m_seqs[ a_sequence ] = new sequence();
    m_seqs[ a_sequence ]->set_master_midi_bus( &m_master_bus );
    set_active(a_sequence, true);
    global_is_modified = true;
}


void perform::print()
{
    //   for( int i=0; i<m_numSeq; i++ ){

    //printf("Sequence %d\n", i);
    //m_seqs[i]->print();
    // }

    //  m_master_bus.print();
}


void perform::set_screen_set_notepad( int a_screen_set, string *a_notepad )
{
    if ( a_screen_set < c_max_sets )
        m_screen_set_notepad[a_screen_set] = *a_notepad;
}


string * perform::get_screen_set_notepad( int a_screen_set )
{
    return &m_screen_set_notepad[a_screen_set];
}


void perform::set_screenset( int a_ss )
{
    m_screen_set = a_ss;

    if ( m_screen_set < 0 )
        m_screen_set = c_max_sets - 1;

    if ( m_screen_set >= c_max_sets )
        m_screen_set = 0;

}


int perform::get_screenset()
{
    return m_screen_set;
}


void perform::play( long a_tick )
{

    /* just run down the list of sequences and have them dump */

    //printf( "play [%d]\n", a_tick );

    m_tick = a_tick;
    for (int i=0; i< c_max_sequence; i++ ){

        if ( is_active(i) ){
            assert( m_seqs[i] );


            if ( m_seqs[i]->get_queued() &&
                    m_seqs[i]->get_queued_tick() <= a_tick ){

                m_seqs[i]->play( m_seqs[i]->get_queued_tick() - 1 );
                m_seqs[i]->toggle_playing();
            }

            m_seqs[i]->play( a_tick );
        }
    }

    /* flush the bus */
    m_master_bus.flush();
}


void perform::set_orig_ticks( long a_tick  )
{
    for (int i=0; i< c_max_sequence; i++ ){

        if ( is_active(i) == true ){
            assert( m_seqs[i] );
            m_seqs[i]->set_orig_tick( a_tick );
        }
    }
}


void perform::start_jack(  )
{
    //printf( "perform::start_jack()\n" );
    if ( m_jack_running)
        jack_transport_start (m_jack_client );
}


void perform::stop_jack(  )
{
    //printf( "perform::stop_jack()\n" );
    if( m_jack_running )
        jack_transport_stop (m_jack_client);
}


void perform::position_jack()
{
    //printf( "perform::position_jack()\n" );
    if ( m_jack_running ){
        jack_transport_locate( m_jack_client, 0 );
    }
}


void perform::start()
{
    if (m_jack_running) {
        return;
    }

    inner_start();
}


void perform::stop()
{
    if (m_jack_running) {
        return;
    }

    inner_stop();
}


void perform::inner_start()
{
    m_condition_var.lock();

    if (!is_running()) {

        set_running(true);
        m_condition_var.signal();
    }

    m_condition_var.unlock();
}


void perform::inner_stop()
{
    set_running(false);
    //off_sequences();
    reset_sequences();
}


void perform::off_sequences()
{
    for (int i = 0; i < c_max_sequence; i++) {

        if (is_active(i)) {
            assert(m_seqs[i]);
            m_seqs[i]->set_playing(false);
        }
    }
}


void perform::all_notes_off()
{
    for (int i=0; i< c_max_sequence; i++) {

        if (is_active(i)) {
            assert(m_seqs[i]);
            m_seqs[i]->off_playing_notes();
        }
    }
    /* flush the bus */
    m_master_bus.flush();
}


void perform::reset_sequences()
{
    for (int i=0; i< c_max_sequence; i++) {

        if (is_active(i)) {
            assert( m_seqs[i] );

            bool state = m_seqs[i]->get_playing();

            m_seqs[i]->off_playing_notes();
            m_seqs[i]->set_playing(false);
            m_seqs[i]->zero_markers();
            m_seqs[i]->set_playing(state);
        }
    }
    /* flush the bus */
    m_master_bus.flush();
}


void perform::launch_output_thread()
{
    int err;

    err = pthread_create(&m_out_thread, NULL, output_thread_func, this);
    if (err != 0) {
        /*TODO: error handling*/
    }
    else
        m_out_thread_launched= true;
}


void perform::launch_input_thread()
{
    int err;

    err = pthread_create(&m_in_thread, NULL, input_thread_func, this);
    if (err != 0) {
        /*TODO: error handling*/
    }
    else
    m_in_thread_launched = true;
}


void* output_thread_func(void *a_pef )
{
    /* set our performance */
    perform *p = (perform *) a_pef;
    assert(p);

    p->output_func();

    return 0;
}



int jack_process_callback(jack_nframes_t nframes, void* arg)
{
    perform *m_mainperf = (perform *) arg;

    jack_position_t pos;
    jack_transport_state_t state = jack_transport_query( m_mainperf->m_jack_client, &pos );

    m_mainperf->m_jack_bpm = pos.beats_per_minute;

    if (state == JackTransportRolling  )
    {
        m_mainperf->inner_start();
    }
    else if (state == JackTransportStopped || state == JackTransportStarting) {
        m_mainperf->inner_stop();
    }

    return 0;
}



void perform::output_func()
{
    while (m_outputing) {

        //printf ("waiting for signal\n");

        m_condition_var.lock();

        while (!m_running) {

            m_condition_var.wait();

            /* if stopping, then kill thread */
            if (!m_outputing)
                break;
        }

        m_condition_var.unlock();

        // system time
        struct timespec system_time;

        // loop timeout: 0.1ms
        struct timespec ts = {
            .tv_sec = 0,
            .tv_nsec = 100000
        };

        long long last_time;
        long long delta_time;

        if (m_jack_running) {
            last_time = jack_get_time();
        } else {
            clock_gettime(CLOCK_REALTIME, &system_time);
            last_time = (system_time.tv_sec * 1000000) + (system_time.tv_nsec / 1000);
        }

        int ppqn = m_master_bus.get_ppqn();
        long current_tick = 0;

        while( m_running ){

            // bpm
            double bpm;
            if (m_jack_running && m_jack_bpm > 0.1) {
                // jack transport
                bpm = m_jack_bpm;
            } else {
                // or file
                bpm = m_master_bus.get_bpm();
            }

            // delta time
            if (m_jack_running) {
                // jack
                delta_time = jack_get_time() - last_time;
            } else {
                // or system
                clock_gettime(CLOCK_REALTIME, &system_time);
                delta_time = ((system_time.tv_sec * 1000000) + (system_time.tv_nsec / 1000)) - last_time;
            }

            // delta time to ticks
            double tick_duration = 1000000 * 60. / bpm / ppqn;
            int ticks = (int)(delta_time / tick_duration);
            current_tick += ticks;

            // increment time
            last_time += ticks * tick_duration;

            // play sequences at current tick
            play(current_tick);

            nanosleep(&ts, NULL);
        }

        m_tick = 0;
        m_master_bus.flush();
    }

    pthread_exit(0);
}


void* input_thread_func(void *a_pef )
{

    /* set our performance */
    perform *p = (perform *) a_pef;
    assert(p);

    p->input_func();

    return 0;
}


void perform::input_func()
{
    event ev;

    while (m_inputing) {

        if (m_master_bus.poll_for_midi() > 0) {

            do {

                if (m_master_bus.get_midi_event(&ev)) {

                    /* filter system wide messages */
                    if (ev.get_status() <= EVENT_SYSEX) {

                        /* is there a sequence set? */
                        if (m_master_bus.is_dumping()) {

                            ev.set_timestamp(m_tick);

                            /* dump to it */
                            (m_master_bus.get_sequence())->stream_event(&ev);
                        }

                    }

                }

            } while (m_master_bus.is_more_input());
        }
    }

    pthread_exit(0);
}


void perform::save_playing_state()
{
    for( int i=0; i<c_total_seqs; i++ ){

        if ( is_active(i) == true ){
            assert( m_seqs[i] );
            m_sequence_state[i] = m_seqs[i]->get_playing();
        }
        else
            m_sequence_state[i] = false;
    }
}


void perform::restore_playing_state()
{
    for( int i=0; i<c_total_seqs; i++ ){

        if ( is_active(i) == true ){
            assert( m_seqs[i] );
            m_seqs[i]->set_playing( m_sequence_state[i] );
        }
    }
}

void jack_shutdown(void *arg)
{
    perform *p = (perform *) arg;
    p->m_jack_running = false;

    printf("JACK shut down.\nJACK sync Disabled.\n");
}
