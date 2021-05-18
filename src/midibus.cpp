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

#include "midibus.h"

#ifdef HAVE_LIBASOUND
#    include <sys/poll.h>
#endif

#ifdef LASH_SUPPORT
#    include "lash.h"
#endif


#ifdef HAVE_LIBASOUND
midibus::midibus( int a_localclient,
		  int a_destclient,
		  int a_destport,
		  snd_seq_t *a_seq,
		  const char *a_client_name,
		  const char *a_port_name,
		  int a_id, int a_queue )
{
    /* set members */
    m_local_addr_client = a_localclient;
    m_dest_addr_client = a_destclient;
    m_dest_addr_port   = a_destport;
    m_seq            = a_seq;
    m_queue          = a_queue;

    m_id = a_id;
    m_inputing = false;


    char name[60];
    if ( global_user_midi_bus_definitions[m_id].alias.length() > 0 )
    {
        snprintf(name, 59, "(%s)",
                global_user_midi_bus_definitions[m_id].alias.c_str());
    }
    else
    {
        snprintf(name,59,"(%s)",a_port_name);
    }

    /* copy names */
    char tmp[80];
    snprintf( tmp, 79, "[%d] %d:%d %s",
	      m_id,
	      m_dest_addr_client,
	      m_dest_addr_port,
	      name );

    m_name = tmp;
}

midibus::midibus( int a_localclient,
		  snd_seq_t *a_seq,
		  int a_id, int a_queue )
{
    /* set members */
    m_local_addr_client = a_localclient;
    m_seq            = a_seq;
    m_queue          = a_queue;

    m_id = a_id;
    m_inputing = false;

    /* copy names */
    char tmp[60];
    snprintf( tmp, 59, "[%d] seq24 %d",
	      m_id,
	      m_id );

    m_name = tmp;
}
#endif

void
midibus::lock( )
{
    m_mutex.lock();
}


void
midibus::unlock( )
{
    m_mutex.unlock();
}


bool midibus::init_out( )
{
#ifdef HAVE_LIBASOUND
    /* temp return */
    int ret;

    /* create ports */
    ret = snd_seq_create_simple_port(m_seq,
				     m_name.c_str(),
			     	     SND_SEQ_PORT_CAP_NO_EXPORT |
				     SND_SEQ_PORT_CAP_READ,
				     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				     SND_SEQ_PORT_TYPE_APPLICATION );
    m_local_addr_port = ret;

    if ( ret < 0 ){
        printf( "snd_seq_create_simple_port(write) error\n");
        return false;
    }

    /* connect to */
    ret = snd_seq_connect_to( m_seq,
			      m_local_addr_port,
			      m_dest_addr_client,
			      m_dest_addr_port );
    if ( ret < 0 ){
        printf( "snd_seq_connect_to(%d:%d) error\n",
                m_dest_addr_client, m_dest_addr_port);
        return false;
    }
#endif
    return true;
}


bool midibus::init_out_sub( )
{
#ifdef HAVE_LIBASOUND
    /* temp return */
    int ret;

    /* create ports */
    ret = snd_seq_create_simple_port(m_seq,
				     m_name.c_str(),
				     SND_SEQ_PORT_CAP_READ |
                     SND_SEQ_PORT_CAP_SUBS_READ,
				     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				     SND_SEQ_PORT_TYPE_APPLICATION );
    m_local_addr_port = ret;

    if ( ret < 0 ){
        printf( "snd_seq_create_simple_port(write) error\n");
        return false;
    }
#endif
    return true;
}



