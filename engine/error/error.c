/*	$Csoft: error.c,v 1.10 2004/09/12 05:55:56 vedge Exp $	*/

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
int engine_debug = 1;				/* Default debug level */
struct error_mement error_mements[M_LAST];
#endif

void
error_init(void)
{
#ifdef DEBUG
	int i;

	for (i = 0; i < M_LAST; i++)
		memset(&error_mements[i], 0, sizeof(struct error_mement));
#endif

#ifdef THREADS
	pthread_key_create(&error_key, NULL);
#else
	error_key = NULL;
#endif
}

void
error_destroy(void)
{
#ifdef THREADS
	pthread_key_delete(error_key);
#else
	Free(error_key, 0);
#endif
}

void
error_set(const char *fmt, ...)
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
error_get(void)
{
#ifdef THREADS
	return ((const char *)pthread_getspecific(error_key));
#else
	return ((const char *)error_key);
#endif
}

void *
error_malloc(size_t len, int type)
{
	void *p;
	
	if ((p = malloc(len)) == NULL)
		fatal("malloc");
#ifdef DEBUG
	if (type > 0) {
		error_mements[type].nallocs++;
		error_mements[type].msize += len;
	}
#endif
	return (p);
}

void *
error_realloc(void *oldp, size_t len)
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
error_free(void *p, int type)
{
	/* XXX redundant on some systems */
	if (p == NULL)
		return;
#ifdef DEBUG
	if (type > 0)
		error_mements[type].nfrees++;
#endif
	free(p);
}

void
error_dprintf(const char *fmt, ...)
{
#ifdef DEBUG
	if (engine_debug > 0) {
		va_list args;

		va_start(args, fmt);
		printf(fmt, args);
		va_end(args);
	}
#endif
}

void
error_dprintf_nop(const char *fmt, ...)
{
}

void
error_debug(int mask, const char *fmt, ...)
{
#ifdef DEBUG
	if (engine_debug & mask) {
		va_list args;

		va_start(args, fmt);
		printf(fmt, args);
		printf("\n");
		va_end(args);
	}
#endif
}

void
error_debug_nop(int level, const char *fmt, ...)
{
}

void
error_debug_n(int mask, const char *fmt, ...)
{
#ifdef DEBUG
	if (engine_debug & mask) {
		va_list args;

		va_start(args, fmt);
		fprintf(stderr, fmt, args);
		va_end(args);
	}
#endif
}

void
error_fatal(const char *fmt, ...)
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
error_strdup(const char *s)
{
	size_t buflen;
	char *ns;
	
	buflen = strlen(s)+1;
	ns = Malloc(buflen, 0);
	memcpy(ns, s, buflen);
	return (ns);
}

