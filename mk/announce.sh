#!/bin/sh
#
#	$Csoft$

sendmail -t << EOF
From: Wilbern Cobb <vedge@csoft.org>
To: agar-announce@lists.csoft.net
Subject: new agar snapshot $1
X-Mailer: announce.sh
X-PGP-Key: 206C63E6

A new Agar source snapshot has been uploaded to beta.csoft.org.
As always, any feedback is greatly appreciated.

	http://beta.csoft.org/agar/agar-$1.tar.gz
	http://beta.csoft.org/agar/agar-$1.tar.gz.asc
	http://beta.csoft.org/agar/agar-$1.tar.gz.md5

EOF