bool midibus::init_in( )
{
#ifdef HAVE_LIBASOUND
    /* temp return */
    int ret;

    /* create ports */
    ret = snd_seq_create_simple_port(m_seq,
                                     "seq24 in",
                                     SND_SEQ_PORT_CAP_NO_EXPORT |
                                     SND_SEQ_PORT_CAP_WRITE,
                                     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                     SND_SEQ_PORT_TYPE_APPLICATION );
    m_local_addr_port = ret;

    if ( ret < 0 ){
        printf( "snd_seq_create_simple_port(read) error\n");
        return false;
    }

    snd_seq_port_subscribe_t *subs;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_addr_t sender, dest;

    /* the destinatino port is actually our local port */
    sender.client = m_dest_addr_client;
    sender.port = m_dest_addr_port;
    dest.client = m_local_addr_client;
    dest.port = m_local_addr_port;

    /* set in and out ports */
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);

    /* use the master queue, and get ticks */
    snd_seq_port_subscribe_set_queue(subs, m_queue);
    snd_seq_port_subscribe_set_time_update(subs, 1);

    /* subscribe */
    ret = snd_seq_subscribe_port(m_seq, subs);

    if ( ret < 0 ){
        printf( "snd_seq_connect_from(%d:%d) error\n",
                m_dest_addr_client, m_dest_addr_port);
        return false;
    }
#endif
    return true;
}


bool midibus::init_in_sub( )
{
#ifdef HAVE_LIBASOUND
    /* temp return */
    int ret;

    /* create ports */
    ret = snd_seq_create_simple_port(m_seq, "seq24 in",
            SND_SEQ_PORT_CAP_WRITE |
            SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC |
            SND_SEQ_PORT_TYPE_APPLICATION );
    m_local_addr_port = ret;

    if ( ret < 0 ){
        printf( "snd_seq_create_simple_port(write) error\n");
        return false;
    }
#endif
    return true;
}


bool midibus::deinit_in( )
{
#ifdef HAVE_LIBASOUND
    /* temp return */
    int ret;

    snd_seq_port_subscribe_t *subs;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_addr_t sender, dest;

    /* the destinatino port is actually our local port */
    sender.client = m_dest_addr_client;
    sender.port = m_dest_addr_port;
    dest.client = m_local_addr_client;
    dest.port = m_local_addr_port;

    /* set in and out ports */
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);

    /* use the master queue, and get ticks */
    snd_seq_port_subscribe_set_queue(subs, m_queue);
    snd_seq_port_subscribe_set_time_update(subs, 1);

    /* subscribe */
    ret = snd_seq_unsubscribe_port(m_seq, subs);

    if ( ret < 0 ){
        printf( "snd_seq_unsubscribe_port(%d:%d) error\n",
                m_dest_addr_client, m_dest_addr_port);
        return false;
    }
#endif
    return true;
}


int
midibus::get_id( )
{
    return m_id;
}


void
midibus::print()
{
    printf( "%s" , m_name.c_str() );
}

string
midibus::get_name()
{
    return m_name;
}

midibus::~midibus()
{

}


/* takes an native event, encodes to alsa event,
   puts it in the queue */
void
midibus::play( event *a_e24, unsigned char a_channel )
{
    lock();

#ifdef HAVE_LIBASOUND

		snd_seq_event_t ev;

		/* alsa midi parser */
		snd_midi_event_t *midi_ev;

		/* temp for midi data */
		unsigned char buffer[3];

		/* fill buffer and set midi channel */
		buffer[0] = a_e24->get_status();
		buffer[0] += (a_channel & 0x0F);
		a_e24->get_data( &buffer[1], &buffer[2] );
		snd_midi_event_new( 10, &midi_ev );

		/* clear event */
		snd_seq_ev_clear( &ev );
		snd_midi_event_encode( midi_ev, buffer, 3, &ev );
		snd_midi_event_free( midi_ev );

		/* set source */
		snd_seq_ev_set_source(&ev, m_local_addr_port );
		snd_seq_ev_set_subs(&ev);

		/* set tag unique to each sequence for removal purposes */
		//ev.tag = a_tag;

		// its immediate
		snd_seq_ev_set_direct( &ev );

		/* pump it into the queue */
		snd_seq_event_output(m_seq, &ev);
#endif
    unlock();
}


inline long
min ( long a, long b ){

    if ( a < b )
        return a;
    return b;
}

/* takes an native event, encodes to alsa event,
   puts it in the queue */
