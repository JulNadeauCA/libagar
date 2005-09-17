/*	$Csoft: string.c,v 1.9 2005/01/05 04:44:04 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

/* Allocate and read a length-encoded string. */
char *
read_string_len(struct netbuf *buf, size_t maxlen)
{
	size_t len;
	char *s;

	if ((len = (size_t)read_uint32(buf))+1 >= maxlen) {
		error_set("String overflow");
		return (NULL);
	}
	if ((s = malloc(len+1)) == NULL) {
		error_set("Out of memory for string");
		return (NULL);
	}
	if (len > 0) {
		netbuf_read(s, len, 1, buf);
	}
	s[len] = '\0';
	return (s);
}

/* Allocate and read a NUL-terminated, length-encoded string. */
char *
read_nulstring_len(struct netbuf *buf, size_t maxlen)
{
	size_t len;
	char *s;

	if ((len = (size_t)read_uint32(buf)) >= maxlen) {
		error_set("String overflow");
		return (NULL);
	}
	if (len == 0) {
		error_set("NULL string");
		return (NULL);
	}
	if ((s = malloc(len)) == NULL) {
		error_set("Out of memory for string");
		return (NULL);
	}
	netbuf_read(s, len, 1, buf);
	return (s);
}

/* Write a length-encoded string. */
void
write_string(struct netbuf *buf, const char *s)
{
	size_t len;

	if (s == NULL || s[0] == '\0') {
		write_uint32(buf, 0);
	} else {
		len = strlen(s);
		write_uint32(buf, (Uint32)len);
		netbuf_write(s, len, 1, buf);
	}
}

/*
 * Copy at most dst_size bytes from a length-encoded string to a fixed-size
 * buffer, returning the number of bytes that would have been copied were
 * dst_size unlimited. The function NUL-terminates the string.
 */
size_t
copy_string(char *dst, struct netbuf *buf, size_t dst_size)
{
	size_t rv, len;
	ssize_t rrv;

	if ((len = (size_t)read_uint32(buf)) > (dst_size-1)) {
#ifdef DEBUG
		fprintf(stderr, "0x%x: %lub string truncated to fit %lub\n",
		    (unsigned)netbuf_tell(buf), (unsigned long)len,
		    (unsigned long)dst_size);
#endif
		rv = len+1;		/* Save the intended length */
		len = dst_size-1;
	} else {
		rv = len;
	}

	if (len == 0) {
		dst[0] = '\0';
	} else {
		if ((rrv = fread(dst, 1, len, buf->file)) < len) {
			if (ferror(buf->file) || feof(buf->file)) {
				fatal("read error");
			} else {
				fatal("short read");
			}
		} else {
			dst[rrv] = '\0';
		}
	}
	return (rv);				/* Count does not include NUL */
}

/*
 * Copy at most dst_size bytes from a length-encoded, NUL-terminated string
 * to a fixed-size buffer, returning the number of bytes that would have been
 * copied were dst_size unlimited.
 */
size_t
copy_nulstring(char *dst, struct netbuf *buf, size_t dst_size)
{
	size_t rv, len;
	ssize_t rrv;

	if ((len = (size_t)read_uint32(buf)) > dst_size) {
#ifdef DEBUG
		fprintf(stderr, "0x%x: %lub string truncated to fit %lub\n",
		    (unsigned)netbuf_tell(buf), (unsigned long)len,
		    (unsigned long)dst_size);
#endif
		rv = len;		/* Save the intended length */
		len = dst_size;
	} else {
		rv = len;
	}

	if ((rrv = fread(dst, 1, len, buf->file)) < len) {
		if (ferror(buf->file) || feof(buf->file)) {
			fatal("read error");
		} else {
			fatal("short read");
		}
	}
	return (rv-1);				/* Count does not include NUL */
}
