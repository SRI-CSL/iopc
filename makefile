ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif
ifeq (${IOPTMPBINDIR},)
IOPTMPBINDIR = /tmp/iopbin
endif
ifeq (${IOPTMPDOCDIR},)
IOPTMPDOCDIR = /tmp/iopdoc
endif

all: java
	cd src; make -f c_makefile

java:
	ant
#	cd src; make -f c_makefile java

javaclean:
	ant clean
#	cd src; make -f c_makefile javaclean

zip:
	cd src; make -f c_makefile clean 
#	rm -f bin/*/*.class
	rm -f *~ *.zip *.bak
	cd ..; mv IOP.zip IOP.zip.bak; zip -r IOP IOP

doc:
	rm -rf ${IOPTMPDOCDIR}
	cd src; make doc_www -f Makefile
	rm -rf ${IOPBINDIR}/doc;
	cp -rf ${IOPTMPDOCDIR} ${IOPBINDIR}/doc

mcs_doc:
	rm -rf ${HOME}/public_html/GraphicsActor2D/doc
	cp -rf ${IOPTMPDOCDIR} ${HOME}/public_html/GraphicsActor2D/doc
	chmod -R go+rx ${HOME}/public_html/GraphicsActor2D/doc


clean: javaclean
	cd src; make -f c_makefile clean;
	rm -rf ${IOPTMPBINDIR}

install:
	mkdir -p ${IOPBINDIR}
	rm -rf ${IOPBINDIR}; #/bin;
	mkdir ${IOPBINDIR}; #/bin;
	cp -rf ${IOPTMPBINDIR}/* ${IOPBINDIR} #/bin/
	cp build/lib/iop.jar ${IOPBINDIR}
