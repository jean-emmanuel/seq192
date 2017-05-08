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

#include "midibus_portmidi.h"

#ifdef __WIN32__

midibus::midibus( char a_id, char a_pm_num, const char *a_client_name )
{
    /* set members */
    m_pm_num = a_pm_num;
    m_id = a_id;
    m_clock_type = e_clock_off;
    m_inputing = false;

    /* copy names */
    char tmp[60];
    snprintf( tmp, 59, "[%d] %s",
	      m_id,
	      a_client_name );

    m_name = tmp;
    m_pms = NULL;
}

int
midibus::poll_for_midi( )
{
    if ( m_pm_num )
    {
        PmError err = Pm_Poll( m_pms);
        
        if ( err == FALSE )
        {
            return 0;     
        }
        if ( err == TRUE )
        {
            return 1;
        }
    
        printf( "Pm_Poll: %s\n", Pm_GetErrorText( err ));
    }
    
    return 0;

}

int midibus::m_clock_mod = 16 * 4;

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
    PmError err = Pm_OpenOutput( &m_pms, m_pm_num, NULL, 100, NULL, NULL, 0);
    
    if ( err != pmNoError )
    {
        printf( "Pm_OpenOutput: %s\n", Pm_GetErrorText( err ));
        return false;
    }
     
    return true;
}



bool midibus::init_in( )
{
    PmError err = Pm_OpenInput( &m_pms, m_pm_num, NULL, 100, NULL, NULL);

    if ( err != pmNoError )
    {
        printf( "Pm_OpenInput: %s\n", Pm_GetErrorText( err ));
        return false;
    }
    
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
    if ( m_pms )
        Pm_Close( m_pms );
    m_pms = NULL;
}


/* takes an native event, encodes to alsa event, 
   puts it in the queue */
void 
midibus::play( event *a_e24, unsigned char a_channel )
{
    lock();

    PmEvent event;
    event.timestamp = 0;

    /* temp for midi data */
	unsigned char buffer[3];
		
	/* fill buffer and set midi channel */
	buffer[0] = a_e24->get_status();
	buffer[0] += (a_channel & 0x0F);
	a_e24->get_data( &buffer[1], &buffer[2] );

    event.message = Pm_Message(buffer[0], buffer[1], buffer[2]);
    
    /*PmError err = */Pm_Write( m_pms, &event, 1 );

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

    unlock();
}


// flushes our local queue events out into ALSA
void 
midibus::flush()
{

} 


void 
midibus::init_clock( long a_tick )
{
                   
    if ( m_clock_type == e_clock_pos && a_tick != 0)
    {
        continue_from( a_tick );
    }
    else 
    if ( m_clock_type == e_clock_mod || a_tick == 0)
    {
        start();

        long clock_mod_ticks = (c_ppqn / 4) * m_clock_mod;
        long leftover = ( a_tick % clock_mod_ticks );
        long starting_tick = a_tick - leftover;

        /* was there anything left?, then wait for next beat (16th note) to start clocking */
        if ( leftover > 0)
        {
            starting_tick += clock_mod_ticks;
        }
        //printf ( "continue_from leftover[%ld] starting_tick[%ld]\n", leftover, starting_tick );

        m_lasttick = starting_tick - 1;    

    }
}

void
midibus::continue_from( long a_tick )
{
                      
    /* tell the device that we are going to start at a certain position */
    long pp16th = (c_ppqn / 4);

    long leftover = ( a_tick % pp16th );
    long beats = ( a_tick / pp16th );

    long starting_tick = a_tick - leftover;

    /* was there anything left?, then wait for next beat (16th note) to start clocking */
    if ( leftover > 0)
    {
        starting_tick += pp16th;
    }
    //printf ( "continue_from leftover[%ld] starting_tick[%ld]\n", leftover, starting_tick );

    m_lasttick = starting_tick - 1;    

    if ( m_clock_type != e_clock_off )
    {

        PmEvent event;
        event.timestamp = 0;
        event.message = Pm_Message( EVENT_MIDI_CONTINUE, 0,0 );
        Pm_Write( m_pms, &event, 1 );
        event.message = Pm_Message( EVENT_MIDI_SONG_POS, (beats & 0x3F80 >> 7), (beats & 0x7F) );
        Pm_Write( m_pms, &event, 1 );
    }

}


/* gets it a runnin */
void 
midibus::start()
{

    m_lasttick = -1;
    
    if ( m_clock_type != e_clock_off ){
          
	    PmEvent event;
        event.timestamp = 0;
        event.message = Pm_Message( EVENT_MIDI_START, 0,0 );
        Pm_Write( m_pms, &event, 1 );

    }
}


