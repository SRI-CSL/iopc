ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif

.PHONY: all c java doc clean install

all: java c


c:
	cd src; make

debug: c
	ant -DDEBUG=on

java:
	ant

javaclean:
	ant clean
	ant -f build-PLA.xml clean

src-zip:
	ant zip

doc: 
	ant api-g2d
	ant api-GUI
	ant -f build-PLA.xml api-pla

clean: javaclean
	cd src; make clean

install:
	ant install

ianstall:
	ant install
	chown -R iop:iop /usr/local/iop

install-PLA:
	@echo -e "OBSOLETE! Please run instead: (see README.txt)"
	@echo -e "  > ant -f build-PLA.xml install [-DGUI={old,new}] [-DPLdir=<PL dir>]"

run-PLA:
	@echo -e "OBSOLETE! Please run instead: (see README.txt)"
	@echo -e "  > ant -f build-PLA.xml run [-DPLdir=<PL dir>]"

webdoc:
	ant api-g2d
	cp -r doc/api-g2d/*  ~iop/public_html/GraphicsActor2D/doc/

