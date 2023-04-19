#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include "socket.h"

static void
handle_getattr(int connfd, const char *path)
{
    struct server_response stbuf;

    int res = lstat(path, &stbuf.stat);

    printf("getattr: %s\n", path);
    if (res < 0)
    {
        printf("handle_getatt failed\n");
        stbuf.bool = 0;
    }
    else
        stbuf.bool = 1;

    send(connfd, &stbuf, sizeof(stbuf), 0);
}

static void handle_open(int connfd, const char *path, int flags)

{
    int res[2] = {0};

    res[1] = open(path, flags);
    printf("open: %s\n", path);
    if (res[1] >= 0)
        res[0] = 1;

    send(connfd, res, sizeof(res), 0);
}

static void handle_opendir(int connfd, const char *path)
{
    DIR *dir = opendir(path);
    printf("opendir: %s\n", path);
    if (dir == NULL)
        printf("Opendir failed on the server");
    send(connfd, dir, sizeof(dir), 0);
}

static void handle_readdir(int connfd, const char *path)
{
    DIR *dir;
    struct dirent *de;
    struct server_response *res;
    int count;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    while ((de = readdir(dir)) != NULL)
    {
        count++;
    }
    closedir(dir);

    dir = opendir(path);
    res = malloc(count * sizeof(struct server_response));

    while ((de = readdir(dir)) != NULL)
    {
        strcpy(res->path, de->d_name);
        res->stat.st_ino = de->d_ino;
        res->stat.st_mode = de->d_type << 12;
    }

    if (errno != 0)
        res->bool = 1;
    send(connfd, res, count * sizeof(struct server_response), 0);
    closedir(dir);
    free(res);
}

void handle_readlink(int connfd, const char *path, size_t size)
{
    char buff[size];

    int ret = readlink(path, buff, size - 1);

    buff[ret] = '\0';

    send(connfd, buff, size, 0);
}

void handle_releasedir(int connfd, uint64_t fh)
{

    int ret = closedir((DIR *)fh);

    send(connfd, &ret, sizeof(ret), 0);
}

void handle_request(int connfd, struct requests *request)
{

    if (strcmp(request->type, "getattr") == 0)
    {
        handle_getattr(connfd, request->path);
    }
    else if (strcmp(request->type, "opendir") == 0)
    {
        handle_opendir(connfd, request->path);
    }
    else if (strcmp(request->type, "readdir") == 0)
    {
        handle_readdir(connfd, request->path);
    }

    else if (strcmp(request->type, "readlink") == 0)
    {
        handle_readlink(connfd, request->path, request->size);
    }

    else if (strcmp(request->type, "releasedir") == 0)
    {
        handle_releasedir(connfd, request->fh);
    }

    else if (strcmp(request->type, "open") == 0)
    {
        handle_open(connfd, request->path, request->flags);
    }
    // else if (strcmp(request->type, "read") == 0)
    // {
    //     return 0;
    // }
    else
        printf("Not implemented");
}

int main()
{
    struct requests recv_request ;
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
        n = recv(connfd, &recv_request, sizeof(struct requests), 0);
        if (n < 0)
        {
            perror("recv");
            continue;
        }

        // handle_request(connfd, &recv_request);
        memset(&recv_request, 0, sizeof(struct requests));
    }
    close(connfd);
    return 0;
}
