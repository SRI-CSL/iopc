#!/usr/bin/perl
use warnings;
use strict;

my $DMODE = 0755;
my $FMODE = 0777;

my $zipfile = <IOP*.zip>;

my $iopbindir = undef;

if(!$zipfile){
    die "Sorry but I can't find my zip file!\n";
}

print <<END_OF_INTRO;
Welcome to the IOP installer. 

To install IOP you will need to choose a directory to hold the
binaries. This directory should then be added to your path. Typical
choices are:

    /usr/local/iop
    ~/bin/iop

To do the former you may need to run this script as root.

END_OF_INTRO

while(!$iopbindir){
    print "Please enter the iop bin directory location: ";
    $iopbindir = <>;
    chomp($iopbindir);
}

print "You chose ${iopbindir} as the iop bin directory location.\n";

if(-e $iopbindir){
    print "This location appears to already exist.\n";
    if(!(-d $iopbindir)){
	die "This choice conflicts with an already exiting file!\n";
    }
} else {
    mkdir($iopbindir, $DMODE) or
	die "I couldn't create the desired directory ${iopbindir}: $!\n";
}

print `unzip -d $iopbindir $zipfile`;