void
midibus::sysex( event *a_e24 )
{
    lock();
#ifdef HAVE_LIBASOUND
    snd_seq_event_t ev;

    /* clear event */
    snd_seq_ev_clear( &ev );
    snd_seq_ev_set_priority( &ev, 1 );

    /* set source */
    snd_seq_ev_set_source(&ev, m_local_addr_port );
    snd_seq_ev_set_subs(&ev);

    // its immediate
    snd_seq_ev_set_direct( &ev );

    unsigned char *data = a_e24->get_sysex();
    long data_size =  a_e24->get_size();

    for (long offset = 0; offset < data_size;
            offset += c_midibus_sysex_chunk) {

        long data_left = data_size - offset;

        snd_seq_ev_set_sysex( &ev,
                min( data_left, c_midibus_sysex_chunk),
                &data[offset] );

        /* pump it into the queue */
        snd_seq_event_output_direct(m_seq, &ev);
        usleep(80000);
        flush();
    }
#endif
    unlock();
}


// flushes our local queue events out into ALSA
void
midibus::flush()
{
    lock();
#ifdef HAVE_LIBASOUND
    snd_seq_drain_output( m_seq );
#endif
    unlock();
}

void
midibus::set_input( bool a_inputing )
{
    if ( m_inputing != a_inputing ){

        m_inputing = a_inputing;

        if (m_inputing){
            init_in();
        }
        else
        {
            deinit_in();
        }
    }
}


bool
midibus::get_input( )
{
    return m_inputing;
}

void
mastermidibus::lock( )
{
   m_mutex.lock();
}


void
mastermidibus::unlock( )
{
   m_mutex.unlock();
}


void
mastermidibus::set_ppqn( int a_ppqn )
{
    lock();
#ifdef HAVE_LIBASOUND
    m_ppqn = a_ppqn;

    /* allocate tempo struct */
    snd_seq_queue_tempo_t *tempo;
    snd_seq_queue_tempo_alloca( &tempo );

    /* fill tempo struct with current tempo info */
    snd_seq_get_queue_tempo( m_alsa_seq, m_queue, tempo );

    /* set ppqn */
    snd_seq_queue_tempo_set_ppq( tempo, m_ppqn );

    /* give tempo struct to the queue */
    snd_seq_set_queue_tempo( m_alsa_seq, m_queue, tempo );
#endif
    unlock();
}


void
mastermidibus::set_bpm( int a_bpm )
{
    lock();
#ifdef HAVE_LIBASOUND
    m_bpm = a_bpm;

    /* allocate tempo struct */
    snd_seq_queue_tempo_t *tempo;
    snd_seq_queue_tempo_alloca( &tempo );

    /* fill tempo struct with current tempo info */
    snd_seq_get_queue_tempo( m_alsa_seq, m_queue, tempo );

    snd_seq_queue_tempo_set_tempo( tempo, 60000000 / m_bpm );

    /* give tempo struct to the queue */
    snd_seq_set_queue_tempo(m_alsa_seq, m_queue, tempo );
#endif
    unlock();
}

// flushes our local queue events out into ALSA
void
mastermidibus::flush()
{
    lock();
#ifdef HAVE_LIBASOUND
    snd_seq_drain_output( m_alsa_seq );
#endif
    unlock();
}


/* fills the array with our buses */
mastermidibus::mastermidibus()
{
    /* temp return */
    int ret;

    /* set initial number buses */
    m_num_out_buses = 0;
    m_num_in_buses = 0;

    for( int i=0; i<c_maxBuses; ++i ){
        m_buses_in_active[i] = false;
        m_buses_out_active[i] = false;
        m_buses_in_init[i] = false;
        m_buses_out_init[i] = false;

        m_init_input[i] = false;
    }

#ifdef HAVE_LIBASOUND
    /* open the sequencer client */
    ret = snd_seq_open(&m_alsa_seq, "default",  SND_SEQ_OPEN_DUPLEX, 0);

    if ( ret < 0 ){
	printf( "snd_seq_open() error\n");
	exit(1);
    }

    /* set our clients name */
    snd_seq_set_client_name(m_alsa_seq, "seq24");

    /* set up our clients queue */
    m_queue = snd_seq_alloc_queue( m_alsa_seq );
#endif
#ifdef LASH_SUPPORT
	/* notify lash of our client ID so it can restore connections */
	lash_driver->set_alsa_client_id(snd_seq_client_id(m_alsa_seq));
#endif
}


