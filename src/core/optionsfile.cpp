//----------------------------------------------------------------------------
//
//  This file is part of seq192.
//
//  seq192 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq192 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq192; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#include <iostream>

#include "optionsfile.h"

extern Glib::ustring last_used_dir;

optionsfile::optionsfile(const Glib::ustring& a_name) :
    configfile( a_name )
{
}

optionsfile::~optionsfile()
{
}



bool
optionsfile::parse( perform *a_perf )
{

    /* open binary file */
    ifstream file ( m_name.c_str(), ios::in | ios::ate );

    if( ! file.is_open() )
        return false;

    /* run to start */
    file.seekg( 0, ios::beg );

    /* last used dir */
    line_after( &file, "[last-used-dir]" );
    //FIXME: check for a valid path is missing
    if (m_line[0] == '/')
        last_used_dir.assign(m_line);

    file.close();

    return true;
}


bool
optionsfile::write( perform *a_perf  )
{
    /* open binary file */

    ofstream file ( m_name.c_str(), ios::out | ios::trunc  );

    if( ! file.is_open() )
        return false;

    file << "#\n";
    file << "# Seq 24 Init File\n";
    file << "#\n\n\n";

    file << "\n\n\n[last-used-dir]\n\n"
         << "# Last used directory.\n"
         << last_used_dir << "\n\n";

    file.close();
    return true;
}
