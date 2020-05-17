#!/usr/bin/env perl
#
# Public domain
#

my $found = 0;
open(CONFIG, "configure.in") || die "configure.in: $!";
foreach $_ (<CONFIG>) {
	chop;
	if (/^\s*RELEASE\(\s*\"(.+)\"\s*\)\s*$/i) {
		print $1;
		if (@ARGV == 0 || $ARGV[0] != '-n') {
			print "\n";
		}
		$found++;
	}
}
close(CONFIG);
die 'cannot get release' unless ($found);

