#!/usr/bin/perl

my $man = $ARGV[0];
my $ns = 0;

$man =~ s/.+\/([\w\-\.]+)$/$1/;

while (<STDIN>) {
	if (/^\.nr nS 1/) { $ns = 1; }
	elsif (/^\.nr nS 0/ || /^\.\\" NOMANLINK/) { $ns = 0; }
	next unless $ns;
	if (/^\.Fn ([\w\-]+)\s+/) {
		print "MLINKS+=$man:$1.3\n";
	}
}
