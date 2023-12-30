obj-m += adxl345.o
#mpu6050-objs += mpu6050.o mpu6050_reg.o
PWD := $(shell pwd)
CROSS := /home/asus/bbb/kernelbuildscripts/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KERNEL := /home/asus/bbb/kernelbuildscripts/KERNEL/
INC = $(PWD)
all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(PWD) EXTRA_CFLAGS="-I$(INC)" modules

clean:
	make -C $(KERNEL) M=$(PWD) clean