void 
midibus::set_clock( clock_e a_clock_type )
{
    m_clock_type = a_clock_type;
}


clock_e
midibus::get_clock( )
{
    return m_clock_type;
}




void 
midibus::set_input( bool a_inputing )
{
    if ( m_inputing != a_inputing )
    {
        m_inputing = a_inputing;
    }    
}


bool 
midibus::get_input( )
{
    return m_inputing;
}



void 
midibus::stop()
{

    m_lasttick = -1;

    if ( m_clock_type != e_clock_off )
    {   
	    PmEvent event;
        event.timestamp = 0;
        event.message = Pm_Message( EVENT_MIDI_STOP, 0,0 );
        Pm_Write( m_pms, &event, 1 );	
	
    }

}




// generates midi clock
void
midibus::clock( long a_tick )
{

    lock();

    if ( m_clock_type != e_clock_off ){

    	bool done = false;
    	
    	long uptotick = a_tick;
    	
    	if ( m_lasttick >= uptotick )
    	    done = true;
    	
    	while ( !done ){
    	    
    	    m_lasttick++;
    	    
    	    if ( m_lasttick >= uptotick )
    		done = true;
    	    
    	    /* tick time? */
    	    if ( m_lasttick % ( c_ppqn / 24 ) == 0 )
            {
    		
    	    PmEvent event;
            event.timestamp = 0;
            event.message = Pm_Message( EVENT_MIDI_CLOCK, 0,0 );
            Pm_Write( m_pms, &event, 1 );	
    
    
            }
    	}
    }

    unlock();
}




void 
mastermidibus::lock( )
{
   // printf( "mastermidibus::lock()\n" );
   m_mutex.lock();
}


void 
mastermidibus::unlock( )
{   
   // printf( "mastermidibus::unlock()\n" );
   m_mutex.unlock();
}



/* gets it a runnin */
void 
mastermidibus::start()
{
    lock();

    
    for ( int i=0; i < m_num_out_buses; i++ )
	    m_buses_out[i]->start();

     unlock();
}


/* gets it a runnin */
    void
mastermidibus::continue_from( long a_tick)
{
    lock();

    for ( int i=0; i < m_num_out_buses; i++ )
        m_buses_out[i]->continue_from( a_tick );

    unlock();
}

void
mastermidibus::init_clock( long a_tick )
{
    lock();

    for ( int i=0; i < m_num_out_buses; i++ )
        m_buses_out[i]->init_clock( a_tick );

    unlock();
}    

void 
mastermidibus::stop()
{
    lock();

    for ( int i=0; i < m_num_out_buses; i++ )
        m_buses_out[i]->stop();

    unlock();
}


// generates midi clock
void
mastermidibus::clock( long a_tick )
{
    lock();
    
    for ( int i=0; i < m_num_out_buses; i++ )
	    m_buses_out[i]->clock( a_tick );
    
    unlock();
}

void 
mastermidibus::set_ppqn( int a_ppqn )
{
    lock();

    m_ppqn = a_ppqn;

    unlock();
}


void 
mastermidibus::set_bpm( int a_bpm )
{
    lock();

    m_bpm = a_bpm;

    unlock();
}

// flushes our local queue events out into ALSA
void 
mastermidibus::flush()
{

} 



/* fills the array with our buses */
mastermidibus::mastermidibus()
{
    /* temp return */
    //int ret;
    
    /* set initial number buses */
    m_num_out_buses = 0;
    m_num_in_buses = 0;

    for( int i=0; i<c_maxBuses; ++i ){
        m_buses_in_active[i] = false;	
        m_buses_out_active[i] = false;
        m_buses_in_init[i] = false;	
        m_buses_out_init[i] = false;

        m_init_clock[i] = e_clock_off;
        m_init_input[i] = false;
    }
    
    Pm_Initialize();

}

