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
#include "event.h"
#include "string.h"

event::event()
{
    m_timestamp = 0;
    m_status = EVENT_NOTE_OFF;
    m_data[0] = 0;
    m_data[1] = 0;

    m_sysex = NULL;

    m_linked = NULL;
    m_selected = false;
    m_marked = false;
    m_has_link = false;
    m_painted = false;
} 

event::~event()
{
  if ( m_sysex != NULL )
    delete[] m_sysex;
  
  m_sysex = NULL;
}

long 
event::get_timestamp()
{ 
    return m_timestamp; 
}

void 
event::set_timestamp( const unsigned long a_time )
{
    m_timestamp = a_time;
}

void 
event::mod_timestamp( unsigned long a_mod )
{
    m_timestamp %= a_mod;
}

void 
event::set_status( const char a_status  )
{
   /* bitwise AND to clear the channel portion of the status */
    if ( (unsigned char) a_status >= 0xF0 ){
      m_status = (char) a_status;
//      fprintf(stderr, ">=0xF0 m_status: %i | a_status: %i\n",m_status, a_status);
    }
    else
    {
//ORL fait de la merde
  //    fprintf(stderr,"a_status: %i| uc a_status %i \n",a_status, (unsigned char) a_status);
      m_status = (char) (a_status & EVENT_CLEAR_CHAN_MASK);
  //    m_status = (unsigned char) a_status;
//      fprintf(stderr, "<0xF0 m_status: %i | a_status: %i | EVENT_CLEAR_CHAN_MASK: %i\n",m_status,a_status,EVENT_CLEAR_CHAN_MASK);
// ORL a fini
    }
}

void 
event::set_status_midibus( const char a_status  )
{
   /* bitwise AND to clear the channel portion of the status */
    if ( (unsigned char) a_status >= 0xF0 ){
      m_status = (char) a_status;
//      fprintf(stderr, ">=0xF0 m_status: %i | a_status: %i\n",m_status, a_status);
    }
    else
    {
//ORL fait de la merde
  //    fprintf(stderr,"a_status: %i| uc a_status %i \n",a_status, (unsigned char) a_status);
//      m_status = (char) (a_status & EVENT_CLEAR_CHAN_MASK);
      m_status = (unsigned char) a_status;
//      fprintf(stderr, "<0xF0 m_status: %i | a_status: %i | EVENT_CLEAR_CHAN_MASK: %i\n",m_status,a_status,EVENT_CLEAR_CHAN_MASK);
// ORL a fini
    }
}

void 
event::make_clock( )
{
    m_status = (unsigned char) EVENT_MIDI_CLOCK;
}

void 
event::set_data( char a_D1  )
{
    m_data[0] = a_D1 & 0x7F;
    //ORL
    //fprintf(stderr,"m_data[0] %d | a_D1 %d | 0x7F %d | a_D1 & 0x7F %d\n",m_data[0],a_D1,0x7F,a_D1 & 0x7F);
}

void 
event::set_data( char a_D1, char a_D2 )
{
    m_data[0] = a_D1 & 0x7F;
    m_data[1] = a_D2 & 0x7F;
    // ORL
    //fprintf(stderr,"m_data[0] / [1] %d / %d | a_D1 / D2 %d / %d | 0x7F %d | a_D1 / D2 & 0x7F %d / %d\n",m_data[0],m_data[1],a_D1,a_D2,0x7F,a_D1 & 0x7F,a_D2 & 0x7F);

}

void 
event::increment_data2(void )
{
	m_data[1] = (m_data[1]+1) & 0x7F;
}

void 
event::decrement_data2(void )
{
	m_data[1] = (m_data[1]-1) & 0x7F;
}



void 
event::increment_data1(void )
{
	m_data[0] = (m_data[0]+1) & 0x7F;
}

void 
event::decrement_data1(void )
{
	m_data[0] = (m_data[0]-1) & 0x7F;
}


void 
event::get_data( unsigned char *D0, unsigned char *D1 )
{
    *D0 = m_data[0]; 
    *D1 = m_data[1];
}

unsigned char 
event::get_status( ) 
{ 
    return m_status; 
}


