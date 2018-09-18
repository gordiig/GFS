obj-m += gfs.o

CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)

all:
	sudo make -C $(KDIR) M=$(PWD) modules

clean:
	sudo make -C $(KDIR) M=$(PWD) clean
