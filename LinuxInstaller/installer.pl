#!/usr/bin/perl
use warnings;
use strict;

my $DMODE = 0755;
my $FMODE = 0777;

my $zipfile = <IOP*.zip>;

my $iopbindir = $ENV{IOPBINDIR};

my $tilde = getTilde();

if(!$zipfile){
    die "Sorry but I can't find my zip file!\n";
}

if(defined $iopbindir){
print <<END_OF_INTROA;
Welcome to the IOP installer. 

You have set the environment variable 

IOPBINDIR to ${iopbindir}

so I will use that directory.

END_OF_INTROA

} else {

print <<END_OF_INTROB;
Welcome to the IOP installer. 
	
To install IOP you will need to choose a directory to hold the
binaries. This directory should then be added to your path. Typical
choices are:
	
/usr/local/iop
~/bin/iop
	
To do the former you may need to run this script as root.

END_OF_INTROB

while(!$iopbindir){
    print "Please enter the iop bin directory location: ";
    $iopbindir = <>;
    chomp($iopbindir);
}

${iopbindir} =~ s/~/$tilde/;

print "You chose ${iopbindir} as the iop bin directory location.\n";
    
}

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


sub getTilde {
    my @my_pw_entry  =  getpwuid($<);
    return $my_pw_entry[7];
}
