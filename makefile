ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif

.PHONY: all c java doc clean install

all: java c

c:
	cd src; make -f c_makefile

java:
	ant

javaclean:
	ant clean

zip:
	ant zip

doc: 
	ant api-g2d

clean: javaclean
	cd src; make -f c_makefile clean

install:
	ant install
