#global multiuser location (for make install)
BINDIR = /usr/local/iop
#local single user location (for make local)
LOCALDIR = ~/bin/IOP
#temporary bin dir (also appears in src/c_makefile)
TMPBINDIR = /tmp/iopbin

TMPDOCDIR = /tmp/iopdoc

all:
	cd src; make -f c_makefile

java:
	cd src; make -f c_makefile java

javaclean:
	cd src; make -f c_makefile javaclean

zip:
	cd src; make -f c_makefile clean 
	rm -f bin/*/*.class
	rm -f *~ *.zip *.bak
	cd ..; mv IOP.zip IOP.zip.bak; zip -r IOP IOP

doc:
	rm -rf ${TMPDOCDIR}
	cd src; make doc_www -f Makefile

clean:
	cd src; make -f c_makefile clean;
	rm -rf ${TMPBINDIR}

install:
	rm -rf ${BINDIR}/bin;
	mkdir ${BINDIR}/bin;
	cp -rf ${TMPBINDIR}/* ${BINDIR}/bin/
	rm -rf ${BINDIR}/doc;
	mv ${TMPDOCDIR} ${BINDIR}/doc

local:
	rm -rf ${LOCALDIR};
	mkdir ${LOCALDIR};
	cp -rf bin/* ${LOCALDIR}
