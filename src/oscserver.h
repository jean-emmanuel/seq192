#ifndef __OSC_H
#define __OSC_H

#include <lo/lo.h>
#include <string>
#include <map>


class OSCServer
{

public:

    OSCServer(const int port);
    ~OSCServer();

    int _port;

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
