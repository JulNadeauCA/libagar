#!/usr/bin/perl

use Cwd;
use Errno qw(EEXIST);

$COOKIE = ".mkconcurrent_$$";
@DIRS = ();
$BUILD = '';

sub Debug
{
	print STDERR @_, "\n";
}

sub ConvertMakefile
{
	my ($dir, $ndir, $ent) = @_;

	open(SRCMAKEFILE, "$dir/$ent") or
	    die "src: $dir/$ent: $!";
	open(DSTMAKEFILE, ">$BUILD/$ndir/$ent") or
	    die "dest: $BUILD/$ndir/$ent: $!";
	my @lines = ();
	my $line = '';
	foreach $_ (<SRCMAKEFILE>) {
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

	my @deps = ();

	foreach $_ (@lines) {
		my @srcs = ();
		my @objs = ();

		s/%SRC%/$SRC/g;
		s/%BUILD%/$BUILD/g;
		s/%SEPARATE_BUILD%/concurrent/g;
		
		if (/^\s*(OBJS|CATMAN\d)\s*=\s*(.+)$/) {
			my $type = $1;
			foreach my $obj (split(/\s/, $2)) {
				next unless $obj;
				my $objsrc = $obj;

				if ($type eq 'OBJS') {			# C
					$objsrc =~ s/\.o$/\.c/g;
				} elsif ($type =~ /CATMAN\d/) {		# Nroff
					$objsrc =~ s/\.cat(\d)$/.\1/;
				}
				push @deps, "$obj: $SRC/$ndir/$objsrc";
			}
		}
		if (/^\s*(SRCS|MAN\d|XCF|TTF)\s*=\s*(.+)$/) {
			my $type = $1;
			my $srcs = $2;
			foreach my $src (split(/\s/, $srcs)) {
				next unless $src;
				push @srcs, $src;
			}
			my $i = 0;
			foreach my $src (@srcs) {
				$srcs[$i] = "$SRC/$ndir/$srcs[$i]";
				$i++;
			}
			print DSTMAKEFILE "${type}=" . join(' ', @srcs), "\n";
		} else {
			print DSTMAKEFILE $_, "\n";
		}
	}
	print DSTMAKEFILE "\n", join("\n", @deps), "\n";
	
	close(DSTMAKEFILE);
	close(SRCMAKEFILE);
}

sub Scan
{
	my $dir = shift;

	opendir(DIR, $dir) or die "$dir: $!";
	ENTRY: foreach my $ent (readdir(DIR)) {
		if ($ent eq '.' or $ent eq '..' or $ent eq 'CVS') {
			next ENTRY;
		}

		my $ndir = $dir;
		$ndir =~ s/^\.\///;

		if (-d "$dir/$ent" and ! -e "$dir/$ent/$COOKIE") {
			unless (mkdir("$BUILD/$ndir/$ent")) {
				if ($! != EEXIST) {
					die "$BUILD/$ndir/$ent: $!";
				}
			}
			Scan("$dir/$ent");
		} else {
			if ($ent eq 'Makefile') {
				ConvertMakefile($dir, $ndir, $ent);
			} elsif ($ent =~ /\.(mk|inc)$/ or $ent eq 'mkdep') {
				open(OLDMK, "$dir/$ent") or
				    die "$dir/$ent: $!";
				open(NEWMK, ">$BUILD/$ndir/$ent") or
				    die "$BUILD/$ndir/$ent: $!";
				print NEWMK <OLDMK>;
				close(NEWMK);
				close(OLDMK);
			}
		}
	}
	closedir(DIR);
}

$SRC = $ARGV[0];

unless ($SRC) {
	print STDERR "Usage: $0 [source-directory-path]\n";
	exit (0);
}

unless (-d $SRC) {
	print STDERR "$SRC: $!\n";
	exit(1);
}
if (-e 'configure.in') {
	print STDERR "Cannot perform concurrent build in source directory\n";
	exit(1);
}

$BUILD = getcwd();
chdir($SRC) or die "$SRC: $!";

open(COOKIE, ">$BUILD/$COOKIE") or die "$BUILD/COOKIE: $!";
Scan('.');
close(COOKIE);

END
{
	unlink("$BUILD/$COOKIE");
}

