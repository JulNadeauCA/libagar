#!/usr/bin/perl

if (@ARGV < 4) {
	print STDERR "Usage: $0 [proj] [os] [arch] [flavor]\n";
	exit(1);
}
my ($SRCDIR, $PROJNAME, $OS, $ARCH, $FLAVOR) = @ARGV;
my $VERSION = `(cd ${SRCDIR} && perl mk/get-version.pl)`;
my $RELEASE = `(cd ${SRCDIR} && perl mk/get-release.pl)`;
chop($VERSION);
chop($RELEASE);

while (<STDIN>) {
	s/%PROJNAME%/$PROJNAME/g;
	s/%VERSION%/$VERSION/g;
	s/%RELEASE%/$RELEASE/g;
	s/%OS%/$OS/g;
	s/%ARCH%/$ARCH/g;
	s/%FLAVOR%/$FLAVOR/g;
	print $_;
}
