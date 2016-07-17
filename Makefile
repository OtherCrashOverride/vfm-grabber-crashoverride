obj-m += vfm_grabber.o

EXTRA_CFLAGS += -I$(src)/include
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(M)  modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(M)  clean
