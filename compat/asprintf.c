/*	$Csoft: asprintf.c,v 1.1 2004/02/26 09:19:37 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <config/have_asprintf.h>

#ifndef HAVE_ASPRINTF

#include <engine/engine.h>
#include "asprintf.h"

#include <stdio.h>
#include <stdarg.h>

int
asprintf(char **ret, const char *fmt, ...)
{
	char *buf;
	int size;
	size_t buflen;
	va_list ap;

	buflen = strlen(fmt) + 64;		/* Guess */
	buf = Malloc(buflen, 0);
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return (size);
	}

	buf = Realloc(buf, size+1, 0);		/* Too large */
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return (size);
}

#endif	/* !HAVE_ASPRINTF */

