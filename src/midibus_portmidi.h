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

#ifndef SEQ24_MIDIBUS_PORTMIDI
#define SEQ24_MIDIBUS_PORTMIDI

class midibus;
class mastermidibus;

#ifdef __WIN32__

#include <string>

#include "portmidi.h"
#include "event.h"
#include "sequence.h"
#include "mutex.h"
#include "globals.h"

const int c_midibus_output_size = 0x100000;
const int c_midibus_input_size =  0x100000;
const int c_midibus_sysex_chunk = 0x100;

enum clock_e
{
    e_clock_off,
    e_clock_pos,
    e_clock_mod

};

class midibus
{
    
 private:

    char m_id;
    char m_pm_num;

    clock_e m_clock_type;
    bool m_inputing;

    static int m_clock_mod;

    /* name of bus */
    string m_name;

    /* last tick */
    long m_lasttick;

    /* locking */
    smutex m_mutex;

    /* mutex */
    void lock();
    void unlock();
    
    PortMidiStream* m_pms;
   

 public:
    midibus( char a_id,
             char a_pm_num,
    	     const char *a_client_name );


    ~midibus();

    bool init_out(  );
    bool init_in(  );
 
    void print();

    string get_name();
    int get_id();

    /* puts an event in the queue */
    void play( event *a_e24, unsigned char a_channel );
    void sysex( event *a_e24 );

    int poll_for_midi( );
    
    /* clock */
    void start();
    void stop();
    void clock(  long a_tick );
    void continue_from( long a_tick ); 
    void init_clock( long a_tick );
    void set_clock( clock_e a_clocking );
    clock_e get_clock( );

    void set_input( bool a_inputing );
    bool get_input( );

    void flush();
    //void remove_queued_on_events( int a_tag );

    /* master midi bus sets up the bus */
    friend class mastermidibus;

	/* address of client */
#if HAVE_LIBASOUND	     
    int get_client( void ) {  return m_dest_addr_client; };
    int get_port( void ) { return m_dest_addr_port; };
#endif

    static void set_clock_mod( int a_clock_mod );
    static int get_clock_mod( void );
 
};

class mastermidibus
{
 private:

    /* sequencer client handle */
#if HAVE_LIBASOUND	     
    snd_seq_t *m_alsa_seq;
#endif

    int m_num_out_buses;
    int m_num_in_buses;

    midibus *m_buses_out[c_maxBuses];
    midibus *m_buses_in[c_maxBuses];
    midibus *m_bus_announce;
    
    bool m_buses_out_active[c_maxBuses];
    bool m_buses_in_active[c_maxBuses];
    
    bool m_buses_out_init[c_maxBuses];
    bool m_buses_in_init[c_maxBuses];

    clock_e m_init_clock[c_maxBuses];
    bool m_init_input[c_maxBuses];

    /* id of queue */
    int m_queue;

    int m_ppqn;
    int m_bpm;
    
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
    //midibus *get_default_bus();
    //midibus *get_bus( int a_bus );


    void init();

    int get_num_out_buses();
    int get_num_in_buses();

    void set_bpm(int a_bpm);
    void set_ppqn(int a_ppqn);
    int get_bpm(){ return m_bpm;}
    int get_ppqn(){ return m_ppqn;}
    
    string get_midi_out_bus_name( int a_bus );
    string get_midi_in_bus_name( int a_bus );
    
    void print();
    void flush(); 
    
    void start();
    void stop();
    
    void clock(  long a_tick );
    void continue_from( long a_tick ); 
    void init_clock( long a_tick );
    
    int poll_for_midi( );
    bool is_more_input( );
    bool get_midi_event( event *a_in );
    void set_sequence_input( bool a_state, sequence *a_seq );
    
    bool is_dumping( ) { return m_dumping_input; }
    sequence* get_sequence( ) { return m_seq; }
    void sysex( event *a_event );
    
    
    void play( unsigned char a_bus, event *a_e24, unsigned char a_channel );
    
    void set_clock( unsigned char a_bus, clock_e a_clock_type );
    clock_e get_clock( unsigned char a_bus );

   
    void set_input( unsigned char a_bus, bool a_inputing );
    bool get_input( unsigned char a_bus );

};

#endif
#endif
