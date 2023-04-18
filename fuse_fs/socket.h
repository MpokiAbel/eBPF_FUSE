#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define ENABLE_REMOTE 1
#define SERVER_PORT 9002

struct requests
{
    char path[256];
    char type[10];
    int flags;
};

struct getaddr_response
{
    int bool;
    struct stat stat;
};

int do_client_connect();

int do_server_connect();
