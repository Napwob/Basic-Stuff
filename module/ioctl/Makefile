obj-m += hello_world.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Wall -o ioctl ioctl.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	ls
	rm ioctl
