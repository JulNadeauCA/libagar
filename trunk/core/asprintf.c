/*
 * Copyright (c) 2002-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/config/have_asprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <agar/core/core.h>

#ifndef HAVE_ASPRINTF

int
AG_TryAsprintf(char **ret, const char *fmt, ...)
{
	char *buf, *bufNew;
	int size;
	size_t buflen;
	va_list ap;

	buflen = strlen(fmt) + 128;	/* Guess */
	if ((buf = TryMalloc(buflen)) == NULL) {
		return (-1);
	}
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return (size);
	}
	if ((bufNew = TryRealloc(buf, size+1)) == NULL) {
		Free(buf);
		return (-1);
	}
	buf = bufNew;
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return (size);
}

void
AG_Asprintf(char **ret, const char *fmt, ...)
{
	char *buf, *bufNew;
	int size;
	size_t buflen;
	va_list ap;

	buflen = strlen(fmt) + 128;	/* Guess */
	if ((buf = TryMalloc(buflen)) == NULL) {
		goto fail;
	}
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return;
	}
	if ((bufNew = TryRealloc(buf, size+1)) == NULL) {
		Free(buf);
		goto fail;
	}
	buf = bufNew;
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return;
fail:
	AG_FatalError("asprintf: Out of memory");
}

#else /* HAVE_ASPRINTF */

int
AG_TryAsprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = AG_TryVasprintf(ret, fmt, ap);
	va_end(ap);

	if (rv == -1) {
		AG_SetError("asprintf: Out of memory");
		return (-1);
	}
	return (rv);
}

void
AG_Asprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = AG_TryVasprintf(ret, fmt, ap);
	va_end(ap);

	if (rv == -1)
		AG_FatalError("asprintf: Out of memory");
}
#endif /* !HAVE_ASPRINTF */
