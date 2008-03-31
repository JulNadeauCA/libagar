/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Routines related to error handling.
 */

#include <core/core.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef THREADS
AG_ThreadKey agErrorKey;
#else
char *agErrorKey;
#endif

#ifdef DEBUG
int agDebugLvl = 1;				/* Default debug level */
#endif

void
AG_InitError(void)
{
#ifdef THREADS
	AG_ThreadKeyCreate(&agErrorKey);
#else
	agErrorKey = NULL;
#endif
}

void
AG_DestroyError(void)
{
#ifdef THREADS
#if 0
	/* XXX uninitialized warnings */
	AG_ThreadKeyDelete(agErrorKey);
#endif
#else
	Free(agErrorKey);
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

		if ((ekey = (char *)AG_ThreadKeyGet(agErrorKey)) != NULL) {
			Free(ekey);
		}
		AG_ThreadKeySet(agErrorKey, buf);
	}
#else
	Free(agErrorKey);
	agErrorKey = buf;
#endif
}

const char *
AG_GetError(void)
{
#ifdef THREADS
	return ((const char *)AG_ThreadKeyGet(agErrorKey));
#else
	return ((const char *)agErrorKey);
#endif
}

void
AG_Debug(void *p, const char *fmt, ...)
{
#ifdef DEBUG
	AG_Object *obj = p;
	va_list args;
	
	if (obj->debugFn != NULL) {
		char *buf, *p;

		va_start(args, fmt);
		Vasprintf(&buf, fmt, args);
		p = &buf[strlen(buf)-1];
		if (*p == '\n') { *p = '\0'; }
		obj->debugFn(obj, obj->debugPtr, buf);
		Free(buf);
		va_end(args);
	} else {
		if (agDebugLvl >= 2 || OBJECT_DEBUG(obj)) {
			if (agDebugLvl >= 5) {
				char path[128];
				FILE *f;

				Strlcpy(path, obj->name, sizeof(path));
				Strlcat(path, ".debug", sizeof(path));
				va_start(args, fmt);
				if (agDebugLvl >= 10 &&
				    (f = fopen(path, "a")) != NULL) {
					vfprintf(f, fmt, args);
					fclose(f);
				}
				va_end(args);
			} else {
				va_start(args, fmt);
				printf("%s: ", OBJECT(obj)->name);
				vprintf(fmt, args);
				va_end(args);
			}
		}
	}
#endif /* DEBUG */
}

void
AG_Verbose(const char *fmt, ...)
{
	va_list args;

	if (!agVerbose)
		return;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void
AG_FatalError(const char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Fatal error: ");
	if (fmt != NULL) {
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	} else {
		fprintf(stderr, "%s", AG_GetError());
	}
	fprintf(stderr, "\n");
	abort();
}

void *AG_PtrMismatch(void) { AG_FatalError("AG_PTR mismatch"); return (NULL); }
int AG_IntMismatch(void) { AG_FatalError("AG_INT mismatch"); return (0); }
float AG_FloatMismatch(void) { AG_FatalError("AG_FLOAT mismatch");
                               return (0.0); }

void *
AG_ObjectMismatch(const char *t1, const char *t2)
{
	AG_FatalError("Object type mismatch (%s != %s)", t1, t2);
	return (NULL);
}
