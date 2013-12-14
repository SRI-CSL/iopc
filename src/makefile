COMPILER = gcc

# location of temporary build products
BUILDDIR = ../build

OS       =  $(shell uname)

ARCH     =  $(shell uname -p)

CPU      =  Intel

ifeq (Darwin, $(findstring Darwin, ${OS}))
CFLAGS = -Wall -D_MAC -arch i386 -g
# add to CFLAGS for debugging:
# -g 
LIBS = -lpthread -lm  -arch i386 
else
LIBS =  -lm -pthread 
CFLAGS =  -Wall -pthread -D_LINUX -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE -pedantic -Wno-unused-but-set-variable
# add to CFLAGS for debugging:
# -g 
endif


# Standard Warnings
CWFLAGS	+=	-Wall 
CWFLAGS	+=	-Werror

# Extended warnings
CWFLAGS  +=     -Wextra
#CWFLAGS	+=	-Wshadow 
CWFLAGS	+=	-Wpointer-arith 
CWFLAGS	+=	-Wcast-align 
#CWFLAGS	+=	-Wwrite-strings
CWFLAGS	+=	-Waggregate-return 
CWFLAGS	+=	-Wstrict-prototypes 
CWFLAGS	+=	-Wmissing-prototypes
CWFLAGS	+=	-Wmissing-declarations 
CWFLAGS	+=	-Wredundant-decls 
CWFLAGS	+=	-Wnested-externs
CWFLAGS	+=	-Winline 
CWFLAGS	+=	-Wbad-function-cast 
CWFLAGS	+=	-fstrict-aliasing
CWFLAGS	+=	-fno-common 
CWFLAGS	+=	-Wno-packed 
CWFLAGS	+=	-Wswitch-default
#CWFLAGS	+=	-Wformat=2 
#CWFLAGS	+=	-Wformat-security
#CWFLAGS	+=	-Wmissing-format-attribute
CWFLAGS	+=	-D_REENTRANT 
CWFLAGS	+=	-D_THREAD_SAFE 
CWFLAGS	+=	-pipe 
CWFLAGS	+=	-Wunused 
CWFLAGS	+=	-Winit-self
CWFLAGS	+=	-Wno-long-long 
CWFLAGS	+=	-Wmissing-include-dirs
CWFLAGS	+=	-Wno-variadic-macros
CWFLAGS	+=	-ansi 
CWFLAGS	+=	-std=c99
CWFLAGS	+=	-pedantic

CFLAGS += ${CWFLAGS}

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

SCRIPTS = iop jlambda jl kill_iop
HEADERS = actor.h\
	  argv.h\
	  cheaders.h\
	  constants.h\
	  dbugflags.h\
	  ec.h\
	  externs.h\
	  iop_lib.h\
	  iop_utils.h\
	  macrostr.h\
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
        registry_lib.o\
        iop_lib.o\
        iop_utils.o\
        socket_lib.o\
        msg.o argv.o wrapper_lib.o sal_lib.o\
				ec.o macrostr.o


all:  scripts ${EXE}

${EXE}: %: %.o ${COBJS} ${HEADERS} ${MISC}
	${COMPILER}  ${LIBS} ${COBJS} $< -o ${BUILDDIR}/$@

%.o: %.c %.h ${HEADERS} ${MISC}
	${COMPILER} ${CFLAGS} $< -c 

scripts:
	mkdir -p ${BUILDDIR}
	cp ${SCRIPTS}  ${BUILDDIR}/

clean_tmp:
	rm -f /tmp/iop_*

clean:
	rm -f ${EXE} ${COBJS} *.o

zip: clean
	rm -f *~ *.zip *.bak
	cd ..; mv src.zip src.zip.bak; zip -r src src
