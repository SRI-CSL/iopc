#global multiuser location (for make install)
BINDIR = /usr/local/iop
#local single user location (for make local)
LOCALDIR = ~/bin/IOP
#temporary bin dir (also appears in src/c_makefile)
TMPBINDIR = /tmp/iopbin
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
	cd src; make doc_local -f java_makefile; mv doc ..

clean:
	cd src; make -f c_makefile clean;
	rm -rf ${TMPBINDIR}

install:
	rm -rf ${BINDIR}/bin;
	mkdir ${BINDIR}/bin;
	cp -rf ${TMPBINDIR}/* ${BINDIR}/bin/
	rm -rf ${BINDIR}/doc;
	mkdir ${BINDIR}/doc;		
	cp -r  doc/* ${BINDIR}/doc/

local:
	rm -rf ${LOCALDIR};
	mkdir ${LOCALDIR};
	cp -rf bin/* ${LOCALDIR}
