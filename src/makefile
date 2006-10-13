COMPILER = gcc

# location of temporary build products
BUILDDIR = ../build

OS       =  $(shell uname)

ARCH     =  $(shell uname -p)

CPU      =  Intel

ifeq (Darwin, $(findstring Darwin, ${OS}))
#we are a mac, find out what CPU:
 ifeq (powerpc, $(findstring powerpc, ${ARCH}))
  CPU = powerpc
 endif
 ifeq (i386, $(findstring i386, ${ARCH}))
  CPU = Intel
 endif
#Mac flags
SCRIPT = mac_iop
CFLAGS =  -Wall -D_MAC -D_${CPU}
# add to CFLAGS for debugging:
# -g 
LIBS = -lpthread -lm 
else
#Linux flags
SCRIPT = linux_iop
LIBS =  -lm 
CFLAGS =  -Wall -pthread -D_LINUX -D_${CPU} -pedantic
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
      iop_graphics2d_wrapper\
      iop_pvs_wrapper\
      iop_maude_wrapper\
      iop_remote_actor\
      iop_sal_actor\
      iop_sal_spawner\
      iop_sal_wrapper

SCRIPTS = iop jlambda  
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

${EXE}: %: %.c ${COBJS} ${HEADERS} ${MISC}
	${COMPILER} ${CFLAGS} ${LIBS} ${COBJS} $< -o ${BUILDDIR}/$@

%.o: %.c %.h ${HEADERS} ${MISC}
	${COMPILER} ${CFLAGS} $< -c 

scripts:
	mkdir -p ${BUILDDIR}
	cp ${SCRIPT}  ${BUILDDIR}/iop
	cp jlambda   ${BUILDDIR}/

clean_tmp:
	rm -f /tmp/iop_*

clean:
	rm -f ${EXE} ${COBJS} 

zip: clean
	rm -f *~ *.zip *.bak
	cd ..; mv src.zip src.zip.bak; zip -r src src
