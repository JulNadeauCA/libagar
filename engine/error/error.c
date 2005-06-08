/*	$Csoft: error.c,v 1.11 2005/01/05 04:44:03 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/threads.h>

#include <compat/vasprintf.h>
#include <compat/queue.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifdef THREADS
#include <pthread.h>
#endif

#include <engine/error/error.h>

#ifdef THREADS
pthread_key_t error_key;
#else
char *error_key;
#endif

#ifdef DEBUG
int agDebugLvl = 1;				/* Default debug level */
struct ag_malloc_type agMallocTypes[M_LAST];
#endif

void
AG_InitError(void)
{
#ifdef DEBUG
	int i;

	for (i = 0; i < M_LAST; i++)
		memset(&agMallocTypes[i], 0, sizeof(struct ag_malloc_type));
#endif

#ifdef THREADS
	pthread_key_create(&error_key, NULL);
#else
	error_key = NULL;
#endif
}

void
AG_DestroyError(void)
{
#ifdef THREADS
	pthread_key_delete(error_key);
#else
	Free(error_key, 0);
#endif
}

void
AG_SetError(const char *fmt, ...)
{
	va_list args;
	char *buf;
	
	va_start(args, fmt);
	Vasprintf(&buf, fmt, args);
	va_end(args);
#ifdef THREADS
	{
		char *ekey;

		ekey = (char *)pthread_getspecific(error_key);
		if (ekey != NULL) {
			free(ekey);
		}
		pthread_setspecific(error_key, buf);
	}
#else
	Free(error_key, 0);
	error_key = buf;
#endif
}

const char *
AG_GetError(void)
{
#ifdef THREADS
	return ((const char *)pthread_getspecific(error_key));
#else
	return ((const char *)error_key);
#endif
}

void *
AG_Malloc(size_t len, int type)
{
	void *p;
	
	if ((p = malloc(len)) == NULL)
		fatal("malloc");
#ifdef DEBUG
	if (type > 0) {
		agMallocTypes[type].nallocs++;
		agMallocTypes[type].msize += len;
	}
#endif
	return (p);
}

void *
AG_Realloc(void *oldp, size_t len)
{
	void *newp;

	/* XXX redundant with most reallocs */
	if (oldp == NULL) {
		if ((newp = malloc(len)) == NULL)
			fatal("malloc");
	} else {
		if ((newp = realloc(oldp, len)) == NULL)
			fatal("realloc");
	}
	return (newp);
}

void
AG_Free(void *p, int type)
{
	/* XXX redundant on some systems */
	if (p == NULL)
		return;
#ifdef DEBUG
	if (type > 0)
		agMallocTypes[type].nfrees++;
#endif
	free(p);
}

void
AG_DebugPrintf(const char *fmt, ...)
{
#ifdef DEBUG
	if (agDebugLvl > 0) {
		va_list args;

		va_start(args, fmt);
		printf(fmt, args);
		va_end(args);
	}
#endif
}

void
AG_DebugPrintfNop(const char *fmt, ...)
{
}

void
AG_Debug(int mask, const char *fmt, ...)
{
#ifdef DEBUG
	if (agDebugLvl & mask) {
		va_list args;

		va_start(args, fmt);
		printf(fmt, args);
		printf("\n");
		va_end(args);
	}
#endif
}

void
AG_DebugNop(int level, const char *fmt, ...)
{
}

void
AG_DebugN(int mask, const char *fmt, ...)
{
#ifdef DEBUG
	if (agDebugLvl & mask) {
		va_list args;

		va_start(args, fmt);
		fprintf(stderr, fmt, args);
		va_end(args);
	}
#endif
}

void
AG_FatalError(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "fatal: ");
	fprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	abort();
}

char *
AG_Strdup(const char *s)
{
	size_t buflen;
	char *ns;
	
	buflen = strlen(s)+1;
	ns = Malloc(buflen, 0);
	memcpy(ns, s, buflen);
	return (ns);
}

