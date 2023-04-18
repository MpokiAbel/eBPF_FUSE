#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "socket.h"

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

static int handle_open(const char *path, int flags, int *fd)

{

    *fd = open(path, flags);

    if (*fd == -1)
        return -errno;

    return 0;
}

void handle_request(int connfd, struct requests *request)
{

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
        free(stbuf);
    }
    // else if (strcmp(request->type, "opendir") == 0)
    // {
    //     return 0;
    // }
    // else if (strcmp(request->type, "readdir") == 0)
    // {
    //     return 0;
    // }

    // else if (strcmp(request->type, "readlink") == 0)
    // {
    //     return 0;
    // }
    // else if (strcmp(request->type, "releasedir") == 0)
    // {
    //     return 0;
    // }
    else if (strcmp(request->type, "open") == 0)
    {
        int res[2];
        int ret = handle_open(request->path, request->flags, &res[1]);

        printf("open: %s\n", request->path);
        if (ret != 0)
        {
            printf("handle_getatt failed\n");
            res[0] = 0;
        }
        else
            res[0] = 1;

        send(connfd, res, sizeof(res), 0);
        // return 0;
    }
    // else if (strcmp(request->type, "read") == 0)
    // {
    //     return 0;
    // }
    // else
    //     return -1;
}

int main()
{
    struct requests *recv_request = malloc(sizeof(struct requests));
    int n;
    int sockfd = do_server_connect();

    // ToDo: make it accept multiple requests i.e connections
    int connfd = accept(sockfd, NULL, NULL);
    if (connfd < 0)
    {
        perror("accept");
        return -1;
    }

    // Loop to handle connection requests
    while (1)
    {
        // receive data in a buffer
        n = recv(connfd, recv_request, sizeof(struct requests), 0);
        if (n < 0)
        {
            perror("recv");
            continue;
        }

        handle_request(connfd, recv_request);
        memset(recv_request, 0, sizeof(struct requests));
    }
    close(connfd);
    free(recv_request);
    return 0;
}
