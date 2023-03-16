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
    }

    m_running = false;
    m_stopping = false;
    m_looping = false;
    m_inputing = true;
    m_outputing = true;
    m_tick = -1;
    m_tick_offset = 0;

    m_screen_set = 0;
    m_reference_sequence = -1;

    #ifdef USE_JACK
    m_jack_running = false;
    #endif

    m_out_thread_launched = false;
    m_in_thread_launched = false;

    set_swing(0);
    set_swing_reference(8);
}

void
perform::start_playing()
{

    stop_playing();

    #ifdef USE_JACK
    position_jack();
    start_jack();
    #endif

    start();
}


void
perform::stop_playing()
{

    #ifdef USE_JACK
    stop_jack();
    #endif

    stop();

    m_stopping_lock.lock();
    while (m_stopping) m_stopping_lock.wait();
    m_stopping_lock.unlock();

}

void
perform::panic()
{
    for (int i = 0; i < c_max_sequence; i++) {
        if (is_active(i)) {
            m_seqs[i]->set_playing(false);
            m_seqs[i]->off_queued();
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
        case SEQ_BPM:
            if (argc > 0)
            {
                double bpm = 0;
                if (types[0] == 'i') bpm = argv[0]->i;
                else if (types[0] == 'f') bpm = argv[0]->f;
                else break;
                self->set_bpm(bpm);
            }
            break;
        case SEQ_SWING:
            if (argc > 0)
            {
                double swing = 0;
                if (types[0] == 'i') swing = argv[0]->i;
                else if (types[0] == 'f') swing = argv[0]->f;
                else break;
                self->set_swing(swing);
            }
            break;
        case SEQ_SWING_REFERENCE:
            if (argc > 0)
            {
                int ref = 0;
                if (types[0] == 'i') ref = argv[0]->i;
                else if (types[0] == 'f') ref = argv[0]->f;
                else break;
                self->set_swing_reference(ref);
            }
            break;
        case SEQ_CURSOR:
            if (argc == 1 && (types[0] == 'i' || types[0] == 'f' || types[0] == 'd')) {
                bool restart = self->m_running;
                if (restart) self->stop_playing();
                if (types[0] == 'i') self->m_tick_offset = argv[0]->i * c_ppqn;
                if (types[0] == 'f') self->m_tick_offset = argv[0]->f * c_ppqn;
                if (types[0] == 'd') self->m_tick_offset = argv[0]->d * c_ppqn;
                self->set_orig_ticks(self->m_tick_offset);
                if (restart) self->start_playing();
            }
            break;
        case SEQ_SSET:
            if (argc > 0 && types[0] == 'i') self->set_screenset((int) argv[0]->i);
            break;
        case SEQ_PANIC:
            self->panic();
            break;
        case SEQ_SSEQ:
        case SEQ_SSEQ_AND_PLAY:
        case SEQ_SSEQ_QUEUED:
        {
            if (argc < 1 || types[0] != 's') return 0;

            // arg 0: mode
            int mode = self->osc_seq_modes[(std::string) &argv[0]->s];
            if (!mode) return 1;

            for (int i = 0; i < c_mainwnd_rows * c_mainwnd_cols; i++) {
                self->osc_selected_seqs[i] = 0;
            }

            if (mode == SEQ_MODE_RECORD_OFF) {
                self->get_master_midi_bus()->set_sequence_input(NULL);
                return 0;
            }

            if (argc < 2) return 0;

            // sequence selection
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
                            if (!self->m_seqs[i]->is_queued()) {
                                self->m_seqs[i]->toggle_queued(self->get_reference_sequence());
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
                                        self->m_seqs[nseq]->set_on_queued(self->get_reference_sequence());
                                } else {
                                    self->m_seqs[nseq]->set_playing(true);
                                }
                                break;
                            case SEQ_MODE_OFF:
                                if (command == SEQ_SSEQ_QUEUED) {
                                        // if playing and not queued or queued and not playing
                                        self->m_seqs[nseq]->set_off_queued(self->get_reference_sequence());
                                } else {
                                    self->m_seqs[nseq]->set_playing(false);
                                }
                                break;
                            case SEQ_MODE_TOGGLE:
                                if (command == SEQ_SSEQ_QUEUED) {
                                    self->m_seqs[nseq]->toggle_queued(self->get_reference_sequence());
                                } else {
                                    self->m_seqs[nseq]->toggle_playing();
                                }
                                break;
                            case SEQ_MODE_RECORD:
                                self->get_master_midi_bus()->set_sequence_input(self->m_seqs[nseq]->get_recording() ? NULL : self->m_seqs[nseq]);
                                return 0;
                            case SEQ_MODE_RECORD_ON:
                                self->get_master_midi_bus()->set_sequence_input(self->m_seqs[nseq]);
                                // only one sequence can be armed for recording
                                // ignore matching sequences after the first
                                return 0;
                            case SEQ_MODE_SYNC:
                                self->set_reference_sequence(nseq);
                                // only one sequence can be selected as reference
                                // ignore matching sequences after the first
                                return 0;
                            case SEQ_MODE_CLEAR:
                                self->m_seqs[nseq]->select_all();
                                self->m_seqs[nseq]->mark_selected();
                                self->m_seqs[nseq]->remove_marked();
                                break;
                            case SEQ_MODE_COPY:
                                // only one sequence can be copied at a time
                                // ignore matching sequences after the first
                                self->copy_sequence(nseq);
                                return 0;
                            case SEQ_MODE_CUT:
                                // only one sequence can be cut at a time
                                // ignore matching sequences after the first
                                self->cut_sequence(nseq);
                                return 0;
                            case SEQ_MODE_DELETE:
                                self->delete_sequence(nseq);
                                break;
                        }
                    } else if (nseq < c_max_sequence && !self->is_active(nseq)) {
                        switch (mode) {
                            case SEQ_MODE_PASTE:
                                self->paste_sequence(nseq);
                                return 0;
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
                    json += "\"queued\":" + std::to_string(m_seqs[nseq]->is_queued()) + ",";
                    json += "\"playing\":" + std::to_string(m_seqs[nseq]->get_playing()) + ",";
                    json += "\"timesPlayed\":" + std::to_string(m_seqs[nseq]->get_times_played()) + ",";
                    json += "\"recording\":" + std::to_string(m_seqs[nseq]->get_recording());
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

#ifdef USE_JACK
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

            m_jack_client = jack_client_open(global_client_name.c_str(), JackNullOption, NULL );

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
void jack_shutdown(void *arg)
{
    perform *p = (perform *) arg;
    p->m_jack_running = false;

    printf("JACK shut down.\nJACK sync Disabled.\n");
}
#endif

void perform::clear_all()
{
    undoable_lock(false);

    reset_sequences();

    for (int i=0; i< c_max_sequence; i++ ){

        if ( is_active(i) )
            delete_sequence( i );
    }

    string e( "" );

    for (int i=0; i<c_max_sets; i++ ){
        set_screen_set_notepad( i, &e );
    }

    for (long unsigned int i = 0; i < m_list_undo.size(); i++) {
        m_list_undo[i]->sequence_map.clear();
    }
    m_list_undo.clear();
    m_list_undo.shrink_to_fit();


    for (long unsigned int i = 0; i < m_list_redo.size(); i++) {
        m_list_redo[i]->sequence_map.clear();
    }
    m_list_redo.clear();
    m_list_redo.shrink_to_fit();

    undoable_unlock();
}

perform::~perform()
{

    stop();

    m_inputing = false;
    m_outputing = false;
    m_running = false;

    m_running_lock.signal();
    m_stopping_lock.signal();

    if (m_out_thread_launched )
        pthread_join( m_out_thread, NULL );

    if (m_in_thread_launched )
        pthread_join( m_in_thread, NULL );

    for (int i=0; i< c_max_sequence; i++ ){
        if ( is_active(i) ){
            delete m_seqs[i];
        }
    }

    #ifdef USE_JACK
    deinit_jack();
    #endif

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
        //a_seq->set_tag( a_perf );

    } else {

        for (int i=a_perf; i< c_max_sequence; i++ ){

            if ( is_active(i) == false ){

                m_seqs[i] = a_seq;

                //a_seq->set_tag( i  );
                break;
            }
        }
    }
}

bool perform::is_active( int a_sequence )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    return m_seqs[ a_sequence ] != NULL;
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

    if ( m_seqs[a_num] != NULL ){
        undoable_lock(true);

        m_seqs[a_num]->set_playing( false );
        delete m_seqs[a_num];
        m_seqs[a_num] = NULL;
        global_is_modified = true;

        undoable_unlock();
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
        undoable_lock(true);

        m_clipboard = *get_sequence(a_num);
        delete_sequence(a_num);

        undoable_unlock();
    }
}

void perform::paste_sequence( int a_num )
{
    if (!is_active(a_num)) {
        undoable_lock(true);

        new_sequence(a_num);
        *get_sequence(a_num) = m_clipboard;

        undoable_unlock();
    }
}

void perform::move_sequence( int a_from, int a_to )
{
    if (is_active(a_from) && !is_active(a_to)) {
        undoable_lock(true);

        new_sequence(a_to);
        *get_sequence(a_to) = *get_sequence(a_from);
        delete_sequence(a_from);

        undoable_unlock();
    }
}

void perform::new_sequence( int a_sequence )
{
    if (!is_active(a_sequence)) {
        undoable_lock(true);

        m_seqs[ a_sequence ] = new sequence();
        m_seqs[ a_sequence ]->set_master_midi_bus( &m_master_bus );
        global_is_modified = true;

        undoable_unlock();
    }
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
    if ( a_screen_set < c_max_sets ) {
        undoable_lock(true);

        m_screen_set_notepad[a_screen_set] = *a_notepad;

        undoable_unlock();
    }
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

void perform::set_reference_sequence( int a_seqnum )
{
    if (a_seqnum == m_reference_sequence) return;

    if (is_active(m_reference_sequence))
    {
        get_sequence(m_reference_sequence)->set_sync_reference(false);
    }

    m_reference_sequence = a_seqnum;

    if (is_active(m_reference_sequence))
    {
        get_sequence(m_reference_sequence)->set_sync_reference(true);
    }
}

sequence * perform::get_reference_sequence()
{
    if (is_active(m_reference_sequence))
    {
        return get_sequence(m_reference_sequence);
    }
    else
    {
        m_reference_sequence = -1;
        return NULL;
    }
}




void perform::play( long a_tick )
{

    if (a_tick <= m_tick) return;

    for (int i=0; i< c_max_sequence; i++ ){

        if ( is_active(i) ){
            assert( m_seqs[i] );


            if ( m_seqs[i]->is_queued() && m_seqs[i]->get_queued_tick() <= a_tick) {


                // m_seqs[i]->play( m_seqs[i]->get_queued_tick() - 1 );
                if (get_reference_sequence() != NULL && get_reference_sequence() != m_seqs[i]) {
                    m_seqs[i]->set_orig_tick( m_tick );
                    m_seqs[i]->set_sync_offset(m_tick % m_seqs[i]->get_length());
                }

                m_seqs[i]->set_playing(m_seqs[i]->get_queued() == QUEUED_ON);
            }

            m_seqs[i]->play( a_tick, m_swing_ratio, m_swing_reference );
        }
    }
    m_tick = a_tick;

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


#ifdef USE_JACK
void perform::start_jack(  )
{
    //printf( "perform::start_jack()\n" );
    if ( m_jack_running)
        jack_transport_start (m_jack_client );
}


void perform::stop_jack(  )
{
    //printf( "perform::stop_jack()\n" );
    if( m_jack_running ) {
        jack_transport_stop (m_jack_client);
    }
}


void perform::position_jack()
{
    //printf( "perform::position_jack()\n" );
    if ( m_jack_running ){
        jack_transport_locate( m_jack_client, 0 );
    }
}
#endif

void perform::start()
{
    #ifdef USE_JACK
    if (m_jack_running) {
        return;
    }
    #endif

    inner_start();
}


void perform::stop()
{
    inner_stop();
}


void perform::inner_start()
{

    m_running_lock.lock();

    if (!is_running()) {
        set_running(true);
        m_running_lock.signal();
    }

    m_running_lock.unlock();
}


void perform::inner_stop()
{

    m_running_lock.lock();
    m_stopping_lock.lock();

    if (is_running() && !m_stopping) {
        m_stopping = true;
    }

    m_stopping_lock.unlock();
    m_running_lock.unlock();

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
            m_seqs[i]->set_sync_offset(0);
            m_seqs[i]->set_playing(false);
            m_seqs[i]->set_orig_tick(m_tick_offset);
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


#ifdef USE_JACK
int jack_process_callback(jack_nframes_t nframes, void* arg)
{
    perform *m_mainperf = (perform *) arg;

    jack_position_t pos;
    jack_transport_state_t state = jack_transport_query( m_mainperf->m_jack_client, &pos );

    if (pos.beats_per_minute > c_bpm_minimum) {
        m_mainperf->m_master_bus.set_bpm(pos.beats_per_minute);
    }

    if (state == JackTransportRolling)
    {
        m_mainperf->inner_start();
    }
    else if (state == JackTransportStopped || state == JackTransportStarting) {
        m_mainperf->inner_stop();
    }

    return 0;
}
#endif


void perform::output_func()
{
    while (m_outputing) {

        //printf ("waiting for signal\n");

        m_running_lock.lock();

        while (!m_running) {

            m_running_lock.wait();

            /* if stopping, then kill thread */
            if (!m_outputing)
                break;
        }

        m_running_lock.unlock();

        // system time
        struct timespec system_time;

        // loop timeout: 2ms
        struct timespec ts = {
            .tv_sec = 0,
            .tv_nsec = 1000 * c_thread_trigger_us
        };

        long long start_time;
        long long now_time;
        long long playing_time;
        long long delta_time;
        long long exec_time;

        clock_gettime(CLOCK_MONOTONIC_RAW, &system_time);
        start_time = (system_time.tv_sec * 1e6) + (system_time.tv_nsec / 1e3);
        playing_time = 0;

        double ppqn = m_master_bus.get_ppqn();

        long double current_tick = 0;

        while (m_running) {

            // delta time
            clock_gettime(CLOCK_MONOTONIC_RAW, &system_time);
            now_time = ((system_time.tv_sec * 1e6) + (system_time.tv_nsec / 1e3)) - start_time;

            // bpm
            double bpm = m_master_bus.get_bpm();

            // delta time
            delta_time = now_time - playing_time;

            // delta time to ticks
            double tick_duration = 1e6 * 60. / bpm / ppqn;
            double ticks = delta_time / tick_duration;

            // increment ticks
            current_tick += ticks;

            // increment playing time
            playing_time = now_time;

            // play sequences at current tick
            play(current_tick + m_tick_offset);

            m_stopping_lock.lock();
            if (m_stopping) break;
            m_stopping_lock.unlock();

            // exec time
            clock_gettime(CLOCK_MONOTONIC_RAW, &system_time);
            exec_time = ((system_time.tv_sec * 1e6) + (system_time.tv_nsec / 1e3)) - start_time;

            if (exec_time - now_time < c_thread_trigger_us) {
                // adjust sleep time
                ts.tv_nsec = 1e3 * c_thread_trigger_us - (exec_time - now_time) * 1e3;
                nanosleep(&ts, NULL);
            }
        }

        m_tick = -1;

        if (m_stopping) {
            m_running_lock.lock();
            set_running(false);
            m_running_lock.unlock();

            reset_sequences();
            m_stopping = false;
            m_stopping_lock.signal();
        } else {
            m_master_bus.flush();
        }

        m_stopping_lock.unlock();

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
                        if (m_master_bus.is_dumping() && m_tick >= 0) {

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


void perform::file_new()
{
    clear_all();
    global_filename = "";
    global_is_modified = false;
}

bool perform::file_open(std::string filename)
{
    clear_all();
    midifile f(filename);
    bool result = f.parse(this, 0);
    global_is_modified = !result;
    if (result) global_filename = filename;
    return result;
}

bool perform::file_import(std::string filename)
{
    midifile f(filename);
    bool result = f.parse(this, get_screenset());
    global_is_modified = !result;
    return result;
}

bool perform::file_save()
{
    midifile f(global_filename);
    bool result = f.write(this, -1, -1);
    if (result) global_is_modified = false;
    return result;
}

bool perform::file_saveas(std::string filename)
{
    midifile f(filename);
    bool result = f.write(this, -1, -1);
    if (result) {
        global_filename = filename;
        global_is_modified = false;
    }
    return result;

}

bool perform::file_export(std::string filename)
{
    midifile f(filename);
    bool result = f.write(this, -1, -1);
    return result;
}

bool perform::file_export_screenset(std::string filename)
{
    midifile f(filename);
    bool result = f.write(this, get_screenset(), -1);
    return result;
}

bool perform::file_export_sequence(std::string filename, int seqnum)
{
    midifile f(filename);
    bool result = f.write(this, get_screenset(), seqnum);
    return result;
}

state * perform::get_state()
{
    state * s = new state();
    for (int i = 0; i < c_max_sequence; i++) {
        if (is_active(i)) {
            s->sequence_map[i] = *get_sequence(i);
        }
    }
    for (int i = 0; i < c_max_sets; i++) {
        s->notepad[i] = m_screen_set_notepad[i];
    }
    return s;
}

void perform::set_state(state * s)
{
    undoable_lock(false);

    save_playing_state();

    for (int i = 0; i < c_max_sequence; i++) {
        if (is_active(i)) delete_sequence(i);
        if (s->sequence_map.find(i) != s->sequence_map.end()) {
            new_sequence(i);
            *m_seqs[i] = s->sequence_map.find(i)->second;
        }
    }

    for (int i = 0; i < c_max_sets; i++) {
        set_screen_set_notepad(i, &s->notepad[i]);
    }

    restore_playing_state();

    global_is_modified = true;

    undoable_unlock();
}

void perform::push_undo()
{
    m_list_undo.push_back(get_state());

    if (can_redo()) {
        for (long unsigned int i = 0; i < m_list_redo.size(); i++) {
            m_list_redo[i]->sequence_map.clear();
        }
        m_list_redo.clear();
        m_list_redo.shrink_to_fit();
    }

    if (m_list_undo.size() > c_max_undo_history) {
        m_list_undo.front()->sequence_map.clear();
        m_list_undo.pop_front();
        m_list_undo.shrink_to_fit();
    }
}

void perform::pop_undo()
{
    if (can_undo())
    {
        m_list_redo.push_back(get_state());
        set_state(m_list_undo.back());
        m_list_undo.back()->sequence_map.clear();
        m_list_undo.pop_back();
        m_list_undo.shrink_to_fit();
    }
}

void perform::pop_redo()
{
    if (can_redo())
    {
        m_list_undo.push_back(get_state());
        set_state(m_list_redo.back());
        m_list_redo.back()->sequence_map.clear();
        m_list_redo.pop_back();
        m_list_redo.shrink_to_fit();
    }
}

bool perform::can_undo()
{
    return m_list_undo.size() > 0;
}

bool perform::can_redo()
{
    return m_list_redo.size() > 0;
}


void perform::undoable_lock(bool a_push_undo)
{
    if (m_undo_lock == 0 && a_push_undo) push_undo();
    m_undo_lock++;
}

void perform::undoable_unlock()
{
    m_undo_lock--;
}

void perform::set_swing(double swing)
{
    // define swing strengh
    // 0 = no swing
    // > 0 = swing
    // < = anti-swing
    if (swing > 3.99) swing = 3.99;
    else if (swing < -3.99) swing = -3.99;
    m_swing_ratio = swing;
}

void perform::set_swing_reference(double swing_reference)
{
    // define which beat unit should be the swing reference
    // 8 = 8th will swing
    // 16 = 16th will swing
    if (swing_reference <= 0) swing_reference = 1;
    m_swing_reference = 2 * (4 * c_ppqn) / swing_reference;
}
