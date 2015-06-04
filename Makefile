#Note:  this makefile is processed by scripts/Makefile.build in the 
# 	linux source directory.  
obj-m += msr_safe.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
