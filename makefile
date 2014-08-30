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
	ant jars

antlr4:
	ant antlr4

src-zip:
	ant zip

doc: 
	ant api-g2d
	ant api-GUI

clean: 
	ant clean
	cd src; make clean

install:
	ant install

ianstall:
	ant install
	chown -R iop:iop /usr/local/iop

webdoc:
	ant api-g2d
	cp -r doc/api-g2d/*  ~iop/public_html/GraphicsActor2D/doc/
