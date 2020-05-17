/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/core.h>

#include <string.h>
#include <stdio.h>

/* Read Agar archive version information. */
int
AG_ReadVersion(AG_DataSource *ds, const char *name, const AG_Version *ver,
    AG_Version *rver)
{
	char nbuf[AG_VERSION_NAME_MAX];
	size_t nlen;
	Uint32 major, minor;

	nlen = strlen(name);

	if (AG_Read(ds, nbuf, sizeof(nbuf)) != 0 ||
	    strncmp(nbuf, name, nlen) != 0) {
		AG_SetErrorS("Bad magic");
		/* AG_SetError("Bad magic (\"%s\" != \"%s\")", name, nbuf); */
		return (-1);
	}
	major = AG_ReadUint32(ds);
	minor = AG_ReadUint32(ds);
	if (rver != NULL) {
		rver->major = major;
		rver->minor = minor;
	}
	if (major != ver->major) {
		AG_SetError(_("%s: Major differs: v%u.%u != %u.%u"),
		    name, (Uint)major, (Uint)minor, 
		    (Uint)ver->major, (Uint)ver->minor);
		return (-1);
	}
	if (minor != ver->minor) {
		Verbose(_("Warning: %s: Minor differs: v%u.%u != %u.%u\n"),
		    name, (Uint)major, (Uint)minor, 
		    (Uint)ver->major, (Uint)ver->minor);
	}
	return (0);
}

/* Write Agar archive version information. */
int
AG_WriteVersion(AG_DataSource *ds, const char *name, const AG_Version *ver)
{
	char nbuf[AG_VERSION_NAME_MAX];
	int i;
	
	for (i = 0; i < sizeof(nbuf); i += 4) {
		nbuf[i  ] = 'a';
		nbuf[i+1] = 'g';
		nbuf[i+2] = 'a';
		nbuf[i+3] = 'r';
	}
	Strlcpy(nbuf, name, sizeof(nbuf));
	
	if (AG_Write(ds, nbuf, sizeof(nbuf)) != 0) {
		return (-1);
	}
	AG_WriteUint32(ds, ver->major);
	AG_WriteUint32(ds, ver->minor);
	return (0);
}

int
AG_ReadObjectVersion(AG_DataSource *_Nonnull ds, void *_Nonnull p,
    AG_Version *_Nullable pver)
{
	AG_ObjectClass *C = OBJECT(p)->cls;

	return AG_ReadVersion(ds, C->name, &C->ver, pver);
}

void
AG_WriteObjectVersion(AG_DataSource *_Nonnull ds, void *_Nonnull p)
{
	AG_ObjectClass *C = OBJECT(p)->cls;

	AG_WriteVersion(ds, C->name, &C->ver);
}

#endif /* AG_SERIALIZATION */
