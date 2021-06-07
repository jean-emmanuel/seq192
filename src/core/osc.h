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

#ifndef SEQ192_OSC
#define SEQ192_OSC

#include <lo/lo.h>
#include <string>
#include <map>


class OSCServer
{

public:

    OSCServer(const char* port);
    ~OSCServer();

    int protocol;


    void start();
    void stop();

    void add_method (const char* path, const char* types, lo_method_handler h, void* user_data = NULL);
    void send_json(const char* address, const char *path, const char* json);

    lo_server_thread serverThread;
    lo_server server;

    static void error(int num, const char *msg, const char *path)
    {
        printf("liblo server error %d in path %s: %s\n", num, path, msg);
        fflush(stdout);
    }

};

#endif
