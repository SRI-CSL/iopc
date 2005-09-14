/*
    The InterOperability Platform: IOP
    Copyright (C) 2004 Ian A. Mason
    School of Mathematics, Statistics, and Computer Science   
    University of New England, Armidale, NSW 2351, Australia
    iam@turing.une.edu.au           Phone:  +61 (0)2 6773 2327 
    http://mcs.une.edu.au/~iam/     Fax:    +61 (0)2 6773 3312 


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define DEBUG         0
#define VERBOSE       0


#define BUFFSZ            1024
#define REGISTRY_ACTOR    "system"
#define REGISTRY_SELECT   "select"
#define REGISTRY_STATUS   "status"
#define REGISTRY_START    "start"
#define REGISTRY_STOP     "stop"
#define REGISTRY_NAME     "name"
#define REGISTRY_ENROLL   "enroll"
#define REGISTRY_UNENROLL "unenroll"
#define FIFO_IN           "*FIFO_IN*"
#define FIFO_OUT          "*FIFO_OUT*"
#define IOP_BIN_DIR       "*IOPBINDIR*"
#define IOP_DENV_VAR      "$IOPBINDIR"
#define IOP_DBENV_VAR      "${IOPBINDIR}"
#define IOP_DPENV_VAR      "$(IOPBINDIR)"
#define IOP_ENV_VAR       "IOPBINDIR"
#define ERRLOG            "iop_errors"
#define INWINDOW          "gui"
#define OUTWINDOW         "outwindow"
#define ERRWINDOW         "errwindow"
#define UNKNOWNNAME       "name request failed"
#define REGREADY          "registry is ready"
#define REGISTRYSZ        64
#define SIZE              64
#define MINPORT           8001
#define MAXPORT           9000
#define HOME              "HOME"
#define JARPATH           "/iop.jar"
#define STARTUP           "startup.txt"

/* name of configuration file, must be keep in synch with GUI/Constants.java */
#define IOPRC             ".ioprc"

#define IN                0
#define OUT               1
#define ERR               2

#define MAXBACKLOG        5


#define INSTRUCTIONS \
"\
\n\tTo send a message to an actor first type the number of the actor.\
\n\tYou will then be prompted to type your command, this will then be\
\n\tsent directly to the actor. Otherwise type:\
\n\n\t\t\"q\" to exit.\
\n\t\t\"h\" to see these instructions.\
\n\t\t\"a\" to see the actor registry again.\
\n\n\tGoodluck!\n\n"

#define IOP_ENV_WARNING \
"\
\n\tThe environment variable: IOP_EXECUTABLES_DIR is not set! Giving up.\
\n\tTry setting it to the appropriate directory, then try again. In\
\n\ttcsh something like \"setenv IOP_EXECUTABLES_DIR /home/iam/SRI/IOP\"\
\n\tmay help. You will also need this directory to be in your PATH.\n\n"


#define IOP_USAGE \
"\
\nUsage: iop options:\
\n\t-d     --debug       Debug mode\
\n\t-a     --actors      Hardwired actors mode\
\n\t-s <n> --server  <n> Server mode on port: <n>\
\n\t-r <n> --remote  <n> Slave to remote applet on socket fd: <n>\
\n\t-g <n> --gui     <n> Enable remote debugging of GUI JVM on port: <n>\
\n\t-n     --nowin       Don't launch any windows\
\n\t-c     --chatter     Prove commandline when in no windows mode\n\n"