void
mastermidibus::init( )
{

    int num_devices = Pm_CountDevices( );

    const PmDeviceInfo* dev_info = NULL;
    
    for ( int i=0; i<num_devices; ++i )
    {
        dev_info = Pm_GetDeviceInfo( i );        
        
        printf( "[0x%x] [%s] [%s] input[%d] output[%d]\n",
            i, dev_info->interf, dev_info->name,
            dev_info->input, dev_info->output );
            
        if ( dev_info->output )
        {
            m_buses_out[m_num_out_buses] = 
                new midibus( m_num_out_buses, i, dev_info->name );

            if ( m_buses_out[m_num_out_buses]->init_out() )
            {
                m_buses_out_active[m_num_out_buses] = true;		
                m_buses_out_init[m_num_out_buses] = true;

                m_num_out_buses++;
            }
            else
            {
                delete m_buses_out[m_num_out_buses];
            }
        }	
        
        if ( dev_info->input )
        {
            m_buses_in[m_num_in_buses] = 
                new midibus( m_num_in_buses, i, dev_info->name );

            if ( m_buses_in[m_num_in_buses]->init_in())
            {

                m_buses_in_active[m_num_in_buses] = true;		
                m_buses_in_init[m_num_in_buses] = true;

                m_num_in_buses++;
            }
            else
            {
                delete  m_buses_in[m_num_in_buses];   
            }
        }	        
    }        


    set_bpm( c_bpm );
    set_ppqn( c_ppqn );

    /* midi input */
    /* poll descriptors */

    set_sequence_input( false, NULL );

    for ( int i=0; i<m_num_out_buses; i++ )
        set_clock(i,m_init_clock[i]);

    for ( int i=0; i<m_num_in_buses; i++ )
        set_input(i,m_init_input[i]);

}
      
mastermidibus::~mastermidibus()
{
    for ( int i=0; i<m_num_out_buses; i++ )
	    delete m_buses_out[i];
    for ( int i=0; i<m_num_in_buses; i++ )
	    delete m_buses_in[i];
        	    
    Pm_Terminate();

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
mastermidibus::set_clock( unsigned char a_bus, clock_e a_clock_type )
{
    lock();
    if ( a_bus < c_maxBuses ){
        m_init_clock[a_bus] = a_clock_type;
    }
    if ( m_buses_out_active[a_bus] && a_bus < m_num_out_buses ){
        m_buses_out[a_bus]->set_clock( a_clock_type );
    }
    unlock();
}

clock_e 
mastermidibus::get_clock( unsigned char a_bus )
{
	if ( m_buses_out_active[a_bus] && a_bus < m_num_out_buses ){
		return m_buses_out[a_bus]->get_clock();
	}
	return e_clock_off;
}

void 
midibus::set_clock_mod( int a_clock_mod )
{
    if (a_clock_mod != 0 )
        m_clock_mod = a_clock_mod;
}

int 
midibus::get_clock_mod( void )
{
    return m_clock_mod;
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
	return "error...";
}


string
mastermidibus::get_midi_in_bus_name( int a_bus )
{
	if ( m_buses_in_active[a_bus] && a_bus < m_num_in_buses ){
		return m_buses_in[a_bus]->get_name();
	}
	return "error...";
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
    //int ret = 0;

    while(1)
    {
        for ( int i=0; i<m_num_in_buses; i++ )
        {
            if( m_buses_in[i]->poll_for_midi( ))
            {
                return 1;    
            }
        }
        
        Sleep(1);
        
        return 0;
    }
}

bool 
mastermidibus::is_more_input( ){
    
    lock();
    
    int size=0;
    
    for ( int i=0; i<m_num_in_buses; i++ )
    {
        if( m_buses_in[i]->poll_for_midi( ))
        {
            size = 1;
        }
    }

    unlock();

    return ( size > 0 );
}





bool
mastermidibus::get_midi_event( event *a_in )
{
    lock();
    
    bool ret = false;
    PmEvent event;
    PmError err;

    for ( int i=0; i<m_num_in_buses; i++ )
    {
        if( m_buses_in[i]->poll_for_midi( ))
        {
            err = Pm_Read( m_buses_in[i]->m_pms, &event, 1 );
            if ( err < 0 )
            {
                printf( "Pm_Read: %s\n", Pm_GetErrorText( err )); 
            }
            
            if ( m_buses_in[i]->m_inputing )
                ret = true;
        }
    }

    if( !ret ){
        unlock();
        return false;
    }
    
    
    // ORL est couillu
    a_in->set_status_midibus( Pm_MessageStatus(event.message));
//    fprintf(stderr,"STATUS MIDIBUS EVENT MESSAGE %d \n",Pm_MessageStatus(event.message))
    // ORL a fini
    a_in->set_size( 3 );
    a_in->set_data( Pm_MessageData1(event.message), Pm_MessageData2(event.message) );
        
    // some keyboards send on's with vel 0 for off
    if ( a_in->get_status() == EVENT_NOTE_ON &&
         a_in->get_note_velocity() == 0x00 ){
         a_in->set_status_midibus( EVENT_NOTE_OFF );
    }
    
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

#endif
