#!/usr/bin/perl
use warnings;
use strict;

my $DMODE = 0755;
my $FMODE = 0777;

my $zipfile = <IOP*.zip>;
if(!$zipfile){
    die "Sorry but I can't find my zip file!\n";
}
my $welcome = "\n\n\tWelcome to the IOP installer"; 
	

my $iopbindir = $ENV{IOPBINDIR};

if(!(defined($iopbindir) && $iopbindir)){
print <<END_OF_INTRO1;
$welcome
	
To install IOP you will need to choose a directory to hold the
binaries. This choice is made by setting the environment variable

IOPBINDIR 

to your choice. Typical choices are:
	
/usr/local/iop
~/bin/iop

Though, to do the former you may need to run this script as
root.  This directory *must* also then be added to your path.  Try doing
this, then rerunning this install script.

END_OF_INTRO1

} else {

my $path = $ENV{PATH};
if(!defined $path){ $path = ""; }
my @path = split /:/, $path;
my $OK = 0;
foreach my $dir (@path){
    if($dir eq $dir){ $OK = 1; last; }
}

if(!$OK){
print <<END_OF_INTRO2;
$welcome

You have set the environment variable 

IOPBINDIR to ${iopbindir}

but I could not see this directory in your PATH.
Why not set this choice of directories in your PATH,
and then retry this script.

END_OF_INTRO2

} else {


my $tilde = getTilde();
${iopbindir} =~ s/~/$tilde/;

print <<END_OF_INTRO3;

$welcome

I am going to try installing iop into the ${iopbindir} directory.

END_OF_INTRO3
    
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

}

sub getTilde {
    my @my_pw_entry  =  getpwuid($<);
    return $my_pw_entry[7];
}


}
