/*	$Csoft: version.c,v 1.25 2003/02/22 00:09:49 vedge Exp $	*/

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

#include "compat/gethostname.h"
#include "engine.h"

#include <pwd.h>
#include <fcntl.h>

#include <libfobj/fobj.h>

#include "version.h"

#ifdef DEBUG
int	version_debug = 0;
#define	engine_debug version_debug
#endif

/*
 * The version minor of a structure is incremented when the changes
 * do not affect reading of a previous version (ie. additions).
 *
 * The version major of a structure is incremented when the changes
 * introduce incompatibilites with a previous version (ie. removals),
 * this is usually avoided using padding whenever possible.
 */

int
version_read(int fd, const struct version *ver, struct version *rver)
{
	char sig[64];
	int siglen;
	Uint32 major, minor;
	char *user, *host;

	siglen = strlen(ver->name);

	if (read(fd, sig, siglen) != siglen ||
	    strncmp(sig, ver->name, siglen) != 0) {
		error_set("%s: bad magic", ver->name);
		return (-1);
	}
	minor = read_uint32(fd);
	major = read_uint32(fd);

	if (rver != NULL) {
		rver->minor = minor;
		rver->major = major;
	}

	if (major != ver->major) {
		error_set("%s: major differs: v%d.%d != %d.%d", ver->name,
		    major, minor, ver->major, ver->minor);
		warning("%s\n", error_get());
		return (-1);
	}
	if (minor != ver->minor) {
		warning("%s: minor differs: v%d.%d != %d.%d\n", ver->name,
		    major, minor, ver->major, ver->minor);
	}

	user = read_string(fd, NULL);
	host = read_string(fd, NULL);
	dprintf("%s: v%d.%d (%s@%s)\n", ver->name, major, minor, user, host);
	free(user);
	free(host);

	return (0);
}

void
version_write(int fd, const struct version *ver)
{
	struct passwd *pw;
	char host[64];

	Write(fd, ver->name, strlen(ver->name));
	write_uint32(fd, ver->minor);
	write_uint32(fd, ver->major);
	
	pw = getpwuid(getuid());
	write_string(fd, pw->pw_name);
	if (gethostname(host, sizeof(host)) != 0) {
		fatal("gethostname: %s\n", strerror(errno));
	}
	write_string(fd, host);
}

void
version_buf_write(struct fobj_buf *buf, const struct version *ver)
{
	struct passwd *pw;
	char host[64];

	buf_write(buf, ver->name, strlen(ver->name));
	buf_write_uint32(buf, ver->minor);
	buf_write_uint32(buf, ver->major);
	
	pw = getpwuid(getuid());
	buf_write_string(buf, pw->pw_name);
	if (gethostname(host, sizeof(host)) != 0) {
		fatal("gethostname: %s\n", strerror(errno));
	}
	buf_write_string(buf, host);
}

