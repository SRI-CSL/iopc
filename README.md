
[![Build Status](https://travis-ci.org/SRI-CSL/iopc.svg?branch=master)](https://travis-ci.org/SRI-CSL/iopc)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/5280/badge.svg)](https://scan.coverity.com/projects/5280)


## iopc


The C infrastructure for the IOP system (plus the latest iop.jar).

## Prerequisites

Using iop and jlambda requires Java and Maude.

## Building and installing 

```
export IOPBINDIR = <where you want to install iop>
make
make install
```

## Using JLambda

```
export PATH = ${IOPBINDIR):${PATH}

jlambda


```

## Using IOP 

```
export PATH = ${IOPBINDIR):${PATH}

iop

```



Todo list:

0.  use vagrant to improve the travis

1.  coverity

2.  documentation

3.  release







