#!/usr/bin/perl
#
# Extract manual links from mdoc source.
# Public domain
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
	if (/^\.nr nS 1/) { $ns = 1; }
	elsif (/^\.nr nS 0/ || /^\.\\" NOMANLINK/) { $ns = 0; }
	next unless $ns;
	if (/^\.Fn ([\w\-]+)\s+/ &&
	    $1.'.'.$section ne $man.'.'.$section) {
		print STDERR "$1.$section => $man.$section\n";
		print "MANLINKS+=$man.${section}:$1.${section}\n";
		print "CATLINKS+=$man.cat${section}:$1.cat${section}\n";
	}
}
