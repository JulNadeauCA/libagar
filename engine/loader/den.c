/*	$Csoft: den.c,v 1.1 2003/06/21 06:50:20 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/strlcpy.h>

#include <engine/error/error.h>

#include <sys/types.h>
#include <SDL_types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/version.h>
#include <engine/loader/integral.h>
#include <engine/loader/string.h>
#include <engine/loader/den.h>

const struct version den_ver = {
	"agar den",
	0, 0
};

/* Read a den header and its mappings. */
static int
den_read_header(struct den *den)
{
	Uint32 i;

	copy_string(den->hint, den->buf, sizeof(den->hint));
	copy_string(den->name, den->buf, sizeof(den->name));

	den->author = read_string(den->buf);
	den->copyright = read_string(den->buf);
	den->descr = read_string(den->buf);
	den->keywords = read_string(den->buf);

	den->nmembers = read_uint32(den->buf);
	den->members = Malloc(den->nmembers * sizeof(struct den_member));

	for (i = 0; i < den->nmembers; i++) {
		struct den_member *memb = &den->members[i];

		copy_string(memb->name, den->buf, sizeof(memb->name));
		copy_string(memb->lang, den->buf, sizeof(memb->lang));
		den->members[i].offs = (off_t)read_uint32(den->buf);
		den->members[i].size = (size_t)read_uint32(den->buf);
	}
	return (0);
}

/* Write a den header and skip the mapping table. */
void
den_write_header(struct den *den, int nmemb)
{
	Uint32 i;

	write_string(den->buf, den->hint);
	write_string(den->buf, den->name);

	write_string(den->buf, den->author);
	write_string(den->buf, den->copyright);
	write_string(den->buf, den->descr);
	write_string(den->buf, den->keywords);

	/* Initialize the mapping table. */
	den->members = Malloc(nmemb * sizeof(struct den_member));
	den->nmembers = (Uint32)nmemb;
	for (i = 0; i < den->nmembers; i++) {
		struct den_member *memb = &den->members[i];

		memset(memb->name, 'B', sizeof(memb->name));
		memset(memb->lang, 'o', sizeof(memb->lang));
	}

	write_uint32(den->buf, den->nmembers);
	
	/* Skip the mappings. */
	den->mapoffs = netbuf_tell(den->buf);
	netbuf_seek(den->buf, den->nmembers * DEN_MAPPING_SIZE, SEEK_CUR);
}

/* Write the den mappings. */
void
den_write_mappings(struct den *den)
{
	Uint32 i;

	netbuf_seek(den->buf, den->mapoffs, SEEK_SET);

	for (i = 0; i < den->nmembers; i++) {
		struct den_member *memb = &den->members[i];

		write_uint32(den->buf, (Uint32)sizeof(memb->name));
		netbuf_write(memb->name, sizeof(memb->name), 1, den->buf);
		write_uint32(den->buf, (Uint32)sizeof(memb->lang));
		netbuf_write(memb->lang, sizeof(memb->lang), 1, den->buf);

		write_uint32(den->buf, (Uint32)memb->offs);
		write_uint32(den->buf, (Uint32)memb->size);
	}
}

/* Open a den archive; load the header as well in read mode. */
struct den *
den_open(const char *path, enum den_open_mode mode)
{
	struct den *den;

	den = Malloc(sizeof(struct den));
	den->buf = netbuf_open(path, (mode == DEN_READ) ? "rb" : "wb",
	    NETBUF_BIG_ENDIAN);
	if (den->buf == NULL) {
		free(den);
		return (NULL);
	}

	switch (mode) {
	case DEN_READ:
		if (version_read(den->buf, &den_ver, NULL) == -1 ||
		    den_read_header(den) == -1) {
			goto fail;
		}
		break;
	case DEN_WRITE:
		version_write(den->buf, &den_ver);
		break;
	}
	return (den);
fail:
	netbuf_close(den->buf);
	free(den);
	return (NULL);
}

void
den_close(struct den *den)
{
	netbuf_close(den->buf);

	free(den->author);
	free(den->copyright);
	free(den->descr);
	free(den->keywords);
	free(den->members);
	free(den);
}

/* Import the contents of a file in a den archive. */
int
den_import_file(struct den *den, int ind, const char *name, const char *lang,
    const char *infile)
{
	char buf[8192];
	struct den_member *memb;
	FILE *f;
	size_t size, rrv;
	off_t offs;
	
	offs = netbuf_tell(den->buf);
	size = 0;

	if ((f = fopen(infile, "rb")) == NULL) {
		error_set("%s: %s", infile, strerror(errno));
		return (-1);
	}
	for (;;) {
		rrv = fread(buf, 1, sizeof(buf), f);
		size += rrv;

		netbuf_write(buf, rrv, 1, den->buf);
		if (rrv < sizeof(buf)) {
			if (feof(f)) {
				break;
			} else {
				error_set("%s: read error", infile);
				return (-1);
			}
		}
	}
	fclose(f);

	memb = &den->members[ind];
	strlcpy(memb->name, name, sizeof(memb->name));
	strlcpy(memb->lang, lang, sizeof(memb->lang));
	memb->offs = offs;
	memb->size = size;
	return (0);
}
