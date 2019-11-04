ifneq ($(KERNELRELEASE),)
	obj-m := my_xtime.o
else
	KERNELDIR ?= /lib/modules/`uname -r`/build/
	PWD := `pwd`
.PHONY: default insert

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
insert: 
	sudo insmod my_xtime.ko
remove:
	sudo rmmod my_xtime
endif

clean:
	rm -f *.ko *.o Module* *mod*
 
