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

#ifndef SEQ24_MIDIBUS
#define SEQ24_MIDIBUS

/* forward declarations*/
class mastermidibus;
class midibus;

#include "config.h"

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

#include <string>

#include "event.h"
#include "sequence.h"
#include "mutex.h"
#include "globals.h"

const int c_midibus_output_size = 0x100000;
const int c_midibus_input_size =  0x100000;
const int c_midibus_sysex_chunk = 0x100;

class midibus
{

 private:

    int m_id;

    bool m_inputing;

    /* sequencer client handle */
    snd_seq_t *m_seq;

    /* address of client */
    int m_dest_addr_client;
    int m_dest_addr_port;

    int m_local_addr_client;
    int m_local_addr_port;

    /* id of queue */
    int m_queue;

    /* name of bus */
    string m_name;

    /* last tick */
    long m_lasttick;


    /* locking */
    smutex m_mutex;

    /* mutex */
    void lock();
    void unlock();



 public:

    /* constructor, client#, port#, sequencer,
       name of client, name of port */
    midibus( int a_localclient,
	     int a_destclient,
	     int a_destport,
	     snd_seq_t  *a_seq,
	     const char *a_client_name,
	     const char *a_port_name,
	     int a_id,
         int a_queue );

    midibus( int a_localclient,
	     snd_seq_t  *a_seq,
	     int a_id,
         int a_queue );

    ~midibus();

    bool init_out(  );
    bool init_in(  );
    bool deinit_in(  );
    bool init_out_sub(  );
    bool init_in_sub(  );

    void print();

    string get_name();
    int get_id();

    /* puts an event in the queue */
    void play( event *a_e24, unsigned char a_channel );
    void sysex( event *a_e24 );


    void set_input( bool a_inputing );
    bool get_input( );

    void flush();

    /* master midi bus sets up the bus */
    friend class mastermidibus;

	/* address of client */
    int get_client() {  return m_dest_addr_client; };
    int get_port() { return m_dest_addr_port; };

};

class mastermidibus
{
 private:

    /* sequencer client handle */
    snd_seq_t *m_alsa_seq;

    int m_num_out_buses;
    int m_num_in_buses;

    midibus *m_buses_out[c_maxBuses];
    midibus *m_buses_in[c_maxBuses];
    midibus *m_bus_announce;

    bool m_buses_out_active[c_maxBuses];
    bool m_buses_in_active[c_maxBuses];

    bool m_buses_out_init[c_maxBuses];
    bool m_buses_in_init[c_maxBuses];

    bool m_init_input[c_maxBuses];

    /* id of queue */
    int m_queue;

    int m_ppqn;
    double m_bpm;

    int  m_num_poll_descriptors;
    struct pollfd *m_poll_descriptors;

    /* for dumping midi input to sequence for recording */
    bool m_dumping_input;
    sequence *m_seq;

    /* locking */
    smutex m_mutex;

    /* mutex */
    void lock();
    void unlock();

 public:

    mastermidibus();
    ~mastermidibus();

    void init();

    snd_seq_t* get_alsa_seq( ) { return m_alsa_seq; };

    int get_num_out_buses();
    int get_num_in_buses();

    void set_bpm(double a_bpm);
    void set_ppqn(int a_ppqn);
    double get_bpm(){ return m_bpm;}
    int get_ppqn(){ return m_ppqn;}

    string get_midi_out_bus_name( int a_bus );
    string get_midi_in_bus_name( int a_bus );

    void print();
    void flush();

    void start();
    void stop();

    int poll_for_midi( );
    bool is_more_input( );
    bool get_midi_event( event *a_in );
    void set_sequence_input( sequence *a_seq );

    bool is_dumping( ) { return m_seq != NULL; }
    sequence* get_sequence( ) { return m_seq; }
    void sysex( event *a_event );

    void play( unsigned char a_bus, event *a_e24, unsigned char a_channel );

    void set_input( unsigned char a_bus, bool a_inputing );
    bool get_input( unsigned char a_bus );

};

#endif
