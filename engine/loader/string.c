/*	$Csoft: string.c,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/

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
#include <string.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>
#include <engine/loader/string.h>

enum {
	STRING_MAX = 32767
};

/* Allocate and read a NUL-terminated, length-encoded string. */
char *
read_string(struct netbuf *buf)
{
	size_t len;
	char *s;

	if ((len = (size_t)read_uint32(buf)) > STRING_MAX) {
		fatal("string is too big");
	} else if (len == 0) {
		fatal("null string");
	}

	s = Malloc(len);
	netbuf_read(s, len, 1, buf);

	if (s[len-1] != '\0') {
		fatal("string is not NUL-terminated");
	}
	return (s);
}

/* Write a NUL-terminated, length-encoded string. */
void
write_string(struct netbuf *buf, const char *s)
{
	size_t len;

	if ((len = strlen(s)+1) > STRING_MAX) {
		fatal("string too big");
	}
	write_uint32(buf, (Uint32)len);
	netbuf_write(s, len, 1, buf);
}

/*
 * Copy at most dst_size bytes from a NUL-terminated, length-encoded string
 * to a fixed-size buffer, returning the number of bytes that would have
 * been copied were dst_size unlimited. 
 */
size_t
copy_string(char *dst, struct netbuf *buf, size_t dst_size)
{
	size_t rv = dst_size;
	size_t len;
	ssize_t rrv;

	/* The terminating NUL is included in the encoding. */
	if ((len = (size_t)read_uint32(buf)) > dst_size) {
		fprintf(stderr, "at 0x%x:\n", (unsigned)netbuf_tell(buf));
		fprintf(stderr, "%lu byte string truncated to fit %lu\n",
		    (unsigned long)len, (unsigned long)dst_size);
		rv = len;				/* Save */
		len = dst_size;				/* Truncate */
		abort();
	}

	if ((rrv = fread(dst, 1, len, buf->file)) < len) {
		if (ferror(buf->file) || feof(buf->file)) {
			fprintf(stderr, "error reading string (truncated)\n");
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

