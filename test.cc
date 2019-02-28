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


    int fd2 = ioctl(fd, TOPICS_GET_FD);
    printf("ioctl get fd = %d\n", fd2);

    int ret = write(fd2, "aaaaa", 5);
    printf("write = %d\n", ret);

    ret = ftruncate(fd2, 100);
    printf("truncate = %d\n", ret);


    close(fd);

    void *addr = mmap(0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
    if ( addr == MAP_FAILED ) {
        printf("fail mmap = %p\n", addr);
    }
    printf("mmap = %p\n", addr);

    while (1) {
        sleep(1);
    }
}
