#!/usr/bin/env perl
#
# Public domain
#
# Look for added or removed files in a directory hierarchy.
#

my $OUTFILE = '.cmpfiles.out';
my $cmp = 0;
my %Files = ();
my $showAdded = 1;
my $showRemoved = 1;
my $sign = 1;

sub Find ($)
{
	my $dir = shift;

	if (opendir(DIR, $dir)) {
		foreach my $file (readdir(DIR)) {
			if ($file =~ /^\./) {
				next;
			}
			if (-d $file) {
				Find($dir.'/'.$file);
			} else {
				$Files{$dir.'/'.$file} = 1;
			}
		}
		closedir(DIR);
	}
}

if (@ARGV) {
	if ($ARGV[0] eq 'added') { $showRemoved = 0; $sign = 0; }
	elsif ($ARGV[0] eq 'removed') { $showAdded = 0; $sign = 0; }
}

Find('.');

if (-e $OUTFILE) {
	open(OUT, $OUTFILE) || die "$OUT: $!";
	foreach my $file (<OUT>) {
		chop($file);
		if (exists($Files{$file})) {
			delete($Files{$file});
		} else {
			if ($showRemoved) {
				if ($sign) {
					print '- '.$file."\n";
				} else {
					print $file."\n";
				}
			}
		}
	}
	close(OUT);
	foreach my $f (keys %Files) {
		if ($showAdded) {
			if ($f ne './'.$OUTFILE) {
				if ($sign) {
					print '+ '.$f."\n";
				} else {
					print $f."\n";
				}
			}
		}
	}
} else {
	open(OUT, ">$OUTFILE") || die "$OUT: $!";
	print OUT join("\n", keys %Files), "\n";
	close(OUT);
}
