#include "oscserver.h"

OSCServer::OSCServer( const int port )
{
    std::string s = std::to_string(port);
    char const *portstr = s.c_str();
    serverThread = lo_server_thread_new( portstr, error );
	server = lo_server_thread_get_server( serverThread );
}

OSCServer::~OSCServer()
{
    if( serverThread )
        lo_server_thread_free( serverThread );
}

void OSCServer::start()
{
    lo_server_thread_start( serverThread );
}


void OSCServer::stop()
{
    lo_server_thread_stop( serverThread );
}


void OSCServer::add_method( const char* path, const char* types, lo_method_handler h, void* user_data)
{
    lo_server_thread_add_method( serverThread, path, types, h, user_data );
}

void OSCServer::send_json( const char* address, const char* json)
{

    lo_address lo_add = lo_address_new_from_url(address);
    if (lo_add != NULL) {
        lo_send_from(lo_add, server, LO_TT_IMMEDIATE, "/status", "s", json);
        lo_address_free(lo_add);
    }

}
