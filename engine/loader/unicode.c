/*	$Csoft: unicode.c,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/loader/unicode.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

enum {
	UNICODE_MAX = 32767
};

Uint16 *
read_unicode(struct netbuf *buf)
{
	size_t len;
	Uint16 *ucs;

	if ((len = (size_t)read_uint32(buf)) > UNICODE_MAX) {
		fatal("string is too big");
	} else if (len == 0) {
		fatal("null string");
	}
	ucs = Malloc(len);
	netbuf_read(ucs, len, 1, buf);
	return (ucs);
}

void
write_unicode(struct netbuf *buf, const Uint16 *ucs)
{
	size_t len;

	if ((len = (ucslen(ucs)+1) * sizeof(Uint16)) > UNICODE_MAX) {
		fatal("string is too big");
	}
	write_uint32(buf, (Uint32)len);
	netbuf_write(ucs, len, 1, buf);
}

/*
 * Copy at most dst_size bytes from a NUL-terminated, length-encoded UTF-16
 * string to a fixed-size buffer, returning the number of bytes that would
 * have been copied were dst_size unlimited. 
 */
size_t
copy_unicode(Uint16 *dst, struct netbuf *buf, size_t dst_size)
{
	size_t len;
	size_t rv = dst_size;
	ssize_t rrv;

	/* The terminating NUL is included in the encoding. */
	if ((len = read_uint32(buf)) > dst_size) {
		fprintf(stderr, "%lu byte UTF-16 string truncated to fit %lu\n",
		    (unsigned long)len, (unsigned long)dst_size);
		rv = len;				/* Save */
		len = dst_size;				/* Truncate */
	}

	if ((rrv = fread(dst, 1, len, buf->file)) < len) {
		if (ferror(buf->file) || feof(buf->file)) {
			fprintf(stderr,
			    "error reading UTF-16 string (truncated)\n");
			if (dst_size > 0) {
				dst[0] = '\0';
				rv = 1;
			}
		} else {
			fprintf(stderr, "short read: %lu/%lu (truncated)\n",
			    (unsigned long)rrv, (unsigned long)len);
			rv = rrv;
			dst[rrv] = '\0';		/* NUL-terminate */
		}
	}
	return (rv - 1);			/* Count does not include NUL */
}
