obj-m += vfm_grabber.o

EXTRA_CFLAGS += -I$(src)/include
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build

all:
	$(MAKE) -C $(KSRC) M=$(shell pwd)  modules

clean:
	$(MAKE) -C $(KSRC) M=$(shell pwd)  clean
