ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif

.PHONY: all c java doc clean install

all: java c

c:
	cd src; make

java:
	ant

javaclean:
	ant clean

src-zip:
	ant zip

doc: 
	ant api-g2d

clean: javaclean
	cd src; make clean

install:
	ant install

dist: all
	ant distributable
