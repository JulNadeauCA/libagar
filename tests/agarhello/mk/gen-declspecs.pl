#!/usr/bin/env perl
#
# Public domain.
# 
# Look for __{BEGIN,END}_DECLS blocks in C header files and insert the
# "DECLSPEC" keyword required on some platforms, where appropriate.
#

# Process and output declaration block.
sub OutputDecls
{
	my $inline = 0;
	my $comment = 0;
		
	foreach my $line (@_) {
		# Begin/resume inline block
		if ($inline) {
			print $line."\n";
			if ($line =~ /^}\s*/) {
				$inline = 0;
			}
			next;
		} elsif ($line =~ /^static\s+__inline__/) {
			$inline = 1;
			print $line."\n";
			next;
		}
	
		# Begin/resume long comment
		if ($comment) {
			print $line."\n";
			if ($line =~ /\*\/\s*$/) {
				$comment = 0;
			}
			next;
		} elsif ($line =~ /^\s*\/\*/) {
			print $line."\n";
			$comment = 1;
			next;
		}

		if ($line =~ /^\s*extern DECLSPEC/) {	# Already processed
			print $line."\n";
			next;
		}
		$line =~ s/\s+/ /g;			# Fix whitespace

		if ($line =~ /^\s*struct\s*\w+\s*;\s*$/) {	# Forward decl
			print $line."\n";
			next;
		} elsif ($line =~ /^\s*extern (.+)$/) {		# Extern var
			print 'extern DECLSPEC '."$1\n";
			next;
		} elsif ($line =~				# Function decl
		    /^\s*([\w\*]+)\s+
		     ([\w\*\[\]]+)
		     ([\w\*\s,\.\[\]\(\)]*)\s*;\s*$/x) {
			print 'extern DECLSPEC '."$1 $2$3;\n";
			next;
		}
		$line =~ s/\s+/ /g;
		print $line."\n";
	}
}

my @input = ();
my $decls = 0;
my @blk = ();

if (@ARGV == 0) {
	print STDERR "Usage: gen-declspecs.pl [file]\n";
	exit(1);
}

open(F, $ARGV[0]) || die "$ARGV[0]: $!";

#
# First pass: Process cpp long lines.
#
my $trunc = 0;
my $truncLine = '';
while (<F>) {
	chop;
	if ($trunc) {
		$truncLine .= ' '.$_;
		if (!/\\$/) {
			$truncLine =~ s/\s+/ /g;
			push @input, $truncLine;
			$truncLine = '';
			$trunc = 0;
		} else {
			chop($truncLine);
		}
	} else {
		if (/\\$/) {
			chop;
			$truncLine = $_;
			$truncLine =~ s/\s+/ /g;
			$trunc = 1;
		} else {
			push @input, $_;
		}
	}
}

#
# Second pass: Process function/variable declarations.
#
my $inline = 0;
my $comment = 0;
my $stmt = 0;
my $stmtLine = '';
foreach $_ (@input) {
	if (/^\s*__BEGIN_DECLS\s*$/) {
		# Begin declarations block
		if ($decls) {
			print STDERR "Nested __BEGIN_DECLS!\n";
		}
		@blk = ($_);
		$decls = 1;
		next;
	}
	unless ($decls) {
		# Not in declarations block
		print $_."\n";
		next;
	}
	if (/^\s*__END_DECLS\s*$/) {
		# End declarations block
		push @blk, $_;
		print "/* Begin generated block */\n";
		OutputDecls(@blk);
		print "/* Close generated block */\n";
		$decls = 0;
		next;
	}

	# Strip comments in block
	s/(\/\*.*\*\/)//g;
	s/(\/\/.*)//g;

	# Begin/resume inline block
	if ($inline) {
		push @blk, $_;
		if (/^}\s*/) {
			$inline = 0;
		}
		next;
	} elsif (/^static __inline__ /) {
		$inline = 1;
		push @blk, '', $_;
		next;
	}
	
	# Begin/resume comment block
	if ($comment) {
		if (/\s*\*\/\s*$/) {
			$comment = 0;
		}
		push @blk, $_;
		next;
	} elsif (/^\s*\/\*/) {
		$comment = 1;
		push @blk, $_;
		next;
	}

	# Ignore cpp directives.
	if (/^\s*#/) {
		push @blk, $_;
		next;
	}

	# Begin/resume C statement
	if ($stmt) {
		$stmtLine .= ' '.$_;
		if (/;\s*$/) {
			$stmt = 0;
			push @blk, $stmtLine;
			$stmtLine = '';
		}
		next;
	} else {
		unless (/;\s*$/) {
			$stmt = 1;
			$stmtLine = $_;
			next;
		}
	}
	push @blk, $_;
}

close(F);

