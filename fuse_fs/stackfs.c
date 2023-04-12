#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

static const char *base_dir = "/";

int stackfs__getattr(const char *path, struct stat *stat)
{
    int res;
    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);
    res = lstat(full_path, stat);
    if (res == -1)
        return -errno;

    return 0;
}

int stackfs__open(const char *path, struct fuse_file_info *fi)
{
    int fd;
    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);

    fd = open(full_path, fi->flags);

    if (fd == -1)
        return -errno;
    fi->fh = fd;
    return 0;
}

int stackfs__opendir(const char *path, struct fuse_file_info *fi)
{

    DIR *dir;
    int ret;
    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);

    // Open directory
    dir = opendir(full_path);
    if (dir == NULL)
    {
        ret = -errno;
        printf("Failed to open directory %s: %s\n", path, strerror(errno));
        return ret;
    }

    // Store directory handle in fuse_file_info
    fi->fh = (intptr_t)dir;

    return 0;
}

int stackfs__read(const char *path, char *buff, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int res;
    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);

    res = pread(fi->fh, buff, size, offset);
    if (res == -1)
        res = -errno;

    return res;
}

int stackfs__readdir(const char *path, void *buff, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);

    dp = opendir(full_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;

        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buff, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

int stackfs__readlink(const char *path, char *buff, size_t size)
{
    int ret;

    char full_path[PATH_MAX];

    sprintf(full_path, "%s%s", base_dir, path);
    printf("Path: %s", full_path);

    ret = readlink(full_path, buff, size - 1);
    if (ret == -1)
        return -errno;

    buff[ret] = '\0';

    return 0;
}

static struct fuse_operations stackfs__op = {
    .getattr = stackfs__getattr,
    .opendir = stackfs__opendir,
    .open = stackfs__open,
    .read = stackfs__read,
    .readdir = stackfs__readdir,
    .readlink = stackfs__readlink,

};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &stackfs__op, NULL);
}