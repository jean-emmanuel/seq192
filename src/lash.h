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

#ifndef SEQ24_LASH
#define SEQ24_LASH

#ifdef __WIN32__
#include "configwin32.h"
#else
#include "config.h"
#endif

#include "perform.h"

#ifdef LASH_SUPPORT
#include <lash/lash.h>
#endif // LASH_SUPPORT

/* all the ifdef skeleton work is done in this class in such a way that any
 * other part of the code can use this class whether or not lash support is
 * actually built in (the functions will just do nothing) */

class lash
{
private:
#ifdef LASH_SUPPORT
    perform       *m_perform;
    lash_client_t *m_client;

    bool process_events();
    void handle_event(lash_event_t* conf);
    void handle_config(lash_config_t* conf);

#endif // LASH_SUPPORT

public:
    lash(int *argc, char ***argv);

	void set_alsa_client_id(int id);
    void start(perform* perform);
};


/* global lash driver, defined in seq24.cpp */
extern lash *lash_driver;


#endif // SEQ24_LASH
