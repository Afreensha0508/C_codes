KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

obj-m = GPIO_driver.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	gcc user_app.c -o user_app

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f user_app

