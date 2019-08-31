# Public domain

use ExtUtils::MakeMaker qw(WriteMakefile prompt);
use Getopt::Long qw(GetOptions);
use strict;

require 5.6.0;

# Look for Agar using agar-config.
my $agar_version = `agar-config --version`;
if ($? != 0) {
	die 'Agar is not installed (agar-config not found). ' .
	    'Get it from: http://libagar.org/';
}
chomp($agar_version);
if ($agar_version =~ /^(\d{1,2})\.(\d{1,2})\.(\d{1,2})$/) {
#	print "Agar Version: $agar_version -> XS version";
	$agar_version = $1 . '.' . $2.$3;
#	print ' ' . $agar_version . "\n";
}
my $agar_cflags = `agar-config --cflags`;
my $agar_libs = `agar-config --libs`;
chomp($agar_cflags);
chomp($agar_libs);
$agar_libs =~ s/-pthread/-lpthread/g;

my $CAPI = defined $ExtUtils::MakeMaker::CAPI_support;
my %Makefile = (
	'AUTHOR'	=> 'Mat Sutcliffe <oktal@gmx.co.uk>, ' .
	                   'Julien Nadeau Carriere <vedge@csoft.net>',
	'ABSTRACT'	=> 'Perl interface to Agar GUI toolkit',
	'NAME'		=> 'Agar',
	'VERSION'	=> $agar_version,
	'PREREQ_PM'	=> { },
	'LICENSE'       => 'freebsd',		# bsd 2-clause
	'LIBS'		=> $agar_libs,
	'DEFINE'	=> ' ',
	'INC'		=> $agar_cflags.' -g',
	'CCFLAGS'	=> ''.
	                   '-Wall -Wno-unused -Werror',
	'OBJECT'	=> q/$(O_FILES)/,
        $CAPI ? ('CAPI' => 'TRUE') : (),
);
WriteMakefile(%Makefile);
