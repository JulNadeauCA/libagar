# Public domain

use strict;
use ExtUtils::MakeMaker qw(WriteMakefile prompt);
use Getopt::Long qw(GetOptions);

# Look for Agar using agar-config.
my $cflags = `agar-config --cflags`;
my $libs = `agar-config --libs`;
if ($? != 0) {
	die "Agar library is not installed; See http://libagar.org/";
}
chomp($cflags);
chomp($libs);

my %Makefile = (
	'AUTHOR'	=> 'Julien Nadeau <vedge@hypertriton.com>, ' .
	                   'Mat Sutcliffe <oktal@gmx.co.uk>',
	'ABSTRACT'	=> 'Perl interface to the Agar GUI toolkit',
	'NAME'		=> 'Agar',
	'VERSION_FROM'	=> 'Agar.pm',
	'PREREQ_PM'	=> { },
	'LIBS'		=> $libs,
	'DEFINE'	=> ' ',
	'INC'		=> $cflags,
	'CCFLAGS'	=> '-D_USE_AGAR_STD -D_USE_AGAR_QUEUE -D_USE_AGAR_TYPES '.
	                   '-Wall -Wno-unused -Werror',
	'OBJECT'	=> q/$(O_FILES)/,
);
WriteMakefile(%Makefile);
