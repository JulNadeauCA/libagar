#!/usr/bin/perl
#
# Copyright (c) 2003 CubeSoft Communications, Inc.
# <http://www.csoft.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

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
	print DSTMAKEFILE "\n";

	my @deps = ();

	foreach $_ (@lines) {
		my @srcs = ();
		my @objs = ();

		if (/^\s*(OBJS|CATMAN\d|PSMAN\d)\s*=\s*(.+)$/) {
			my $type = $1;
			foreach my $obj (split(/\s/, $2)) {
				next unless $obj;
				my $objsrc = $obj;

				if ($type eq 'OBJS') {
					# C source
					$objsrc =~ s/\.o$/\.c/g;
				} elsif ($type =~ /CATMAN\d/) {
					# Nroff->catman
					$objsrc =~ s/\.cat(\d)$/.\1/;
				} elsif ($type =~ /PSMAN\d/) {
					# Nroff->postscript
					$objsrc =~ s/\.ps(\d)$/.\1/;
				}
				push @deps, "$obj: $SRC/$ndir/$objsrc";
				if ($type eq 'OBJS') {
					# C source
					push @deps, << 'EOF';
	@echo "${CC} ${CFLAGS} ${CPPFLAGS}" -c $<
	@${CC} ${CFLAGS} -I`pwd` -I${BUILD} ${CPPFLAGS} -c $<
EOF
				} elsif ($type =~ /CATMAN\d/) {
					# Nroff->catman
					push @deps, << 'EOF';
	@echo "${NROFF} -Tascii -mandoc $< > $@"
	@${NROFF} -Tascii -mandoc $< > $@ || exit 0
EOF
				} elsif ($type =~ /PSMAN\d/) {
					# Nroff->postscript
					push @deps, << 'EOF';
	@echo "${NROFF} -Tps -mandoc $< > $@"
	@${NROFF} -Tps -mandoc $< > $@ || exit 0
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
	print DSTDEPEND "\n";
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

