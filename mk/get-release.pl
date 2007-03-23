#!/usr/bin/perl
# Public domain

my $found = 0;
open(CONFIG, "configure.in") || die "configure.in: $!";
foreach $_ (<CONFIG>) {
	chop;
	if (/^HDEFINE\(RELEASE,\s*"\\"(.+)\\""\)$/) {
		print $1, "\n";
		$found++;
	}
}
close(CONFIG);
die 'cannot get release' unless ($found);

