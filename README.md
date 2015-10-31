
[![Build Status](https://travis-ci.org/SRI-CSL/iopc.svg?branch=master)](https://travis-ci.org/SRI-CSL/iopc)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/5280/badge.svg)](https://scan.coverity.com/projects/5280)


## iopc


The C infrastructure for the IOP system (plus the latest iop.jar).

## Prerequisites

Using jlambda requires Java, using iop with the GUI interface also Java.
Without the GUI front end iop has no prerequisites.

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
## Javadoc for iop.jar

The javadoc for iop.jar is [here](http://www.jlambda.com/~iop/g2d/doc/)

## Manuals

The jlambda manual can be found [here](https://github.com/SRI-CSL/iopc/blob/master/doc/jlambda_manual.pdf?raw=true)

The iop manual can be found [here](https://github.com/SRI-CSL/iopc/blob/master/doc/iop_manual.pdf?raw=true)

## Websites

The jlambda webpage lives over [here](http://jlambda.com/~iop/jlambda.html)

The iop webpage lives over [here](http://jlambda.com/~iop/)

The main client of the iop system is [SRI International's Pathway Logic](http://pl.csl.sri.com/)







