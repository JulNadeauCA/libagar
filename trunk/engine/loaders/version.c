/*	$Csoft: version.c,v 1.14 2005/09/17 07:35:30 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>

#include  "version.h"

int
AG_ReadVersion(AG_Netbuf *buf, const AG_Version *ver,
    AG_Version *rver)
{
	char nbuf[AG_VERSION_NAME_MAX];
	size_t nlen;
	Uint32 major, minor;

	nlen = strlen(ver->name);

	if (AG_NetbufReadE(nbuf, sizeof(nbuf), 1, buf) < 1 ||
	    strncmp(nbuf, ver->name, nlen) != 0) {
		AG_SetError("%s: Bad magic", ver->name);
		return (-1);
	}
	major = AG_ReadUint32(buf);
	minor = AG_ReadUint32(buf);

	if (rver != NULL) {
		rver->major = major;
		rver->minor = minor;
	}
	if (major != ver->major) {
		AG_SetError("%s: Major differs: v%d.%d != %d.%d.",
		    ver->name, major, minor, ver->major, ver->minor);
		return (-1);
	}
	if (minor != ver->minor) {
		fprintf(stderr, "%s: Minor differs: v%d.%d != %d.%d.\n",
		    ver->name, major, minor, ver->major, ver->minor);
	}
	return (0);
}

void
AG_WriteVersion(AG_Netbuf *buf, const AG_Version *ver)
{
	char nbuf[AG_VERSION_NAME_MAX];

	memset(nbuf, '!', sizeof(nbuf));
	strlcpy(nbuf, ver->name, sizeof(nbuf));
	AG_NetbufWrite(nbuf, sizeof(nbuf), 1, buf);
	AG_WriteUint32(buf, ver->major);
	AG_WriteUint32(buf, ver->minor);
}

