#!/usr/bin/env perl
#
# Public domain
#
# manlinks.pl: Extract manual links from mdoc source.
#

my $man = $ARGV[0];
my $ns = 0;

unless ($man) {
	print STDERR "Usage: $0 [man]\n";
	exit 1;
}

$man =~ s/([\w\-]+)\.(\d)$/$1/;
$section = $2;

while (<STDIN>) {
	if (/^\.\\"\s*MANLINK\s*\(([\w\-]+)\)/) {
		print "MANLINKS+=$man.${section}:$1.${section}\n";
#		print "CATLINKS+=$man.cat${section}:$1.cat${section}\n";
	}
	if (/^\.nr nS 1/) { $ns = 1; }
	elsif (/^\.nr nS 0/ || /^\.\\" NOMANLINK/) { $ns = 0; }
	next unless $ns;
	if (/^\.Fn ([\w\-]+)\s+/ &&
	    $1.'.'.$section ne $man.'.'.$section) {
		print "MANLINKS+=$man.${section}:$1.${section}\n";
#		print "CATLINKS+=$man.cat${section}:$1.cat${section}\n";
	}
}
