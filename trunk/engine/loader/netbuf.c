/*	$Csoft: netbuf.c,v 1.6 2005/09/06 04:13:16 vedge Exp $	*/

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
#include <engine/error/malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <engine/loader/netbuf.h>

AG_Netbuf *
AG_NetbufOpen(const char *path, const char *mode, enum ag_netbuf_endian byte_order)
{
	AG_Netbuf *buf;

	buf = Malloc(sizeof(AG_Netbuf), 0);
	buf->byte_order = byte_order;
	buf->path = Strdup(path);
	if ((buf->file = fopen(path, mode)) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		goto fail;
	}
	return (buf);
fail:
	Free(buf, 0);
	return (NULL);
}

void
AG_NetbufClose(AG_Netbuf *buf)
{
	fclose(buf->file);
	Free(buf->path, 0);
	Free(buf, 0);
}

void
AG_NetbufRead(void *ptr, size_t size, size_t nmemb, AG_Netbuf *buf)
{
	if (fread(ptr, size, nmemb, buf->file) < nmemb) {
		if (feof(buf->file)) {
			fatal("read premature EOF");
		} else {
			fatal("read error");
		}
	}
}

__inline__ ssize_t
AG_NetbufReadE(void *ptr, size_t size, size_t nmemb, AG_Netbuf *buf)
{
	return (fread(ptr, size, nmemb, buf->file));
}

void
AG_NetbufWrite(const void *ptr, size_t size, size_t nmemb, AG_Netbuf *buf)
{
	size_t rv;

	if ((rv = fwrite(ptr, size, nmemb, buf->file)) < nmemb) {
		if (feof(buf->file)) {
			fatal("write premature EOF");
		} else {
			fatal("write error");
		}
	}
}

void
AG_NetbufPwrite(const void *ptr, size_t size, size_t nmemb, off_t offs,
    AG_Netbuf *buf)
{
	long oldoffs;

	oldoffs = ftell(buf->file);

	if (fseek(buf->file, (long)offs, SEEK_SET) == -1)
		fatal("seek1 fail");

	if (fwrite(ptr, size, nmemb, buf->file) < nmemb) {
		if (feof(buf->file)) {
			fatal("pwrite premature EOF");
		} else {
			fatal("pwrite error");
		}
	}

	if (fseek(buf->file, oldoffs, SEEK_SET) == -1)
		fatal("seek2 fail");
}

off_t
AG_NetbufTell(AG_Netbuf *buf)
{
	return (ftell(buf->file));
}

void
AG_NetbufSeek(AG_Netbuf *buf, off_t offs, int whence)
{
	if (fseek(buf->file, (long)offs, whence) == -1)
		fatal("seek fail");
}

void
AG_NetbufFlush(AG_Netbuf *buf)
{
	fflush(buf->file);
}
