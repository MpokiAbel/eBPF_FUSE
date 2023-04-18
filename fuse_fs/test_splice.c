#include <unistd.h>

#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <fcntl.h>

int main()
{
    int pipefd[2];
    int ret = pipe(pipefd);
    ssize_t nwritten = splice(pipefd[0], NULL, STDOUT_FILENO, NULL, 4096, 0);
    return 0;
}
