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
# -D_POSIX_SOURCE -D_BSD_SOURCE _POSIX_C_SOURCE >= 199309L 199506L
# -g 
endif


# Standard Warnings
CFLAGS	+=	-Wall 
CFLAGS	+=	-Werror

# Extended warnings
#CFLAGS  +=     -Wextra
#CFLAGS	+=	-Wshadow 
CFLAGS	+=	-Wpointer-arith 
CFLAGS	+=	-Wcast-align 
#CFLAGS	+=	-Wwrite-strings
CFLAGS	+=	-Waggregate-return 
CFLAGS	+=	-Wstrict-prototypes 
CFLAGS	+=	-Wmissing-prototypes
CFLAGS	+=	-Wmissing-declarations 
CFLAGS	+=	-Wredundant-decls 
CFLAGS	+=	-Wnested-externs
CFLAGS	+=	-Winline 
CFLAGS	+=	-Wbad-function-cast 
CFLAGS	+=	-fstrict-aliasing
CFLAGS	+=	-fno-common 
CFLAGS	+=	-Wno-packed 
CFLAGS	+=	-Wswitch-default
CFLAGS	+=	-Wformat=2 
CFLAGS	+=	-Wformat-security
#CFLAGS	+=	-Wmissing-format-attribute
CFLAGS	+=	-D_REENTRANT 
CFLAGS	+=	-D_THREAD_SAFE 
CFLAGS	+=	-pipe 
CFLAGS	+=	-Wunused 
CFLAGS	+=	-Winit-self
CFLAGS	+=	-Wno-long-long 
CFLAGS	+=	-Wmissing-include-dirs
CFLAGS	+=	-Wno-variadic-macros
CFLAGS	+=	-ansi 
CFLAGS	+=	-std=c99
CFLAGS	+=	-pedantic




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
	cp ${SCRIPTS}  ${BUILDDIR}/

clean_tmp:
	rm -f /tmp/iop_*

clean:
	rm -f ${EXE} ${COBJS} *.o

zip: clean
	rm -f *~ *.zip *.bak
	cd ..; mv src.zip src.zip.bak; zip -r src src
