/*	$Csoft	    */

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/version.h>

/*
 * The version minor of a structure is incremented when the changes
 * do not affect reading of a previous version (ie. additions).
 *
 * The version major of a structure is incremented when the changes
 * introduce incompatibilites with a previous version (ie. removals),
 * this is usually avoided using padding whenever possible.
 */

int
version_read(int fd, const struct version *ver)
{
	char sig[64];
	int siglen;
	Uint32 vermin, vermaj;
	char *user, *host;

	siglen = strlen(ver->name);

	if (read(fd, sig, siglen) != siglen ||
	    strncmp(sig, ver->name, siglen) != 0) {
		fatal("%s: bad magic\n", ver->name);
		return (-1);
	}
	vermin = fobj_read_uint32(fd);
	vermaj = fobj_read_uint32(fd);
	user = fobj_read_string(fd);
	host = fobj_read_string(fd);

	if (vermaj != ver->vermaj) {
		fatal("%s: : v%d.%d != %d.%d major differs\n", ver->name,
		    vermaj, vermin, ver->vermaj, ver->vermin);
		return (-1);
	}
	if (vermin != ver->vermin) {
		warning("%s: v%d.%d != %d.%d\n", ver->name, vermaj, vermin,
		    ver->vermaj, ver->vermin);
	}

	dprintf("%s: v%d.%d (%s@%s)\n", ver->name, vermaj, vermin, user, host);
	free(user);

	return (0);
}

int
version_write(int fd, const struct version *ver)
{
	struct passwd *pw;
	char host[64];

	write(fd, ver->name, strlen(ver->name));
	fobj_write_uint32(fd, ver->vermin);
	fobj_write_uint32(fd, ver->vermaj);
	
	pw = getpwuid(getuid());
	fobj_write_string(fd, pw->pw_name);
	gethostname(host, sizeof(host));
	fobj_write_string(fd, host);

	return (0);
}

