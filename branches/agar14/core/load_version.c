/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Routines for reading and writing signature and version information
 * from archive data.
 */

#include <core/core.h>

#include <string.h>
#include <stdio.h>

int
AG_ReadVersion(AG_DataSource *ds, const char *name, const AG_Version *ver,
    AG_Version *rver)
{
	char nbuf[AG_VERSION_NAME_MAX];
	size_t nlen;
	Uint32 major, minor;

	nlen = strlen(name);

	if (AG_Read(ds, nbuf, sizeof(nbuf), 1) != 0 ||
	    strncmp(nbuf, name, nlen) != 0) {
		AG_SetError(_("%s: Bad signature in data source"), name);
		return (-1);
	}
	major = AG_ReadUint32(ds);
	minor = AG_ReadUint32(ds);

	if (rver != NULL) {
		rver->major = major;
		rver->minor = minor;
	}
	if (major != ver->major) {
		AG_SetError(_("%s: Major differs: v%d.%d != %d.%d"),
		    name, major, minor, ver->major, ver->minor);
		return (-1);
	}
	if (minor != ver->minor) {
		Verbose(_("Warning: %s: Minor differs: v%d.%d != %d.%d\n"),
		    name, major, minor, ver->major, ver->minor);
	}
	return (0);
}

void
AG_WriteVersion(AG_DataSource *ds, const char *name, const AG_Version *ver)
{
	char nbuf[AG_VERSION_NAME_MAX];

	memset(nbuf, '!', sizeof(nbuf));
	Strlcpy(nbuf, name, sizeof(nbuf));
	if (AG_Write(ds, nbuf, sizeof(nbuf), 1) != 0) { AG_FatalError(NULL); }
	AG_WriteUint32(ds, ver->major);
	AG_WriteUint32(ds, ver->minor);
}

int
AG_ReadObjectVersion(AG_DataSource *ds, void *p, AG_Version *pver)
{
	AG_ObjectClass *cls = OBJECT(p)->cls;
	return (AG_ReadVersion(ds, cls->name, &cls->ver, pver));
}

void
AG_WriteObjectVersion(AG_DataSource *ds, void *p)
{
	AG_ObjectClass *cls = OBJECT(p)->cls;
	AG_WriteVersion(ds, cls->name, &cls->ver);
}