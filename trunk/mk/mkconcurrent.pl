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

		if (/(.+)\\$/) {	# Expansion
			$line .= $1;
		} else {		# New line
			if ($line) {
				push @lines, $line;
				$line = '';
			}
			push @lines, $_;
		}
	}
	my @srcs = ();
	my @deps = ();
	my @objs = ();
	foreach $_ (@lines) {
		if (/^\s*OBJS\s*=\s*(.+)$/) {
			foreach my $obj (split(/\s/, $1)) {
				next unless $obj;
				my $objsrc = $obj;
				$objsrc =~ s/\.o$/\.c/g;
				push @deps, "$obj: $objsrc";
			}
		}
		if (/^\s*SRCS\s*=\s*(.+)$/) {
			foreach my $src (split(/\s/, $1)) {
				next unless $src;
				push @srcs, $src;
			}
			my $i = 0;
			foreach my $src (@srcs) {
				$srcs[$i] = "$SRC/$srcs[$i]";
				$i++;
			}
			print DSTMAKEFILE 'SRCS=' . join(' ', @srcs), "\n";
		} else {
			print DSTMAKEFILE $_, "\n";
		}
	}
	print DSTMAKEFILE "\n", join("\n", @deps);
	
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
			} elsif ($ent =~ /\.mk$/ or $ent eq 'mkdep') {
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

