/*	$Csoft: version.c,v 1.8 2004/02/02 01:52:34 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <config/have_getpwuid.h>
#include <config/have_getuid.h>

#include <compat/gethostname.h>
#include <engine/error/error.h>

#include <sys/types.h>
#include <SDL_types.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>
#include <engine/loader/string.h>
#include <engine/loader/version.h>

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

	if (netbuf_eread(sig, siglen, 1, buf) < 1 ||
	    strncmp(sig, ver->name, siglen) != 0) {
		error_set("%s: bad magic.", ver->name);
		return (-1);
	}
	minor = read_uint32(buf);
	major = read_uint32(buf);

	if (rver != NULL) {
		rver->minor = minor;
		rver->major = major;
	}

	if (major != ver->major) {
		error_set("%s: major differs: v%d.%d != %d.%d.",
		    ver->name, major, minor, ver->major, ver->minor);
		return (-1);
	}
	if (minor != ver->minor) {
		fprintf(stderr, "%s: minor differs: v%d.%d != %d.%d.\n",
		    ver->name, major, minor, ver->major, ver->minor);
	}

	copy_string(user, buf, sizeof(user));
	copy_string(host, buf, sizeof(host));
#if 0
	fprintf(stderr, "%s: v%d.%d (%s@%s)\n", ver->name, major, minor, user,
	    host);
#endif
	return (0);
}

void
version_write(struct netbuf *buf, const struct version *ver)
{
	struct passwd *pw;
	char host[64];

	netbuf_write(ver->name, strlen(ver->name), 1, buf);
	write_uint32(buf, ver->minor);
	write_uint32(buf, ver->major);

#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	if ((pw = getpwuid(getuid())) != NULL) {
		write_string(buf, pw->pw_name);
	} else {
		write_string(buf, "???");
	}
#else
	write_string(buf, "???");
#endif

	if (gethostname(host, sizeof(host)) == 0) {
		write_string(buf, host);
	} else {
		write_string(buf, "???");
	}
}

