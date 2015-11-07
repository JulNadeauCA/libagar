#!/usr/bin/env perl
#
# Public domain.
# 
# Scan C header files and generate preprocessed versions of them in the
# specified target directory.
#

my $outdir = '';

sub Scan ($$)
{
	my $dir = shift;
	my $outdir = shift;

	if (! -e $outdir) {
		mkdir($outdir, 0755) || die "$outdir: $!";
	}
	unless (opendir(CWD, $dir)) {
		print STDERR "$dir: $!; ignored\n";
		return;
	}
	foreach my $ent (readdir(CWD)) {
		my $file = $dir.'/'.$ent;
		my $outfile = $outdir.'/'.$ent;

		if ($ent =~ /^\./ || -l $outfile || -l $file) {
			next;
		}
		if (-d $ent) {
			if (-e $file.'/.generated') { next; }
			Scan($file, $outfile);
			next;
		}
		if ($ent =~ /\.h$/) {
			my @o = readpipe("perl mk/gen-declspecs.pl '$file'");
			if ($? != 0) {
				print STDERR "gen-declspecs.pl failed\n";
				exit(1);
			}
			open(OUT, ">$outfile") || die "$outfile: $!";
			print OUT @o;
			close(OUT);
		}
	}
	closedir(CWD);
}

sub CleanEmptyDirs ($)
{
	my $dir = shift;

	unless (opendir(CWD, $dir)) {
		print STDERR "$dir: $!; ignored\n";
		return;
	}
	foreach my $ent (readdir(CWD)) {
		my $subdir = $dir.'/'.$ent;

		if ($ent =~ /^\./ || -l $subdir || !-d $subdir) {
			next;
		}
		CleanEmptyDirs($subdir);
		rmdir($subdir);
	}
	closedir(CWD);
}

if (@ARGV < 1) {
	print STDERR "Usage: gen-includes.pl [directory]\n";
	exit(1);
}
$outdir = $ARGV[0];

if (! -e $outdir) {
	my $now = localtime;
	mkdir($outdir, 0755) || die "$outdir: $!";
	open(STAMP, ">$outdir/.generated") || die "$outdir/.generated: $!";
	print STAMP $now,"\n";
	close(STAMP);
}
Scan('.', $outdir);
CleanEmptyDirs($outdir);
