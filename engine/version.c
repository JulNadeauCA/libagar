/*	$Csoft: version.c,v 1.30 2003/04/14 08:56:20 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <engine/compat/gethostname.h>

#include <engine/engine.h>
#include <engine/version.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

#ifdef DEBUG
int	version_debug = 0;
#define	engine_debug version_debug
#endif

int
version_read(struct netbuf *buf, const struct version *ver,
    struct version *rver)
{
	char user[VERSION_USER_MAX];
	char host[VERSION_HOST_MAX];
	char sig[64];
	int siglen;
	Uint32 major, minor;

	siglen = strlen(ver->name);

	if (netbuf_read(buf, sig, siglen) != siglen ||
	    strncmp(sig, ver->name, siglen) != 0) {
		error_set(_("%s: bad magic"), ver->name);
		return (-1);
	}
	minor = read_uint32(buf);
	major = read_uint32(buf);

	if (rver != NULL) {
		rver->minor = minor;
		rver->major = major;
	}

	if (major != ver->major) {
		error_set(_("%s: major differs: v%d.%d != %d.%d"), ver->name,
		    major, minor, ver->major, ver->minor);
		return (-1);
	}
	if (minor != ver->minor) {
		dprintf("%s: minor differs: v%d.%d != %d.%d\n", ver->name,
		    major, minor, ver->major, ver->minor);
	}

	copy_string(user, buf, sizeof(user));
	copy_string(host, buf, sizeof(host));
	dprintf("%s: v%d.%d (%s@%s)\n", ver->name, major, minor, user, host);
	return (0);
}

void
version_write(struct netbuf *buf, const struct version *ver)
{
	struct passwd *pw;
	char host[64];

	netbuf_write(buf, ver->name, strlen(ver->name));
	write_uint32(buf, ver->minor);
	write_uint32(buf, ver->major);
	
	if ((pw = getpwuid(getuid())) != NULL) {
		write_string(buf, pw->pw_name);
	} else {
		write_string(buf, "???");
	}

	if (gethostname(host, sizeof(host)) == 0) {
		write_string(buf, host);
	} else {
		write_string(buf, "???");
	}
}

