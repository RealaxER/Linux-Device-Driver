BINARY := invcase
BUILD := /lib/modules/$(shell uname -r)/build
obj-m := $(BINARY).o

all:
	make -C $(BUILD) M=$(PWD) modules
clean:
	make -C $(BUILD) M=$(PWD) clean
