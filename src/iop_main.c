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

#include "cheaders.h"
#include "constants.h"
#include "types.h"
#include "actor.h"
#include "registry_lib.h"
#include "iop_lib.h"
#include "externs.h"
#include "dbugflags.h"
#include "options.h"

int main(int argc, char** argv){
#ifdef _LINUX
  parseOptions(argc, argv, short_options, long_options);
#elif defined(_MAC)
  parseOptions(argc, argv, short_options);
#endif

  self_debug_flag   = IOP_DEBUG || iop_debug_flag;
  self = argv[0];

  announce("iop_debug_flag            = %d\n", iop_debug_flag);
  announce("iop_no_windows_flag       = %d\n", iop_no_windows_flag);
  announce("iop_chatter_flag          = %d\n", iop_chatter_flag);
  announce("iop_hardwired_actors_flag = %d\n", iop_hardwired_actors_flag);
  announce("iop_remote_fd             = %d\n", iop_remote_fd);
  announce("iop_server_mode           = %d\n", iop_server_mode);
  announce("iop_gui_debug_port        = %s\n", iop_gui_debug_port);
  announce("iop_port                  = %s\n", iop_port);
  
  if(iop_server_mode){
    if(iop_port == NULL){
      fprintf(stderr, "Usage: iop -sp <portno>\n");
      exit(EXIT_FAILURE);
    } else {
      spawnServer(argc, argv);
    }
  } else {
    iop_init(argc, argv, optind, iop_remote_fd);
  }
  exit(EXIT_SUCCESS);
}
