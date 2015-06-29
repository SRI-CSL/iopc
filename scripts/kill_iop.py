#!/usr/bin/env python
#

import sys
import os
import subprocess


def getCandidates():
    pl = subprocess.Popen("ps aux |  awk '/ system /  {print $2}'", stdout=subprocess.PIPE, shell=True).communicate()[0]
    return pl.split()

list0 = getCandidates()
list1 = getCandidates()

iops = []

for pid in list0:
    if pid in list1:	
        iops.insert(0, pid)
    
for pid in iops:
    #os.kill(pid, signal.SIGUSR1)
    print pid

