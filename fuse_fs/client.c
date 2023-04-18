#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 9002

static int sockfd;

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

static int do_connect()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        return -1;
    }

    return 0;
}

static int remote_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    // Send the 'getattr' request to the server
    (void)fi;

    // char buf[256];
    // snprintf(buf, sizeof(buf), "getattr:%s", path);

    struct requests *request = malloc(sizeof(struct requests));
    struct getaddr_response *stat_res = malloc(sizeof(struct getaddr_response));

    strcpy(request->path, path);
    strcpy(request->type, "getattr");

    // printf("getattr: %s\n", buf);
    if (send(sockfd, request, sizeof(struct requests), 0) != sizeof(struct requests))
    {
        perror("send");
        return -errno;
    }

    // Receive the response from the server
    recv(sockfd, stat_res, sizeof(struct getaddr_response), 0);
    if (stat_res->bool == 0)
        printf("There is an error\n");
    else
        *stbuf = stat_res->stat;

    free(request);
    free(stat_res);

    return 0;
}

// Define other FUSE operations (e.g., readdir, open, read, write, etc.) using socket communication

static struct fuse_operations remote_ops = {
    .getattr = remote_getattr,
    // Define other FUSE operations here
};

int main(int argc, char *argv[])
{
    if (do_connect() < 0)
    {
        fprintf(stderr, "Failed to connect to server\n");
        return 1;
    }

    return fuse_main(argc, argv, &remote_ops, NULL);
}
