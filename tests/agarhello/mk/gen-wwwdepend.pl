#!/usr/bin/env perl
#
# Public domain.
#
# Scan <build.www.mk> sources for m4 include() statements and output
# the required make dependencies.
#

sub Scan ($)
{
	my $file = shift;
	my @rv = ();

	if (!open(SRC, $file)) {
		return ();
	}
	foreach my $line (<SRC>) {
		if ($line !~ /include\s*\(([\w\-\.\s\/]+)\)/) { next; }
		my $incl = $1;
		$incl =~ s/__FILE/$file/g;
		$incl =~ s/__LANG/en/g;
		$incl =~ s/__BASE_DIR/$ENV{'BASEDIR'}/g;
		$incl =~ s/__TEMPLATE/$ENV{'TEMPLATE'}/g;
		$incl =~ s/__CSS_TEMPLATE/$ENV{'CSS_TEMPLATE'}/g;
		if ($incl eq $ENV{'BASEDIR'}.'/base.htm') {
			next;
		}
		push @rv, $incl;
		if (-e $incl) {
			push @rv, Scan($incl);
		}
	}
	close(SRC);

	return (@rv);
}

if (@ARGV < 1) {
	print STDERR "Usage: gen-wwwdepend.pl [files]\n";
	exit(1);
}
foreach my $file (@ARGV) {
	my $src;
	if ($file =~ /^([\w\-\.\s]+)\.(html|html\.var)$/) {
		my $src = $1.'.htm';
		my $tmplFile = $ENV{'BASEDIR'}.'/'.$ENV{'TEMPLATE'}.'.m4';
		print $file.': '. join(' ',
		    $src, Scan($src), $tmplFile, Scan($tmplFile)), "\n";
	} elsif ($file =~ /^([\w\-\.\s]+)\.(css)$/) {
		my $src = $1.'.css-in';
		my $tmplFile = $ENV{'BASEDIR'}.'/'.$ENV{'CSS_TEMPLATE'}.'.m4';
		print $file.': '. join(' ',
		    $src, Scan($src), $tmplFile, @tmplDeps), "\n";
	} else {
		print STDERR "$file: Unknown extension\n";
	}
}
