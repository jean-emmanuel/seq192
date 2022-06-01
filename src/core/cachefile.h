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


#ifndef SEQ192_CACHEFILE
#define SEQ192_CACHEFILE

#include <fstream>
#include <string>
#include <list>

using namespace std;

class CacheFile
{


    public:

        CacheFile(const std::string& a_name);
        ~CacheFile();

        bool parse();
        bool write();

    private:

        std::string m_name;


};


#endif
