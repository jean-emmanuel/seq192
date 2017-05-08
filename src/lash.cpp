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

#include <string>
#include <sigc++/slot.h>
#include <gtkmm.h>

#include "lash.h"
#include "midifile.h"


lash::lash(int *argc, char ***argv)
{
#ifdef LASH_SUPPORT
    m_client = lash_init(lash_extract_args(argc, argv), PACKAGE_NAME,
        LASH_Config_File, LASH_PROTOCOL(2, 0));
    if (m_client == NULL) {
        fprintf(stderr, "Failed to connect to LASH.  Session management will not occur.\n");
    } else {
        lash_event_t* event = lash_event_new_with_type(LASH_Client_Name);
        lash_event_set_string(event, "Seq24");
        lash_send_event(m_client, event);
        printf("[Connected to LASH]\n");
    }
#endif // LASH_SUPPORT
}


void
lash::set_alsa_client_id(int id)
{
#ifdef LASH_SUPPORT
	lash_alsa_client_id(m_client, id);
#endif
}


void
lash::start(perform* perform)
{
#ifdef LASH_SUPPORT
	m_perform = perform;

    /* Process any LASH events every 250 msec (arbitrarily chosen interval) */
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &lash::process_events), 250);
#endif // LASH_SUPPORT
}


#ifdef LASH_SUPPORT

bool
lash::process_events()
{
    lash_event_t  *ev = NULL;
    //lash_config_t *conf = NULL;

    // Process events
    while ((ev = lash_get_event(m_client)) != NULL) {
        handle_event(ev);
        lash_event_destroy(ev);
    }

    return true;
}


void
lash::handle_event(lash_event_t* ev)
{
    LASH_Event_Type type   = lash_event_get_type(ev);
    const char      *c_str = lash_event_get_string(ev);
    std::string     str    = (c_str == NULL) ? "" : c_str;

    if (type == LASH_Save_File) {
        midifile f(str + "/seq24.mid");
        f.write(m_perform);
        lash_send_event(m_client, lash_event_new_with_type(LASH_Save_File));
    } else if (type == LASH_Restore_File) {
        midifile f(str + "/seq24.mid");
        f.parse(m_perform, 0);
        lash_send_event(m_client, lash_event_new_with_type(LASH_Restore_File));
    } else if (type == LASH_Quit) {
        m_client = NULL;
        Gtk::Main::quit();
    } else {
		fprintf(stderr, "Warning:  Unhandled LASH event.\n");
	}
}


void
lash::handle_config(lash_config_t* conf)
{
    const char *key     = NULL;
    const void *val     = NULL;
    size_t     val_size = 0;

    key      = lash_config_get_key(conf);
    val      = lash_config_get_value(conf);
    val_size = lash_config_get_value_size(conf);
}


#endif // LASH_SUPPORT
