obj-m += lan8720.o
PWD := $(shell pwd)
CROSS := /home/bhien/bbb/kernelbuildscripts/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KERNEL := /home/bhien/bbb/kernelbuildscripts/KERNEL/
INC=$PWD
all: 
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD) clean
