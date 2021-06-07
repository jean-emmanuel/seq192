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


#ifndef SEQ192_CONFIGFILE
#define SEQ192_CONFIGFILE

#include "perform.h"
#include <fstream>
#include <string>
#include <list>

class configfile
{

 protected:

    int m_pos;
    Glib::ustring m_name;

    /* holds our data */
    unsigned char *m_d;

    list<unsigned char> m_l;

    char m_line[1024];

    bool m_done;

    void next_data_line( ifstream *a_file);
    void line_after( ifstream *a_file, string a_tag);

 public:

    configfile(const Glib::ustring& a_name);
    virtual ~configfile();

    virtual bool parse( perform *a_perf ) = 0;
    virtual bool write( perform *a_perf ) = 0;

};


#endif