void
mastermidibus::init( )
{
#ifdef HAVE_LIBASOUND
    /* client info */
    snd_seq_client_info_t *cinfo;
    /* port info */
    snd_seq_port_info_t *pinfo;

    int client;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_client_info_set_client(cinfo, -1);

    //printf( "global_ports %d\n", global_manual_alsa_ports );

    if ( global_manual_alsa_ports )
    {

        int num_buses = 16;

        for( int i=0; i<num_buses; ++i )
        {

            m_buses_out[i] =
                new midibus( snd_seq_client_id( m_alsa_seq ), m_alsa_seq, i+1, m_queue );

            m_buses_out[i]->init_out_sub();
            m_buses_out_active[i] = true;
            m_buses_out_init[i] = true;
        }

        m_num_out_buses = num_buses;

        /* only one in */
        m_buses_in[0] =
            new midibus( snd_seq_client_id( m_alsa_seq ),
                    m_alsa_seq,
                    m_num_in_buses, m_queue);

        m_buses_in[0]->init_in_sub();
        m_buses_in_active[0] = true;
        m_buses_in_init[0] = true;
        m_num_in_buses = 1;


    }
    else
    {
        /* while the next client one the sequencer is avaiable */
        while (snd_seq_query_next_client(m_alsa_seq, cinfo) >= 0){

            /* get client from cinfo */
            client = snd_seq_client_info_get_client(cinfo);

            /* fill pinfo */
            snd_seq_port_info_alloca(&pinfo);
            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);

            /* while the next port is avail */
            while (snd_seq_query_next_port(m_alsa_seq, pinfo) >= 0 ){

                /* get its capability */
                int cap =  snd_seq_port_info_get_capability(pinfo);

                if ( snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo) &&
                        snd_seq_port_info_get_client(pinfo) != SND_SEQ_CLIENT_SYSTEM){

                    /* the outs */
                    if ( (cap & SND_SEQ_PORT_CAP_SUBS_WRITE) != 0 &&
                            snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo)){

                        m_buses_out[m_num_out_buses] =
                            new midibus( snd_seq_client_id( m_alsa_seq ),
                                    snd_seq_port_info_get_client(pinfo),
                                    snd_seq_port_info_get_port(pinfo),
                                    m_alsa_seq,
                                    snd_seq_client_info_get_name(cinfo),
                                    snd_seq_port_info_get_name(pinfo),
                                    m_num_out_buses, m_queue );

                        if ( m_buses_out[m_num_out_buses]->init_out() ){
                            m_buses_out_active[m_num_out_buses] = true;
                            m_buses_out_init[m_num_out_buses] = true;
                        } else {
                            m_buses_out_init[m_num_out_buses] = true;
                        }

                        m_num_out_buses++;
                    }

                    /* the ins */
                    if ( (cap & SND_SEQ_PORT_CAP_SUBS_READ) != 0 &&
                            snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo)){

                        m_buses_in[m_num_in_buses] =
                            new midibus( snd_seq_client_id( m_alsa_seq ),
                                    snd_seq_port_info_get_client(pinfo),
                                    snd_seq_port_info_get_port(pinfo),
                                    m_alsa_seq,
                                    snd_seq_client_info_get_name(cinfo),
                                    snd_seq_port_info_get_name(pinfo),
                                    m_num_in_buses, m_queue);

                        //if ( m_buses_in[m_num_in_buses]->init_in() ){
                        m_buses_in_active[m_num_in_buses] = true;
                        m_buses_in_init[m_num_in_buses] = true;
                        //} else {
                        //m_buses_in_init[m_num_in_buses] = true;
                        //}
                        m_num_in_buses++;
                    }
                }
            }

        } /* end loop for clients */
    }

    set_bpm( c_bpm );
    set_ppqn( c_ppqn );

    /* midi input */
    /* poll descriptors */

    /* get number of file descriptors */
    m_num_poll_descriptors = snd_seq_poll_descriptors_count(m_alsa_seq, POLLIN);

    /* allocate into */
    m_poll_descriptors = new pollfd[m_num_poll_descriptors];

    /* get descriptors */
    snd_seq_poll_descriptors(m_alsa_seq,
            m_poll_descriptors,
            m_num_poll_descriptors,
            POLLIN);

    set_sequence_input( false, NULL );

    /* sizes */
    snd_seq_set_output_buffer_size(m_alsa_seq, c_midibus_output_size );
    snd_seq_set_input_buffer_size(m_alsa_seq, c_midibus_input_size );


    m_bus_announce =
        new midibus( snd_seq_client_id( m_alsa_seq ),
                SND_SEQ_CLIENT_SYSTEM,
                SND_SEQ_PORT_SYSTEM_ANNOUNCE,
                m_alsa_seq,
                "system", "annouce",
                0, m_queue);

    m_bus_announce->set_input(true);

