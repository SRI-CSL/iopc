ifeq (${IOPBINDIR},)
IOPBINDIR = ${HOME}/bin/IOP
endif

.PHONY: all c clean install

all: c


c:
	cd src; make

install: c
	cd src; make install

clean: 
	cd src; make clean
	rm -rf build


