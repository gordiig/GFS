ifneq ($(KERNELRELEASE),)
	objs-m := main.o

else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build
	PWD = $(shell pwd)

default:
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules
	sudo make clean

clean:
	rm *.o
	rm *.mod.c
	rm *.symvers
	rm *.order

endif
