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
	rm -rf ${BINDIR}/doc;
	cp -rf ${TMPDOCDIR} ${BINDIR}/doc

mcs_doc:
	rm -rf ${HOME}/public_html/GraphicsActor2D/doc
	cp -rf ${TMPDOCDIR} ${HOME}/public_html/GraphicsActor2D/doc
	chmod -R go+rx ${HOME}/public_html/GraphicsActor2D/doc


clean:
	cd src; make -f c_makefile clean;
	rm -rf ${TMPBINDIR}

install:
	rm -rf ${BINDIR}/bin;
	mkdir ${BINDIR}/bin;
	cp -rf ${TMPBINDIR}/* ${BINDIR}/bin/

local:
	rm -rf ${LOCALDIR};
	mkdir ${LOCALDIR};
	cp -rf bin/* ${LOCALDIR}
