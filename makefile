ifeq (${IOPBINDIR},)
IOPBINDIR = /usr/local/iop
endif
#ifeq (${IOPTMPDOCDIR},)
IOPTMPDOCDIR = doc/apidoc
#endif

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
	ant javadoc
	rm -rf ${IOPBINDIR}/doc;
	cp -rf ${IOPTMPDOCDIR} ${IOPBINDIR}/doc

mcs_doc:
	rm -rf ${HOME}/public_html/GraphicsActor2D/doc
	cp -rf ${IOPTMPDOCDIR} ${HOME}/public_html/GraphicsActor2D/doc
	chmod -R go+rx ${HOME}/public_html/GraphicsActor2D/doc


clean: javaclean
	cd src; make -f c_makefile clean;

install:
	ant install
