/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Loader for agar .den archives, which are typically used to store
 * resources such as application icons and sounds.
 */

#include <core/core.h>

#include <stdio.h>
#include <string.h>

#include <core/load_den.h>

const char *agDenMagic = "AG_DEN";
const AG_Version agDenVer = { 0, 0 };

/* Read a den header and its mappings. */
static int
ReadDenHeader(AG_Den *den)
{
	Uint32 i;

	AG_CopyString(den->hint, den->buf, sizeof(den->hint));
	AG_CopyString(den->name, den->buf, sizeof(den->name));

	den->author = AG_ReadString(den->buf);
	den->copyright = AG_ReadString(den->buf);
	den->descr = AG_ReadString(den->buf);
	den->keywords = AG_ReadString(den->buf);

	den->nmembers = AG_ReadUint32(den->buf);
	den->members = Malloc(den->nmembers*sizeof(AG_DenMember));

	for (i = 0; i < den->nmembers; i++) {
		AG_DenMember *memb = &den->members[i];

		AG_CopyNulString(memb->name, den->buf, sizeof(memb->name));
		AG_CopyNulString(memb->lang, den->buf, sizeof(memb->lang));
		den->members[i].offs = (off_t)AG_ReadUint32(den->buf);
		den->members[i].size = (size_t)AG_ReadUint32(den->buf);
	}
	return (0);
}

/* Write a den header and skip the mapping table. */
void
AG_DenWriteHeader(AG_Den *den, int nmemb)
{
	Uint32 i;

	AG_WriteString(den->buf, den->hint);
	AG_WriteString(den->buf, den->name);

	AG_WriteString(den->buf, den->author);
	AG_WriteString(den->buf, den->copyright);
	AG_WriteString(den->buf, den->descr);
	AG_WriteString(den->buf, den->keywords);

	/* Initialize the mapping table. */
	den->members = Malloc(nmemb*sizeof(AG_DenMember));
	den->nmembers = (Uint32)nmemb;
	for (i = 0; i < den->nmembers; i++) {
		AG_DenMember *memb = &den->members[i];

		memset(memb->name, '\0', sizeof(memb->name));
		memset(memb->lang, '\0', sizeof(memb->lang));
	}

	AG_WriteUint32(den->buf, den->nmembers);
	
	/* Skip the mappings. */
	den->mapoffs = AG_Tell(den->buf);
	AG_Seek(den->buf, den->nmembers*AG_DEN_MAPPING_SIZE, AG_SEEK_CUR);
}

/* Write the den mappings. */
void
AG_DenWriteMappings(AG_Den *den)
{
	Uint32 i;

	AG_Seek(den->buf, den->mapoffs, AG_SEEK_SET);

	for (i = 0; i < den->nmembers; i++) {
		AG_DenMember *memb = &den->members[i];

		AG_WriteUint32(den->buf, (Uint32)sizeof(memb->name));

		if (AG_Write(den->buf, memb->name, sizeof(memb->name), 1) != 0)
			AG_FatalError(NULL);

		AG_WriteUint32(den->buf, (Uint32)sizeof(memb->lang));
		
		if (AG_Write(den->buf, memb->lang, sizeof(memb->lang), 1) != 0)
			AG_FatalError(NULL);
			
		AG_WriteUint32(den->buf, (Uint32)memb->offs);
		AG_WriteUint32(den->buf, (Uint32)memb->size);
	}
}

/* Open a den archive; load the header as well in read mode. */
AG_Den *
AG_DenOpen(const char *path, enum ag_den_open_mode mode)
{
	AG_Den *den;

	den = Malloc(sizeof(AG_Den));
	den->buf = AG_OpenFile(path, (mode == AG_DEN_READ) ? "rb" : "wb");
	if (den->buf == NULL) {
		Free(den);
		return (NULL);
	}
	switch (mode) {
	case AG_DEN_READ:
		if (AG_ReadVersion(den->buf, agDenMagic, &agDenVer, NULL)==-1 ||
		    ReadDenHeader(den) == -1) {
			goto fail;
		}
		break;
	case AG_DEN_WRITE:
		AG_WriteVersion(den->buf, agDenMagic, &agDenVer);
		break;
	}
	return (den);
fail:
	AG_CloseFile(den->buf);
	Free(den);
	return (NULL);
}

void
AG_DenClose(AG_Den *den)
{
	AG_CloseFile(den->buf);

	Free(den->author);
	Free(den->copyright);
	Free(den->descr);
	Free(den->keywords);
	Free(den->members);
	Free(den);
}

/* Import the contents of a file in a den archive. */
int
AG_DenImportFile(AG_Den *den, int ind, const char *name, const char *lang,
    const char *infile)
{
	char buf[8192];
	AG_DenMember *memb;
	FILE *f;
	size_t size, rrv;
	off_t offs;
	
	offs = AG_Tell(den->buf);
	size = 0;

	if ((f = fopen(infile, "rb")) == NULL) {
		AG_SetError("Unable to open %s", infile);
		return (-1);
	}
	for (;;) {
		rrv = fread(buf, 1, sizeof(buf), f);
		size += rrv;

		if (AG_Write(den->buf, buf, rrv, 1) != 0) {
			return (-1);
		}
	}
	fclose(f);

	memb = &den->members[ind];
	Strlcpy(memb->name, name, sizeof(memb->name));
	Strlcpy(memb->lang, lang, sizeof(memb->lang));
	memb->offs = offs;
	memb->size = size;
	return (0);
}
