ifneq ($(KERNELRELEASE),)
	obj-m := topics.o
else
	KERNEL_DIR = /lib/modules/`uname -r`/build
	MODULEDIR := $(shell pwd)

all:modules test

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

insmod:topics.ko
	rmmod  $<
	insmod $< 

mknod:
	rm -rf /dev/topics
	mknod /dev/topics c 231 0
	chmod 666 /dev/topics

test:test.cc
	g++ -o $@ $< -O2

install:modules insmod mknod
	echo "installed"

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko
	rm -rf .tmp_versions
endif

