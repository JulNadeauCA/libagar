#!/usr/bin/env perl
#
# Public domain
#
# Remove files specified on input.
#

foreach my $file (<STDIN>) {
	chop($file);
	unlink($file) || print STDERR "Removing $file: $!\n";
}
