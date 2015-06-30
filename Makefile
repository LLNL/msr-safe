#Note:  this makefile is processed by scripts/Makefile.build in the 
# 	linux source directory.  
obj-m += msr-safe.o
msr-safe-objs := msr_safe.o msr-smp.o msr-whitelist.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
