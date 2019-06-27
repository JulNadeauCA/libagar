#!/usr/bin/env perl
#
# Public domain.
# 
# Create shadow directories of symbolic links to include files in the
# source directory (i.e., for ./configure --include=link).
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
			unless (symlink($srcdir.'/'.$file, $outfile)) {
				print STDERR "$file to $outfile: $!\n";
			}
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

if (@ARGV < 2) {
	print STDERR "Usage: gen-includelinks.pl [source-dir] [target-dir]\n";
	exit(1);
}
$srcdir = $ARGV[0];
$outdir = $ARGV[1];

if (! -e $outdir) {
	my $now = localtime;
	mkdir($outdir, 0755) || die "$outdir: $!";
	open(STAMP, ">$outdir/.generated") || die "$outdir/.generated: $!";
	print STAMP $now,"\n";
	close(STAMP);
}
Scan('.', $outdir);
CleanEmptyDirs($outdir);
