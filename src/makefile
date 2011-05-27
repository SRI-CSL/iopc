COMPILER = gcc

# location of temporary build products
BUILDDIR = ../build

OS       =  $(shell uname)

ARCH     =  $(shell uname -p)

CPU      =  Intel

ifeq (Darwin, $(findstring Darwin, ${OS}))
#we are a mac, find out what CPU:
# ifeq (powerpc, $(findstring powerpc, ${ARCH}))
#  CPU = powerpc
# endif
# ifeq (i386, $(findstring i386, ${ARCH}))
#  CPU = Intel
# endif
#Mac flags
SCRIPT = mac_iop
#CFLAGS =  -Wall -D_MAC -D_${CPU}
#we build universal
CFLAGS = -Wall -D_MAC -arch i386 -arch ppc  -g
# on tiger remove the arches
# add to CFLAGS for debugging:
# -g 
LIBS = -lpthread -lm  -arch i386 -arch ppc
# on tiger remove the arches
else
#Linux flags
SCRIPT = linux_iop
LIBS =  -lm -pthread 
CFLAGS =  -Wall -D_LINUX -pedantic
# add to CFLAGS for debugging:
# -g 
endif


EXE = talk2iop\
      iop_executor\
      iop_registry\
      iop_main\
      iop_filemanager\
      iop_socketfactory\
      iop_listener\
      iop_socket\
      iop_server\
      iop_daemon\
      iop_graphics2d_wrapper\
      iop_pvs_wrapper\
      iop_qtbrowser_wrapper\
      iop_netrequest\
      iop_maude_wrapper\
      iop_remote_actor\
      iop_sal_actor\
      iop_sal_spawner\
      iop_sal_wrapper

SCRIPTS = jlambda kill_iop
HEADERS = actor.h argv.h cheaders.h constants.h dbugflags.h ec.h externs.h iop_lib.h macrostr.h msg.h options.h registry_lib.h sal_lib.h socket_lib.h types.h version.h wrapper_lib.h
MISC    = makefile 

vpath % ${BUILDDIR}

COBJS = actor.o\
        registry_lib.o\
        iop_lib.o\
        socket_lib.o\
        msg.o argv.o wrapper_lib.o sal_lib.o\
				ec.o macrostr.o


all:  scripts ${EXE}

${EXE}: %: %.o ${COBJS} ${HEADERS} ${MISC}
	${COMPILER}  ${LIBS} ${COBJS} $< -o ${BUILDDIR}/$@

#${CFLAGS}

%.o: %.c %.h ${HEADERS} ${MISC}
	${COMPILER} ${CFLAGS} $< -c 

scripts:
	mkdir -p ${BUILDDIR}
	cp ${SCRIPT}  ${BUILDDIR}/iop
	cp ${SCRIPTS}  ${BUILDDIR}/

clean_tmp:
	rm -f /tmp/iop_*

clean:
	rm -f ${EXE} ${COBJS} *.o

zip: clean
	rm -f *~ *.zip *.bak
	cd ..; mv src.zip src.zip.bak; zip -r src src