void 
event::start_sysex( void  )
{
  if ( m_sysex != NULL )
    delete[] m_sysex;

  m_sysex = NULL;
  m_size = 0;
}

bool
event::append_sysex( unsigned char *a_data, long a_size )
{
  bool ret = true;

  unsigned char *buffer = new unsigned char[m_size + a_size];

  /* copy old and append */
  memcpy(  buffer        , m_sysex, m_size );
  memcpy( &buffer[m_size], a_data, a_size );

  if ( m_sysex != NULL )
    delete[] m_sysex;

  m_size = m_size + a_size;
  m_sysex = buffer;

  for ( int i=0; i<a_size; i++ ){

    if ( a_data[i] == EVENT_SYSEX_END )
      ret = false;
  }
    
  return ret;

}


unsigned char *
event::get_sysex( void )
{
  return m_sysex;
}



void
event::set_size( long a_size )
{
  m_size = a_size;
}

long
event::get_size( void )
{
  return m_size;
}

void 
event::set_note_velocity( int a_vel )
{
    m_data[1] = a_vel & 0x7F; 
}

bool 
event::is_note_on()
{
    return (m_status == EVENT_NOTE_ON); 
}

bool 
event::is_note_off()
{
    return (m_status == EVENT_NOTE_OFF);
}

unsigned char 
event::get_note()
{
    return m_data[0];
}

void 
event::set_note( char a_note )
{
    m_data[0] = a_note & 0x7F;
}

unsigned char 
event::get_note_velocity()
{
    return m_data[1];
}


void 
event::print()
{
    printf( "[%06ld] [%04lX] %02X ",
	    m_timestamp,
	    m_size,
	    m_status );

    if ( m_status == EVENT_SYSEX ){

      for( int i=0; i<m_size; i++ ){

	if ( i%16 == 0 )
	  printf( "\n    " );

	printf( "%02X ", m_sysex[i] );

      }

      printf( "\n" );
    }
    else {

      printf( "%02X %02X\n",
	      m_data[0],
	      m_data[1] );
    }
}

int 
event::get_rank( void ) const
{
    switch ( m_status )
    {
        case EVENT_NOTE_OFF:
            return 0x100;
        case EVENT_NOTE_ON:
            return 0x090;

        case EVENT_AFTERTOUCH:
        case EVENT_CHANNEL_PRESSURE:
        case EVENT_PITCH_WHEEL: 
            return 0x050;
            
        case EVENT_CONTROL_CHANGE:
            return 0x010;
        case EVENT_PROGRAM_CHANGE:
            return 0x000;
        default:
            return 0;
    }
}

bool 
event::operator>( const event &a_rhsevent )
{
    if ( m_timestamp == a_rhsevent.m_timestamp )
    {
        return (get_rank() > a_rhsevent.get_rank());
    }
    else
    {
        return (m_timestamp > a_rhsevent.m_timestamp);
    }
}


bool 
event::operator<( const event &a_rhsevent )
{ 
    if ( m_timestamp == a_rhsevent.m_timestamp )
    {
        return (get_rank() < a_rhsevent.get_rank());
    }
    else
    {
        return (m_timestamp < a_rhsevent.m_timestamp);
    }
}

bool 
event::operator<=( const unsigned long &a_rhslong )
{ 
    return (m_timestamp <= a_rhslong); 
}   



bool 
event::operator>( const unsigned long &a_rhslong )
{ 
    return (m_timestamp > a_rhslong); 
}   

void 
event::link( event *a_event )
{
    m_has_link = true;
    m_linked = a_event;
}

event*
event::get_linked( )
{
    return m_linked;
}

bool
event::is_linked( )
{
    return m_has_link;
}

void
event::clear_link( )
{
    m_has_link = false;
}

void 
event::select( )
{
    m_selected = true;
}

void 
event::unselect( )
{
    m_selected = false;
}

bool 
event::is_selected( )
{
    return m_selected;
}
void 
event::paint( )
{
    m_painted = true;
}

void 
event::unpaint( )
{
    m_painted = false;
}

bool 
event::is_painted( )
{
    return m_painted;
}
    
void 
event::mark( )
{
    m_marked = true;
}

void 
event::unmark( )
{
    m_marked = false;
}

bool 
event::is_marked( )
{
    return m_marked;
}
