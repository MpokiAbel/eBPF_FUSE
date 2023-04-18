#define FUSE_USE_VERSION 31
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <fuse.h>
#include "socket.h"

static int sockfd;
// static const char *base_dir = "/home/mpoki/Documents/M2_MOSIG/Stage/fuse_fs/root";

int stackfs__getattr(const char *path, struct stat *stat, struct fuse_file_info *fi)
{
    int res;
    (void)fi;

    // char full_path[PATH_MAX];
    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("getattr: %s\n", full_path);

    if (ENABLE_REMOTE)
    {
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
            *stat = stat_res->stat;

        free(request);
        free(stat_res);
    }
    else
    {

        res = lstat(path, stat);
        if (res == -1)
            return -errno;
    }

    return 0;
}

int stackfs__open(const char *path, struct fuse_file_info *fi)
{
    int fd;
    // char full_path[PATH_MAX];
    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("open: %s\n", full_path);

    if (ENABLE_REMOTE)
    {
        struct requests *request = malloc(sizeof(struct requests));
        int res[2];

        strcpy(request->path, path);
        strcpy(request->type, "open");
        request->flags = fi->flags;

        if (send(sockfd, request, sizeof(struct requests), 0) != sizeof(struct requests))
        {
            perror("send");
            return -errno;
        }

        recv(sockfd, res, sizeof(res), 0);
        if (res[0] == 0)
            printf("There is an error\n");
        else
            fi->fh = res[1];

        free(request);
    }
    else
    {

        fd = open(path, fi->flags);

        if (fd == -1)
            return -errno;
        fi->fh = fd;
    }

    return 0;
}

int stackfs__opendir(const char *path, struct fuse_file_info *fi)
{

    DIR *dir;
    int ret;
    // char full_path[PATH_MAX];

    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("opendir: %s \n", full_path);

    // Open directory
    if (ENABLE_REMOTE)
    {
    }
    else
    {
        dir = opendir(path);
        if (dir == NULL)
        {
            ret = -errno;
            printf("Failed to open directory %s: %s\n", path, strerror(errno));
            return ret;
        }

        // Store directory handle in fuse_file_info
        fi->fh = (intptr_t)dir;
    }

    return 0;
}

int stackfs__read(const char *path, char *buff, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // int res;

    // Create the pipe end points i.e 0-read and 1-write
    // int pipefd[2];
    // pipe(pipefd);

    // ssize_t nwritten = splice(fi->fh, offset, pipefd[1], NULL, size, 1);
    // if (nwritten == -1)
    // {
    //     res = -errno;
    // }

    // ssize_t nread = splice(pipefd[0], NULL, STDOUT_FILENO, NULL, nwritten, 1);
    // if (nread == -1)
    // {
    //     res = -errno;
    // }

    // memcpy(buff, read_buf, nread);
    return 0;

    // char full_path[PATH_MAX];

    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("read: %s\n", path);

    // res = pread(fi->fh, buff, size, offset);
    // if (res == -1)
    //     res = -errno;

    // return res;
}

int stackfs__readdir(const char *path, void *buff, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    // char full_path[PATH_MAX];

    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("readdir: %s\n", path);
    if (ENABLE_REMOTE)
    {
    }
    else
    {
        dp = opendir(path);
        if (dp == NULL)
            return -errno;

        while ((de = readdir(dp)) != NULL)
        {
            struct stat st;

            memset(&st, 0, sizeof(st));
            st.st_ino = de->d_ino;
            st.st_mode = de->d_type << 12;

            if (filler(buff, de->d_name, &st, 0, FUSE_FILL_DIR_PLUS))
                break;
        }
    }

    return 0;
}

int stackfs__readlink(const char *path, char *buff, size_t size)
{
    int ret;

    // char full_path[PATH_MAX];

    // sprintf(full_path, "%s%s", base_dir, path);
    // printf("readlink: %s\n", path);
    if (ENABLE_REMOTE)
    {
    }
    else
    {
        ret = readlink(path, buff, size - 1);
        if (ret == -1)
            return -errno;

        buff[ret] = '\0';
    }
    return 0;
}

int stackfs__read_buf(const char *path, struct fuse_bufvec **bufp,
                      size_t size, off_t off, struct fuse_file_info *fi)
{
    (void)path;
    struct fuse_bufvec *buf;

    buf = malloc(sizeof(struct fuse_bufvec));

    if (buf == NULL)
        return -ENOMEM;

    *buf = FUSE_BUFVEC_INIT(size);

    buf->buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
    buf->buf[0].fd = fi->fh;
    buf->buf[0].pos = off;

    *bufp = buf;

    // fuse_buf_copy();
    return 0;
}

int stackfs__releasedir(const char *path, struct fuse_file_info *fi)
{
    if (ENABLE_REMOTE)
    {
    }
    else
    {
        closedir((DIR *)fi->fh);
    }
    return 0;
}

static struct fuse_operations stackfs__op = {
    .getattr = stackfs__getattr,
    .opendir = stackfs__opendir,
    .open = stackfs__open,
    .read = stackfs__read,
    .readdir = stackfs__readdir,
    .readlink = stackfs__readlink,
    // .read_buf = stackfs__read_buf,
    .releasedir = stackfs__releasedir,

};

int main(int argc, char *argv[])
{

    if (ENABLE_REMOTE)
    {
        int retries = 1;

        while ((sockfd = do_client_connect()) < 0 && retries < 11)
        {
            fprintf(stderr, "Failed to connect to server, retry %d\n", retries);
            sleep(1);
            retries++;
        }
        if (retries > 10)
            return 1;
    }

    return fuse_main(argc, argv, &stackfs__op, NULL);
}
