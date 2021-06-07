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


#ifndef SEQ192_USERFILE
#define SEQ192_USERFILE

#include "perform.h"
#include "configfile.h"
#include <fstream>
#include <string>
#include <list>

class userfile  : public configfile
{

 public:

    userfile( string a_name );
    ~userfile( );

    bool parse( perform *a_perf );
    bool write( perform *a_perf );

};


#endif