#endif
}

mastermidibus::~mastermidibus()
{
    for ( int i=0; i<m_num_out_buses; i++ )
	delete m_buses_out[i];
#ifdef HAVE_LIBASOUND
    snd_seq_event_t ev;

    /* kill timer */
    snd_seq_ev_clear(&ev);

    snd_seq_stop_queue( m_alsa_seq, m_queue, &ev );
    snd_seq_free_queue( m_alsa_seq, m_queue );

    /* close client */
    snd_seq_close( m_alsa_seq );
#endif
}



void
mastermidibus::sysex( event *a_ev )
{
	lock();

    for ( int i=0; i<m_num_out_buses; i++ )
      m_buses_out[i]->sysex( a_ev );

    flush();

	unlock();
}


void
mastermidibus::play( unsigned char a_bus, event *a_e24, unsigned char a_channel )
{
	lock();
	if ( m_buses_out_active[a_bus] && a_bus < m_num_out_buses ){
		m_buses_out[a_bus]->play( a_e24, a_channel );
	}
	unlock();
}

void
mastermidibus::set_input( unsigned char a_bus, bool a_inputing )
{
    lock();
    if ( a_bus < c_maxBuses ){
        m_init_input[a_bus] = a_inputing;
    }

    if ( m_buses_in_active[a_bus] && a_bus < m_num_in_buses ){
        m_buses_in[a_bus]->set_input( a_inputing );
    }
    unlock();
}

bool
mastermidibus::get_input( unsigned char a_bus )
{
	if ( m_buses_in_active[a_bus] && a_bus < m_num_in_buses ){
		return m_buses_in[a_bus]->get_input();
	}
	return false;
}


string
mastermidibus::get_midi_out_bus_name( int a_bus )
{
	if ( m_buses_out_active[a_bus] && a_bus < m_num_out_buses ){
		return m_buses_out[a_bus]->get_name();
	}

	/* copy names */
	char tmp[60];

	if ( m_buses_out_init[a_bus] ){
		snprintf( tmp, 59, "[%d] %d:%d (disconnected)",
				  a_bus,
				  m_buses_out[a_bus]->get_client(),
				  m_buses_out[a_bus]->get_port() );
	} else {
		snprintf( tmp, 59, "[%d] (unconnected)",
				  a_bus );
	}

	string ret = tmp;
	return ret;

}


string
mastermidibus::get_midi_in_bus_name( int a_bus )
{
	if ( m_buses_in_active[a_bus] && a_bus < m_num_in_buses ){
		return m_buses_in[a_bus]->get_name();
	}

	/* copy names */
	char tmp[60];

	if ( m_buses_in_init[a_bus] ){
		snprintf( tmp, 59, "[%d] %d:%d (disconnected)",
				  a_bus,
				  m_buses_in[a_bus]->get_client(),
				  m_buses_in[a_bus]->get_port() );
	} else {
		snprintf( tmp, 59, "[%d] (unconnected)",
				  a_bus );
	}

	string ret = tmp;
	return ret;

}


void
mastermidibus::print()
{
    printf( "Available Buses\n");
    for ( int i=0; i<m_num_out_buses; i++ ){
	printf( "%s\n", m_buses_out[i]->m_name.c_str() );
    }
}


