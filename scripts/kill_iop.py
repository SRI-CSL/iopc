#!/usr/bin/env python
#

import sys
import os
import subprocess
import signal


def getCandidates():
    pl = subprocess.Popen("ps aux |  awk '/ system /  {print $2}'", stdout=subprocess.PIPE, shell=True).communicate()[0]
    return pl.split()


# get the current processes that look plausibly like the iop system actor
list0 = getCandidates()

# get the current processes that look plausibly like the iop system actor
list1 = getCandidates()

iops = []

#get those that are not transient (i.e. do not belong to this commands subprocess tree)
for pid in list0:
    if pid in list1:	
        iops.insert(0, pid)

#send them the SIGUSR1 signal, they should do the rest, if they are iops.        
for pid in iops:
    os.kill(int(pid), signal.SIGUSR1)


