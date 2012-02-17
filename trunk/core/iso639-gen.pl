#!/usr/bin/perl
#
# Public domain
#
# http://ftp.ics.uci.edu/pub/ietf/http/related/iso639.txt
#

my @ents = ();

foreach $_ (<STDIN>) {
	chop;
	if (/^(\w\w) (.+)$/) {
		push @ents, $_;
	}
}

print "enum ag_language {\n";
print "\tAG_LANG_NONE,\t/* Undefined */\n";
foreach $_ (@ents) {
	if (/^(\w\w) (.+)$/) {
		print "\tAG_LANG_".uc($1).",\t/* ".$2." */\n";
	}
}
print "};\n";

print "const char *agLanguageCodes[] = {\n";
my $c = 1;
print "\t\"??\", ";
foreach $_ (@ents) {
	if (/^(\w\w) (.+)$/) {
		print "\"".$1."\", ";
		if ($c++ > 10) {
			print "\n\t";
			$c = 0;
		}
	}
}
print "};\n";

print "const char *agLanguageNames[] = {\n";
print "\tN_(\"Undefined\"),\n";
foreach $_ (@ents) {
	if (/^(\w\w) (.+)$/) {
		print "\tN_(\"".$2."\"),\n";
	}
}
print "};\n";
