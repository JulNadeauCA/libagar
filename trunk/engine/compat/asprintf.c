/*	$Csoft: vasprintf.c,v 1.4 2002/09/06 00:50:43 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <engine/mcconfig.h>

#ifndef HAVE_ASPRINTF

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include <engine/error.h>

#include "asprintf.h"

int
asprintf(char **ret, const char *fmt, ...)
{
	char *buf;
	int size;
	size_t buflen;
	va_list ap;

	/* Make a guess, might save one call. */
	buflen = strlen(fmt) + 128;
	buf = emalloc(buflen);
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return (size);
	}

	/* Too large. */
	buf = erealloc(buf, size);
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return (size);
}

#endif	/* !HAVE_ASPRINTF */

