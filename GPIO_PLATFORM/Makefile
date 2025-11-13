obj-m += gpio_led_driver.o

ARCH = arm
CROSS_COMPILE = aarch64-linux-gnu-

KDIR := $(HOME)/linux	
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

