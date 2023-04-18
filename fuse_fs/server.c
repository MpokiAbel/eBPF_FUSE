#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define SERVER_PORT 9002

struct requests
{
    char path[256];
    char type[10];
};

struct getaddr_response
{
    int bool;
    struct stat stat;
};

static int
handle_getattr(const char *path, struct stat *stbuf)
{
    // Get the attributes for the specified file or directory
    int res = lstat(path, stbuf);
    if (res < 0)
    {
        perror("lstat");
        return -errno;
    }

    return 0;
}

// Define handlers for other FUSE operations (e.g., readdir, open, read, write, etc.)

// static void handle_request(int sockfd)
// {
//     // Receive the request from the client

// // Handle other FUSE operations here
// }

int main()
{

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Set necessary infos
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    // Listen to connection
    listen(sockfd, 5);

    int connfd = accept(sockfd, NULL, NULL);
    if (connfd < 0)
    {
        perror("accept");
        return -1;
    }
    char recvbuf[1024];
    // char sendbuf[1024];

    int n;
    // char *request;
    struct requests *request;

    // Loop to handle connection requests
    while (1)
    {

        // handle_request(connfd);

        n = recv(connfd, recvbuf, sizeof(recvbuf), 0);
        if (n < 0)
        {
            perror("recv");
            continue;
        }

        // Parse the request and call the appropriate handler function
        request = (struct requests *)recvbuf;

        if (strcmp(request->type, "getattr") == 0)
        {
            struct getaddr_response *stbuf = malloc(sizeof(struct getaddr_response));

            int res = handle_getattr(request->path, &stbuf->stat);
            printf("getattr: %s\n", request->path);
            if (res < 0)
            {
                printf("handle_getatt failed\n");
                stbuf->bool = 0;
            }
            else
                stbuf->bool = 1;

            send(connfd, stbuf, sizeof(struct getaddr_response), 0);
        }
    }
    close(connfd);
    return 0;
}
