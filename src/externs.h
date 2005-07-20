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

pid_t registry_pid               = -1;
pid_t iop_pid                    = -1;
char* registry_fifo_in           = NULL;
char* registry_fifo_out          = NULL;
int   reg2InPort                 = -1;
int   in2RegPort                 = -1;
int   in2RegFd                   = -1;
char* iop_bin_dir                = NULL;

int   iop_debug_flag             =  0;
int   iop_no_windows_flag        =  0;
int   iop_hardwired_actors_flag  =  0;
int   iop_remote_fd              =  0;
int   iop_server_mode            =  0;
char *iop_port                   =  NULL;
char *iop_gui_debug_port         =  NULL;
int   iop_chatter_flag           =  0;

pid_t child                      = -1;


/* externs used in: announce, signal handlers etc */
int   self_debug_flag = 0;
char* self;

