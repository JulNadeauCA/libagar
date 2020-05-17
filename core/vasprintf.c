/*
 * Copyright (c) 2002-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/have_vasprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>

#if (defined(__APPLE__) && !defined(_DARWIN_C_SOURCE))
#define _DARWIN_C_SOURCE
#endif
#ifdef __NetBSD__
#define _NETBSD_SOURCE
#endif
#define _GNU_SOURCE

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <agar/core/core.h>

int
AG_TryVasprintf(char **ret, const char *fmt, va_list ap)
{
#ifndef HAVE_VASPRINTF
	int size;

	size = vsnprintf(NULL, 0, fmt, ap);
	if ((*ret = TryMalloc(size+1)) == NULL) {
		return (-1);
	}
	if (size == 0) {
		(*ret)[0] = '\0';
		return (0);
	} else {
		return vsprintf(*ret, fmt, ap);
	}
#else /* !HAVE_VASPRINTF */
	if (vasprintf(ret, fmt, ap) == -1) {
		AG_SetError("Out of memory");
		return (-1);
	}
	return (0);
#endif /* HAVE_VASPRINTF */
}

void
AG_Vasprintf(char **s, const char *fmt, va_list args)
{
	if (AG_TryVasprintf(s, fmt, args) == -1) 
		AG_FatalError(NULL);
}
