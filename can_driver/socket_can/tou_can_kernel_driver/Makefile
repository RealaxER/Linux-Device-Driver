obj-m := main.o

kernel_ver := $(shell uname -r)

all:
	make -C /lib/modules/$(kernel_ver)/build M=$(shell pwd) modules

install:
	make -C /lib/modules/$(kernel_ver)/build M=$(shell pwd) modules_install

clean:
	make -C /lib/modules/$(kernel_ver)/build M=$(shell pwd) clean



# obj-m += main.o
# PWD := $(shell pwd)
# CROSS := /home/bhien/bbb/kernelbuildscripts/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
# KERNEL := /home/bhien/bbb/kernelbuildscripts/KERNEL/
# INC=$PWD
# all: 
# 	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(PWD) EXTRA_CFLAGS="-I$(INC)" modules
# clean:
# 	make -C $(KERNEL) M=$(PWD) clean
