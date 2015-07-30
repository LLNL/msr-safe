obj-m += msr-safe.o
msr-safe-objs := msr_entry.o msr_whitelist.o msr-smp.o msr_batch.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
