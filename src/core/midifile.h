// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SEQ192_MIDIFILE
#define SEQ192_MIDIFILE

#include "perform.h"
#include <fstream>
#include <string>
#include <list>

class midifile
{

 private:

    int m_pos;
    Glib::ustring m_name;

    /* holds our data */
    unsigned char *m_d;

    list<unsigned char> m_l;

    unsigned long read_long();
    unsigned short read_short();
    unsigned long read_var();

    void write_long( unsigned long );
    void write_short( unsigned short );

 public:

    midifile(const Glib::ustring&);

    ~midifile();

    bool parse( perform *a_perf, int a_screen_set );
    bool write( perform *a_perf, int a_screen_set, int a_sequence);

};


#endif
