#use the system wide default if unset
ifndef (${IOPBINDIR})
	IOPBINDIR = /usr/local/iop
endif
#use the system wide default if unset
ifndef (${IOPTMPBINDIR})
	IOPTMPBINDIR = /tmp/iopbin
endif
#use the system wide default if unset
ifndef (${IOPTMPDOCDIR})
	IOPTMPDOCDIR = /tmp/iopdoc
endif

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
	rm -rf ${IOPTMPDOCDIR}
	cd src; make doc_www -f Makefile
	rm -rf ${IOPBINDIR}/doc;
	cp -rf ${IOPTMPDOCDIR} ${IOPBINDIR}/doc

mcs_doc:
	rm -rf ${HOME}/public_html/GraphicsActor2D/doc
	cp -rf ${IOPTMPDOCDIR} ${HOME}/public_html/GraphicsActor2D/doc
	chmod -R go+rx ${HOME}/public_html/GraphicsActor2D/doc


clean:
	cd src; make -f c_makefile clean;
	rm -rf ${IOPTMPBINDIR}

install:
	rm -rf ${IOPBINDIR}/bin;
	mkdir ${IOPBINDIR}/bin;
	cp -rf ${IOPTMPBINDIR}/* ${IOPBINDIR}/bin/
