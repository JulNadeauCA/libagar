#!/usr/bin/perl

use Cwd;
#use Errno qw(EEXIST);

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

	print DSTMAKEFILE "SRC=$SRC\n";
	print DSTMAKEFILE "BUILD=$BUILD\n";

	my @deps = ();

	foreach $_ (@lines) {
		my @srcs = ();
		my @objs = ();

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
				if ($type eq 'OBJS') {			# C
					push @deps, << 'EOF';
	@echo "${CC} ${CFLAGS} ${CPPFLAGS}" -c $<
	@${CC} ${CFLAGS} -I`pwd` -I${BUILD} ${CPPFLAGS} -c $<
EOF
				} elsif ($type =~ /CATMAN\d/) {		# Nroff
					push @deps, << 'EOF';
	@echo "${NROFF} ${NROFF_FLAGS} $< > $@"
	@${NROFF} ${NROFF_FLAGS} $< > $@ || exit 0
EOF
				}
			}
		}
		if (/^\s*(SRCS|MAN\d|XCF|XCF\d|TTF|MAP)\s*=\s*(.+)$/) {
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
			print DSTMAKEFILE $type . '=' . join(' ', @srcs), "\n";
		} else {
			print DSTMAKEFILE $_, "\n";
		}
	}
	if (@deps) {
		print DSTMAKEFILE "\n", join("\n", @deps), "\n";
		print DSTMAKEFILE 'include .depend'."\n";
	}

	close(DSTMAKEFILE);
	close(SRCMAKEFILE);

	# Prevent make from complaining.
	open(DSTDEPEND, ">$BUILD/$ndir/.depend") or
	    die "$BUILD/$ndir/.depend: $!";
	close(DSTDEPEND);
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
			mkdir("$BUILD/$ndir/$ent", 0755);
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

