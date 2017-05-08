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
#include "mutex.h"

const pthread_mutex_t smutex::recmutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
const pthread_cond_t condition_var::cond  = PTHREAD_COND_INITIALIZER;

smutex::smutex( )
{
    m_mutex_lock = recmutex;
}

void 
smutex::lock( )
{
    pthread_mutex_lock( &m_mutex_lock );
}


void 
smutex::unlock( )
{      
    pthread_mutex_unlock( &m_mutex_lock );
}

condition_var::condition_var( )
{
    m_cond = cond;
}


void
condition_var::signal( )
{
    pthread_cond_signal( &m_cond );
}

void
condition_var::wait( )
{
    pthread_cond_wait( &m_cond, &m_mutex_lock );
}