int
mastermidibus::get_num_out_buses()
{
    return m_num_out_buses;
}


int
mastermidibus::get_num_in_buses()
{
    return m_num_in_buses;
}

int
mastermidibus::poll_for_midi( )
{
    int ret = 0;
#ifdef HAVE_LIBASOUND
    ret = poll( m_poll_descriptors,
		 m_num_poll_descriptors,
		 1000);
#endif
    return ret;
}

bool
mastermidibus::is_more_input( ){

    lock();

    int size=0;

#ifdef HAVE_LIBASOUND
    size = snd_seq_event_input_pending(m_alsa_seq, 0);
#endif
    unlock();

    return ( size > 0 );
}


void
mastermidibus::port_start( int a_client, int a_port )
{
    lock();

#ifdef HAVE_LIBASOUND

    /* client info */
    snd_seq_client_info_t *cinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_get_any_client_info( m_alsa_seq, a_client, cinfo );

    /* port info */
    snd_seq_port_info_t *pinfo;

    /* fill pinfo */
    snd_seq_port_info_alloca(&pinfo);
    snd_seq_get_any_port_info( m_alsa_seq, a_client, a_port, pinfo );


    /* get its capability */
    int cap =  snd_seq_port_info_get_capability(pinfo);

    if ( snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo)){

        /* the outs */
        if ( (cap & (SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE ))
             == (SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE )
             && snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo)){


            bool replacement = false;
            int bus_slot = m_num_out_buses;

            for( int i=0; i< m_num_out_buses; i++ ){

                if( m_buses_out[i]->get_client() == a_client  &&
                    m_buses_out[i]->get_port() == a_port &&
                    m_buses_out_active[i] == false ){

                    replacement = true;
                    bus_slot = i;
                }
            }

            m_buses_out[bus_slot] =
                new midibus( snd_seq_client_id( m_alsa_seq ),
                             snd_seq_port_info_get_client(pinfo),
                             snd_seq_port_info_get_port(pinfo),
                             m_alsa_seq,
                             snd_seq_client_info_get_name(cinfo),
                             snd_seq_port_info_get_name(pinfo),
                             m_num_out_buses, m_queue );

            m_buses_out[bus_slot]->init_out();
            m_buses_out_active[bus_slot] = true;
            m_buses_out_init[bus_slot] = true;

            if ( !replacement ){
                m_num_out_buses++;
            }
        }

        /* the ins */
        if ( (cap & (SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_READ ))
             == (SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_READ )
             && snd_seq_client_id( m_alsa_seq ) != snd_seq_port_info_get_client(pinfo)){

            bool replacement = false;
            int bus_slot = m_num_in_buses;



            for( int i=0; i< m_num_in_buses; i++ ){

                if( m_buses_in[i]->get_client() == a_client  &&
                    m_buses_in[i]->get_port() == a_port &&
                    m_buses_in_active[i] == false ){

                    replacement = true;
                    bus_slot = i;
                }
            }

            //printf( "in [%d] [%d]\n", replacement, bus_slot );

            m_buses_in[bus_slot] =
                new midibus( snd_seq_client_id( m_alsa_seq ),
                             snd_seq_port_info_get_client(pinfo),
                             snd_seq_port_info_get_port(pinfo),
                             m_alsa_seq,
                             snd_seq_client_info_get_name(cinfo),
                             snd_seq_port_info_get_name(pinfo),
                             m_num_in_buses, m_queue);

            //m_buses_in[bus_slot]->init_in();
            m_buses_in_active[bus_slot] = true;
            m_buses_in_init[bus_slot] = true;

            if ( !replacement ){
                m_num_in_buses++;
            }
        }
    }

    /* end loop for clients */


    /* midi input */
    /* poll descriptors */

    /* get number of file descriptors */
    m_num_poll_descriptors = snd_seq_poll_descriptors_count(m_alsa_seq, POLLIN);

    /* allocate into */
    m_poll_descriptors = new pollfd[m_num_poll_descriptors];

    /* get descriptors */
    snd_seq_poll_descriptors(m_alsa_seq,
			     m_poll_descriptors,
			     m_num_poll_descriptors,
			     POLLIN);
