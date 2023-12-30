ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := arducam.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif


# obj-m += arducam.o
# PWD := $(shell pwd)
# CROSS := /home/bhien/bbb/kernelbuildscripts/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
# KERNEL := /home/bhien/bbb/kernelbuildscripts/KERNEL/
# INC=$PWD
# all: 
# 	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(PWD) EXTRA_CFLAGS="-I$(INC)" modules
# clean:
# 	make -C $(KERNEL) M=$(PWD) clean

