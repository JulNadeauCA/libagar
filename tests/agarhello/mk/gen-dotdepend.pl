#!/usr/bin/env perl
# Public domain.
# Scan Makefiles for "include .depend" and generate empty ".depend" files,
# such that make can be run prior to an initial "make depend".
#

my %V = ();

sub MakefileIncludesDepend ($$)
{
	my $path = shift;
	my $cwd = shift;

	if (!open(MF, $path)) {
		return (0);
	}
	my @lines = ();
	foreach $_ (<MF>) {
		chop;

		if (/^(.+)\\$/) {			# Expansion
			$line .= $1;
		} else {				# New line
			if ($line) {
				push @lines, $line . $_;
				$line = '';
			} else {
				push @lines, $_;
			}
		}
	}
	foreach $_ (@lines) {
		if (/^\s*#/) { next; }
		if (/^\t/) { next; }
		s/\$\{(\w+)\}/$V{$1}/g;
		if (/^\s*(\w+)\s*=\s*"(.+)"$/ ||
		    /^\s*(\w+)\s*=\s*(.+)$/) {
			$V{$1} = $2;
		} elsif (/^\s*(\w+)\s*\+=\s*"(.+)"$/ ||
		         /^\s*(\w+)\s*\+=\s*(.+)$/) {
			if (exists($V{$1}) && $V{$1} ne '') {
				$V{$1} .= ' '.$2;
			} else {
				$V{$1} = $2;
			}
		}
		if (/^\s*include\s+(.+)$/) {
			if ($1 eq '.depend' ||
			    MakefileIncludesDepend($cwd.'/'.$1, $cwd)) {
				return (1);
			}
		}
	}
	close(MF);
	return (0);
}

sub Scan ($)
{
	my $dir = shift;

	unless (opendir(CWD, $dir)) {
		print STDERR "$dir: opendir: $!; ignoring\n";
		return;
	}
	%V = ();
	if (-e $dir.'/Makefile' &&
	    MakefileIncludesDepend("$dir/Makefile", $dir)) {
		if (open(OUT, ">$dir/.depend")) {
			close(OUT);
		} else {
			print STDERR "$dir/.depend: $!; ignoring\n";
		}
	}
	foreach my $ent (readdir(CWD)) {
		my $file = $dir.'/'.$ent;

		if ($ent =~ /^\./) {
			next;
		}
		if (-d $file) {
			Scan($file);
			next;
		}
	}
	closedir(CWD);
}
if (@ARGV < 1) {
	print STDERR "Usage: gen-dotdepend.pl [directory]\n";
	exit(1);
}
Scan($ARGV[0]);
