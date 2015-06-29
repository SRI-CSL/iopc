# location of temporary build products
BUILDDIR = ../build

OS   =  $(shell uname)
ARCH =  $(shell uname -p)


ifeq (Darwin, $(findstring Darwin, ${OS}))

######  DARWIN

CFLAGS = -Wall -D_MAC -arch i386 -g
LOLA = lola-2-macintel
# add to CFLAGS for debugging:
# -g 
LIBS = -lpthread -lm  -arch i386 

else

######## LINUX

LIBS =  -lm -pthread 
CFLAGS = -pthread -D_LINUX -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE -pedantic
TARGET = LINUX
# add to CFLAGS for debugging:
# -g

#check the arch for 32 or 64 bit
ifeq (i386, $(findstring i386, ${ARCH}))
LOLA = lola-2-linux32
else
LOLA = lola-2-linux64
endif

endif


EXES = talk2iop\
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
      iop_java_wrapper\
      iop_pvs_wrapper\
      iop_qtbrowser_wrapper\
      iop_netrequest\
      iop_maude_wrapper\
      iop_remote_actor\
      iop_sal_actor\
      iop_sal_spawner\
      iop_sal_wrapper

SCRIPTS = iop jlambda jl kill_iop

HEADERS = actor.h\
	  argv.h\
	  authenticate.h\
	  cheaders.h\
	  constants.h\
	  dbugflags.h\
	  ec.h\
	  externs.h\
	  iop_lib.h\
	  iop_utils.h\
	  msg.h\
	  options.h\
	  registry_lib.h\
	  sal_lib.h\
	  socket_lib.h\
	  types.h\
	  version.h\
	  wrapper_lib.h
MISC    = makefile 

vpath % ${BUILDDIR}

COBJS = actor.o\
        authenticate.o\
        registry_lib.o\
        iop_lib.o\
        iop_utils.o\
        socket_lib.o\
        msg.o argv.o\
	wrapper_lib.o\
	sal_lib.o\
	ec.o

JARS = iop.jar

all:  scripts ${EXES}

${EXES}: %: %.o ${COBJS} ${HEADERS} ${MISC}
	${CC}  ${LIBS} ${COBJS} $< -o ${BUILDDIR}/$@

%.o: %.c %.h ${HEADERS} ${MISC}
	${CC} ${CFLAGS} $< -c 

scripts:
	mkdir -p ${BUILDDIR}
	for script in ${SCRIPTS} ; do cp ../scripts/$${script} ${BUILDDIR}; done

install: all
	echo "Installing for ${TARGET} in ${IOPBINDIR}"
	mkdir -pv ${IOPBINDIR}
	for exe in ${EXES} ; do cp ../build/$${exe} ${IOPBINDIR}; done
	for script in ${SCRIPTS} ; do cp ../build/$${script} ${IOPBINDIR}; done
	for jar in ${JARS}  ; do cp ../lib/$${jar} ${IOPBINDIR}; done
	cp ../3rdParty/${LOLA}  ${IOPBINDIR}/lola-2

clean:
	rm -f ${EXES} ${COBJS} *.o

