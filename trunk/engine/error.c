/*	$Csoft: error.c,v 1.15 2002/12/13 07:51:46 vedge Exp $	*/

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

#include <config/serialization.h>

#include "compat/vasprintf.h"
#include "engine.h"

#ifdef SERIALIZATION
extern pthread_key_t engine_errorkey;	/* engine.c */
#else
extern char *engine_errorkey;		/* engine.c */
#endif

void *
emalloc(size_t len)
{
	void *p;

	p = malloc(len);
	if (p == NULL) {
		perror("malloc");
		engine_stop();
	}
	return (p);
}

void *
erealloc(void *ptr, size_t len)
{
	void *p;

	p = realloc(ptr, len);
	if (p == NULL) {
		perror("realloc");
		engine_stop();
	}
	return (p);
}

void
error_set(const char *fmt, ...)
{
	va_list args;
	char *ekey, *buf;
	
	va_start(args, fmt);
	if (vasprintf(&buf, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);

#ifdef SERIALIZATION
	ekey = (char *)pthread_getspecific(engine_errorkey);
	if (ekey != NULL) {
		free(ekey);
	}
	pthread_setspecific(engine_errorkey, buf);
#else
	Free(engine_errorkey);
	engine_errorkey = buf;
#endif
}

const char *
error_get(void)
{
#ifdef SERIALIZATION
	return ((const char *)pthread_getspecific(engine_errorkey));
#else
	return ((const char *)engine_errorkey);
#endif
}

void
_dprintf_noop(const char *fmt, ...)
{
}

void
_debug_noop(int level, const char *fmt, ...)
{
}

void
_dprintf(const char *fmt, ...)
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
_debug(int mask, const char *fmt, ...)
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
_debug_n(int mask, const char *fmt, ...)
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

char *
Strdup(const char *s)
{
	char *sd;

	if ((sd = strdup(s)) == NULL) {
		fatal("strdup: out of memory\n");
	}
	return (sd);
}

