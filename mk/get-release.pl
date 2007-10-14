#!/usr/bin/perl
# Public domain

my $found = 0;
open(CONFIG, "configure.in") || die "configure.in: $!";
foreach $_ (<CONFIG>) {
	chop;
	if (/^\s*HDEFINE\s*\(RELEASE,\s*\"(.+)\"\s*\)\s*$/) {
		print $1, "\n";
		$found++;
	}
}
close(CONFIG);
die 'cannot get release' unless ($found);

