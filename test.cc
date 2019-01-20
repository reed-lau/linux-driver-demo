#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd = open("/dev/topics", O_RDWR);
    printf("fd = %d %s\n", fd, strerror(errno));

    ssize_t ret = write(fd, "abc", 3);
    printf("ret = %d\n", ret);
}
