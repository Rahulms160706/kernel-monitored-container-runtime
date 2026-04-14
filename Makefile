obj-m += monitor.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc engine.c -o engine
	gcc workloads/memhog.c -o rootfs-alpha/memhog
	gcc workloads/cpu_stress.c -o rootfs-alpha/cpu_stress

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f engine
	rm -f rootfs-alpha/memhog
	rm -f rootfs-alpha/cpu_stress
