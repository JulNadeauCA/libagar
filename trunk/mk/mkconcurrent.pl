#!/usr/bin/perl
#
# $Csoft: mkconcurrent.pl,v 1.16 2003/08/13 03:57:04 vedge Exp $
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

	open(SRCMAKEFILE, "$dir/$ent") ||
	    die "src: $dir/$ent: $!";
	open(DSTMAKEFILE, ">$BUILD/$ndir/$ent") ||
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

	print DSTMAKEFILE '# $Csoft: mkconcurrent.pl,v 1.16 2003/08/13 03:57:04 vedge Exp $', "\n";
	print DSTMAKEFILE "SRC=$SRC\n";
	print DSTMAKEFILE "BUILD=$BUILD\n";
	print DSTMAKEFILE "\n";

	my @deps = ();
	my @objs = ();
	my %catman;
	my %psman;

	foreach $_ (@lines) {
		my @srcs = ();

		if (/^\s*(SRCS|MAN\d|MOS)\s*=\s*(.+)$/) {
			my $type = $1;

			foreach my $src (split(/\s/, $2)) {
				unless ($src) {
					next;
				}
				my $obj = $src;

				if ($type eq 'SRCS') {
					$obj =~ s/\.c$/\.o/;
					push @objs, $obj;
				} elsif ($type =~ /MAN(\d)/) {
					$obj =~ s/\.(\d)$//;
					$catman{$1} .= " $obj.cat$1";
					$psman{$1} .= " $obj.ps$1";
				} elsif ($type =~ /MOS/) {
					$src =~ s/\.mo$/\.po/g;
				}
				if ($type eq 'SRCS') {
					# C/C++/Asm/Lex/Yacc source -> object
					# XXX C++/Asm/Lex/Yacc
					push @deps, "$obj: $SRC/$ndir/$src";
				} elsif ($type =~ /MAN(\d)/) {
					# Nroff -> ASCII/PostScript
					push @deps,
					    "$obj.cat$1: $SRC/$ndir/$src";
					push @deps,
					    "$obj.ps$1: $SRC/$ndir/$src";
				} elsif ($type =~ /MOS/) {
					# Portable object -> machine object
					push @deps, "$obj: $SRC/$ndir/$src";
				}
			}
		}
		if (/^\s*(SRCS|MAN\d|XCF|TTF|POS)\s*=\s*(.+)$/) {
			my $type = $1;
			my $srclist = $2;

			foreach my $src (split(/\s/, $srclist)) {
				unless ($src) {
					next;
				}
				push @srcs, $src;
			}
			my $i = 0;
			foreach my $src (@srcs) {
				$srcs[$i] = "$SRC/$ndir/$srcs[$i]";
				$i++;
			}
			print DSTMAKEFILE $type . '=' . join(' ', @srcs), "\n";
		} else {
			# XXX horrible
			if (/^\s*include\s*(.+)$/) {
				print DSTMAKEFILE "OBJS=@objs\n";
				print DSTMAKEFILE "CATMAN1=$catman{1}\n";
				print DSTMAKEFILE "CATMAN2=$catman{2}\n";
				print DSTMAKEFILE "CATMAN3=$catman{3}\n";
				print DSTMAKEFILE "CATMAN4=$catman{4}\n";
				print DSTMAKEFILE "CATMAN5=$catman{5}\n";
				print DSTMAKEFILE "CATMAN6=$catman{6}\n";
				print DSTMAKEFILE "CATMAN7=$catman{7}\n";
				print DSTMAKEFILE "CATMAN8=$catman{8}\n";
				print DSTMAKEFILE "CATMAN9=$catman{9}\n";
				print DSTMAKEFILE "PSMAN1=$psman{1}\n";
				print DSTMAKEFILE "PSMAN2=$psman{2}\n";
				print DSTMAKEFILE "PSMAN3=$psman{3}\n";
				print DSTMAKEFILE "PSMAN4=$psman{4}\n";
				print DSTMAKEFILE "PSMAN5=$psman{5}\n";
				print DSTMAKEFILE "PSMAN6=$psman{6}\n";
				print DSTMAKEFILE "PSMAN7=$psman{7}\n";
				print DSTMAKEFILE "PSMAN8=$psman{8}\n";
				print DSTMAKEFILE "PSMAN9=$psman{9}\n";
			}
			print DSTMAKEFILE $_, "\n";
		}

	}

	if (@deps) {
		print DSTMAKEFILE 'CFLAGS+=-I${BUILD}', "\n";
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

	opendir(DIR, $dir) || die "$dir: $!";
	ENTRY: foreach my $ent (readdir(DIR)) {
		if ($ent eq '.' || $ent eq '..' || $ent eq 'CVS') {
			next ENTRY;
		}

		my $ndir = $dir;
		$ndir =~ s/^\.\///;

		if (-d "$dir/$ent" && ! -e "$dir/$ent/$COOKIE") {
			mkdir("$BUILD/$ndir/$ent", 0755);
			Scan("$dir/$ent");
		} else {
			if ($ent eq 'Makefile') {
				ConvertMakefile($dir, $ndir, $ent);
			} elsif ($ent =~ /\.(mk|inc)$/ || $ent eq 'mkdep') {
				open(OLDMK, "$dir/$ent") ||
				    die "$dir/$ent: $!";
				open(NEWMK, ">$BUILD/$ndir/$ent") ||
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
chdir($SRC) || die "$SRC: $!";

open(COOKIE, ">$BUILD/$COOKIE") || die "$BUILD/COOKIE: $!";
Scan('.');
close(COOKIE);

END
{
	unlink("$BUILD/$COOKIE");
}

