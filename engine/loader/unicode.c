/*	$Csoft: unicode.c,v 1.3 2003/06/22 06:34:19 vedge Exp $	*/

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

#include <engine/error/error.h>

#include <sys/types.h>
#include <SDL_types.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <engine/unicode/unicode.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>
#include <engine/loader/unicode.h>

enum {
	UNICODE_UTF16BE			/* Length-encoded, UTF-16BE */
};

Uint16 *
read_unicode(struct netbuf *buf)
{
	Uint32 encoding;
	Uint16 *ucs;
	size_t len;
	int i;

	encoding = read_uint32(buf);
	switch (encoding) {
	case UNICODE_UTF16BE:
		len = (size_t)read_uint32(buf);
		if (len > UNICODE_STRING_MAX) {
			error_set("unicode string is too big");
			return (NULL);
		} else if (len == 0) {
			error_set("NULL unicode string");
			return (NULL);
		}
		ucs = Malloc(len * sizeof(Uint16));	     /* Includes NUL */
		for (i = 0; i < len; i++) {
			ucs[i] = read_uint16(buf);
		}
		if (ucs[len-1] != '\0') {
			free(ucs);
			error_set("no NUL termination");
			return (NULL);
		}
		break;
	default:
		error_set("bad encoding");
		return (NULL);
	}
	return (ucs);
}

void
write_unicode(struct netbuf *buf, const Uint16 *ucs)
{
	size_t len;
	int i;

	if (ucs == NULL) {
		write_uint32(buf, 0);
	} else {
		len = ucslen(ucs)+1;			 /* Include the NUL */
		write_uint32(buf, UNICODE_UTF16BE);
		write_uint32(buf, (Uint32)len);
		for (i = 0; i < len; i++) {
			write_uint16(buf, ucs[i]);
		}
	}
}

/*
 * Copy at most dst_size bytes from an encoded Unicode string to a fixed-size
 * UTF-16 buffer, returning the number of bytes (not characters) that would have
 * been copied were dst_size unlimited. 
 */
size_t
copy_unicode(Uint16 *dst, struct netbuf *buf, size_t dst_size)
{
	Uint32 encoding;
	size_t len;
	int i;

	encoding = read_uint32(buf);
	switch (encoding) {
	case UNICODE_UTF16BE:
		len = (size_t)read_uint32(buf);
		for (i = 0;
		     i < len && i*sizeof(Uint16) < dst_size-1;
		     i++) {
			dst[i] = read_uint16(buf);
		}
		if (i < len) {
			dst[i] = 0;
			fprintf(stderr,
			   "%u-char UTF-16 string truncated to fit %u\n",
			   (unsigned)len, (unsigned)dst_size/sizeof(Uint16));
		} else if (dst[len-1] != '\0') {
			fatal("no terminating NUL");
		}
		break;
	default:
		fatal("bad encoding");
		break;
	}
	return ((len-1) * sizeof(Uint16));	/* Count does not include NUL */
}