#endif
    unlock();
}

void
mastermidibus::port_exit( int a_client, int a_port )
{
	lock();
#ifdef HAVE_LIBASOUND
	for( int i=0; i< m_num_out_buses; i++ ){

		if( m_buses_out[i]->get_client() == a_client  &&
			m_buses_out[i]->get_port() == a_port ){

			m_buses_out_active[i] = false;
		}
	}

        for( int i=0; i< m_num_in_buses; i++ ){

		if( m_buses_in[i]->get_client() == a_client  &&
			m_buses_in[i]->get_port() == a_port ){

			m_buses_in_active[i] = false;
		}
	}
#endif
	unlock();
}


bool
mastermidibus::get_midi_event( event *a_in )
{
    lock();
#ifdef HAVE_LIBASOUND
    snd_seq_event_t *ev;

    bool sysex = false;
    bool ret = false;

    /* temp for midi data */
    unsigned char buffer[0x1000];

    snd_seq_event_input(m_alsa_seq, &ev);

    if (! global_manual_alsa_ports )
    {
        switch(ev->type) {

            case SND_SEQ_EVENT_PORT_START:
                {
                    //printf("SND_SEQ_EVENT_PORT_START:    addr[%d:%d]\n",
                    //	   ev->data.addr.client, ev->data.addr.port );
                    port_start( ev->data.addr.client, ev->data.addr.port );
                    ret = true;
                    break;
                }

            case SND_SEQ_EVENT_PORT_EXIT:
                {
                    //printf("SND_SEQ_EVENT_PORT_EXIT:     addr[%d:%d]\n",
                    //	   ev->data.addr.client, ev->data.addr.port );
                    port_exit( ev->data.addr.client, ev->data.addr.port );
                    ret = true;
                    break;
                }

            case SND_SEQ_EVENT_PORT_CHANGE:
                {
                    //printf("SND_SEQ_EVENT_PORT_CHANGE:   addr[%d:%d]\n",
                    //	   ev->data.addr.client,
                    //	   ev->data.addr.port );
                    ret = true;
                    break;
                }

            default:
                break;

        }
    }

    if (ret) {
        unlock();
        return false;
    }

    /* alsa midi parser */
    snd_midi_event_t *midi_ev;
    snd_midi_event_new(sizeof(buffer), &midi_ev);

    long bytes = snd_midi_event_decode(midi_ev, buffer, sizeof(buffer), ev);

    if (bytes <= 0) {
        unlock();
        return false;
    }

    a_in->set_timestamp( ev->time.tick );
    a_in->set_status_midibus( buffer[0] );     // keep channel bit
    a_in->set_size( bytes );

    /* we will only get EVENT_SYSEX on the first
       packet of midi data, the rest we have
       to poll for */
    //if ( buffer[0] == EVENT_SYSEX ){
    if (0) {

        /* set up for sysex if needed */
        a_in->start_sysex( );
        sysex = a_in->append_sysex( buffer, bytes );
    }
    else {
        a_in->set_data( buffer[1], buffer[2] );

        // some keyboards send on's with vel 0 for off
        if ( a_in->get_status() == EVENT_NOTE_ON &&
             a_in->get_note_velocity() == 0x00 ){
            a_in->set_status( EVENT_NOTE_OFF );
        }

        sysex = false;
    }

    /* sysex messages might be more than one message */
    while (sysex) {

        snd_seq_event_input(m_alsa_seq, &ev);

        bytes = snd_midi_event_decode(midi_ev, buffer, sizeof(buffer), ev);

        if (bytes > 0)
            sysex = a_in->append_sysex( buffer, bytes );
        else
            sysex = false;
    }

    snd_midi_event_free( midi_ev );

#endif

    unlock();
    return true;
}

void
mastermidibus::set_sequence_input( bool a_state, sequence *a_seq )
{
    lock();

    m_seq = a_seq;
    m_dumping_input = a_state;

    unlock();
}
