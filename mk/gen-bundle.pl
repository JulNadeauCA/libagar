#!/usr/bin/env perl
#
# Copyright (c) 2015-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#
# Generate an application or library bundle for the given target platform.
# Invoked by "make bundle" target of <build.prog.mk> or <build.lib.mk>.
#

my $type = $ARGV[0];
my $tgt = $ARGV[1];

my $prog = '';
my $InstallProg = '';

unless ($type && $tgt && $type =~ /^(prog|lib)$/) {
	print STDERR "Usage: $0 [prog|lib] [target-platform]\n";
	exit(1);
}

if ($type eq 'prog') {
	$prog = $ENV{'PROG'};
	$InstallProg = $ENV{'INSTALL_PROG'};
	unless ($prog && $InstallProg) {
		print STDERR "Missing PROG/INSTALL_PROG\n";
		exit(1);
	}
}

my $App = $prog.'.app';
my $Contents = '';
my $Plist = '';
my @dirs = ();

if ($tgt eq 'OSX') {	
	$Contents = $App.'/Contents';
	$Plist = $Contents.'/Info.plist';
	@dirs = ($App, $Contents, $Contents.'/Resources', $Contents.'/MacOS');
} elsif ($tgt eq 'iOS') {
	$Plist = $App.'/Info.plist';
	@dirs = ($App);
}

foreach my $dir (@dirs) {
	if (! -e $dir) {
		print "mkdir $dir\n";
		mkdir($dir) || die "$dir: $!";
	}
}

if ($tgt eq 'OSX' || $tgt eq 'iOS') {
	my $BundleVersion = 1;

	if (-e $Plist) {
		if (open(PLIST, $Plist)) {
			my $vers = 0;
			LINE: foreach $_ (<PLIST>) {
				if (/<key>\s*CFBundleVersion\s*<\/key>/) {
					$vers = 1;
				}
				if ($vers && /<string>\s*([\d]+)\.0\.0\s*<\/string>/) {
					print "Updating bundle version: $1+1\n";
					$BundleVersion = $1 + 1;
					last LINE;
				}
			}
			close(PLIST);
		}
	}

	open(PLIST, ">$Plist") || die "$Plist: $!";
	print PLIST << 'EOF';
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
EOF
	my $InfoSignature = $ENV{'PROG_SIGNATURE'};
	if ($prog =~ /^(\w{4})/) {
		$InfoSignature = $1;
	} else {
		$InfoSignature = 'xxxx';
	}

	my $ProgDisplayName = $ENV{'PROG_DISPLAY_NAME'};
	my $ProgIdentifier = $ENV{'PROG_IDENTIFIER'};

	print PLIST << "EOF";
  <key>CFBundleDevelopmentRegion</key>
  <string>English</string>
  <key>CFBundleDisplayName</key>
  <string>$ProgDisplayName</string>
  <key>CFBundleExecutable</key>
  <string>$prog</string>
  <key>CFBundleIdentifier</key>
  <string>$ProgIdentifier</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleVersion</key>
  <string>${BundleVersion}.0.0</string>
EOF

	if ($tgt eq 'OSX') {
		my $ProgSysVersion = $ENV{'PROG_OSX_VERSION'};
		my $PrincipalClass = $ENV{'PROG_PRINCIPAL_CLASS'};
		my $Copyright = $ENV{'PROG_COPYRIGHT'};
		my $ProgVersion = $ENV{'PROG_VERSION'};
		my $ProgCategory = $ENV{'PROG_CATEGORY'};

		print PLIST << "EOF";
  <key>CFBundleName</key>
  <string>$prog</string>
  <key>CFBundleSignature</key>
  <string>$InfoSignature</string>
  <key>CFBundleGetInfoString</key>
  <string>$prog $ProgVersion</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0.0</string>
  <key>LSApplicationCategoryType</key>
  <string>$ProgCategory</string>
  <key>LSMinimumSystemVersion</key>
  <string>$ProgSysVersion</string>
  <key>NSHumanReadableCopyright</key>
  <string>$Copyright</string>
  <key>NSPrincipalClass</key>
  <string>$PrincipalClass</string>
  <key>CFBundleIconFile</key>
  <string>${prog}.icns</string>
EOF
		print "$InstallProg $prog $Contents/MacOS\n";
		system("$InstallProg $prog $Contents/MacOS");
	} elsif ($tgt eq 'iOS') {
		my $RequiredCaps = $ENV{'PROG_REQUIRED_CAPABILITIES'};

		print PLIST "  <key>UIRequiredDeviceCapabilities</key>\n";
		print PLIST "    <array>\n";
		foreach my $cap (split(' ', $RequiredCaps)) {
			print PLIST "      <string>$cap</string>\n";
		}
		print PLIST "    </array>\n";
		print PLIST << "EOF";
  <key>LSRequiresIPhoneOS</key>
  <string>YES</string>
  <array>
  <key>CFBundleIconFiles</key>
  <array>
    <string>${prog}</string>
  </array>
EOF
		print "$InstallProg $prog $App\n";
		system("$InstallProg $prog $App");
	}

	if (my $ProgInfoExtra = $ENV{'PROG_INFO_EXTRA'}) {
		print PLIST "$ProgInfoExtra\n";
	}
	print PLIST "</dict>\n";
	print PLIST "</plist>\n";
	close(PLIST);
}


