#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "topics.h"

int main(int argc, char *argv[]) {
    int fd = open("/dev/topics", O_RDWR);
    printf("fd = %d %s\n", fd, strerror(errno));

    void *addr = mmap(0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if ( addr == MAP_FAILED ) {
        printf("fail mmap = %p\n", addr);
    }
    printf("mmap = %p\n", addr);

    close(fd);

    memset(addr, 0, 1024);

    printf("set ok\n");

    while (1) {
        sleep(1);
    }
}
