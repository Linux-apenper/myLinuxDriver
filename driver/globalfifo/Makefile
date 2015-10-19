
define X86_TEST
	:
	1 /
	2
endef
X86_KERN = /lib/modules/2.6.32-71-generic/build
ARM_KERN = /home/sky/linux-2.6.38 

ifndef X86_TEST
KERN := $(ARM_KERN)
else
KERN := $(X86_KERN)
endif


obj-m += globalfifo.o
# modules-y := hello.o calculate.o
# modules-objs := hello.o calculate.o

#prepare:
#	make -C $(KERN) M=$(PWD) prepare

all:
	make -C $(KERN) M=$(shell pwd) modules
	./copyShell
#	gcc -O2 -Wall -o pollmonitor pollmonitor.c

clean:
#	make -C $(KERN) M=$(shell pwd) clean
	rm -f *.o *~.depend *.ko *.order *.symvers *.mod.c
