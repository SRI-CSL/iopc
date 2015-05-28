ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif

.PHONY: all c clean install

all: c


c:
	cd src; make

clean: 
	cd src; make clean
	rm -rf build


