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

#ifndef SEQ24_MUTEX
#define SEQ24_MUTEX

#include "globals.h"
#include <pthread.h>

class smutex {
    
private:

    static const pthread_mutex_t recmutex;
    
protected:
    
    /* mutex lock */
    pthread_mutex_t  m_mutex_lock;
    
public:
    
    smutex();

    void lock();
    void unlock();

};

class condition_var : public smutex {

private:

    static const pthread_cond_t cond;

    pthread_cond_t m_cond;

public:

    condition_var();
    
    void wait();
    void signal();
    
};

#endif
