obj-m += csr-safe.o
csr-safe-objs := csr_entry.o csr_whitelist.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
