ifneq ($(KERNELRELEASE),)
	obj-m := topics.o
else
	KERNEL_DIR = /lib/modules/`uname -r`/build
	MODULEDIR := $(shell pwd)

.PHONY: modules
	default: modules

modules:
	make -C $(KERNEL_DIR) M=$(MODULEDIR) modules

test:test.cc
	g++ -o $@ $< -O2

clean distclean:
	rm -f *.o *.mod.c .*.*.cmd *.ko
	rm -rf .tmp_versions
endif

