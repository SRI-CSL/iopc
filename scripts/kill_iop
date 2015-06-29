#!/usr/bin/env python
#

import sys
import os
import subprocess
import signal


def getIOPCandidates():
    pl = subprocess.Popen("ps aux |  awk '/ system /  {print $2}'",
                          stdout=subprocess.PIPE, shell=True).communicate()[0]
    return pl.split()

def getIOPServerCandidates():
    pl = subprocess.Popen("ps aux |  awk '/ iop_server /  {print $2}'", 
                          stdout=subprocess.PIPE, shell=True).communicate()[0]
    return pl.split()

def filterCandidates(getCandidates):
    # get the current processes that look plausible
    list0 = getCandidates()
    # get the current processes that look plausible
    list1 = getCandidates()
    # start of conservatively
    iops = []
    #get those that are not transient (i.e. do not belong to this commands subprocess tree)
    for pid in list0:
        if pid in list1:	
            iops.insert(0, pid)
    return iops

iops = filterCandidates(getIOPCandidates)
#send them the SIGUSR1 signal, they should do the rest, if they are iops.        
for pid in iops:
    print "Killing iop system: ", pid
    os.kill(int(pid), signal.SIGUSR1)


iopServers = filterCandidates(getIOPServerCandidates)
#send them the SIGUSR1 signal, they should do the rest, if they are iops.        
for pid in iopServers:
    print "Killing iop server: ", pid
    os.kill(int(pid), signal.SIGUSR1)


