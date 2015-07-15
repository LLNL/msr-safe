obj-m += msr-safe.o
msr-safe-objs := msr_safe.o msr-whitelist.o msr-smp.o msr-batch.o
KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
