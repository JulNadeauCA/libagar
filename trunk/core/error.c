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
AG_ThreadKey agErrorKey;		/* Error message (thread-specific) */
AG_ThreadKey agErrorCode;		/* Error code (thread-specific) */
#else
char *agErrorKey;			/* Error message */
AG_ErrorCode agErrorCode; 		/* Error code */
#endif
int agDebugLvl = 1;			/* Default debug level */

/* Error callback for AG_FatalError() */
static void (*agErrorCallback)(const char *) = NULL;

/* Initialize the error facility. */
void
AG_InitError(void)
{
#ifdef THREADS
	AG_ThreadKeyCreate(&agErrorKey);
	AG_ThreadKeyCreate(&agErrorCode);
#else
	agErrorKey = NULL;
	agErrorCode = AG_EUNDEFINED;
#endif
}

/* Destroy the error facility. */
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

/* Set the error message string. */
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

/* Retrieve the error message string. */
const char *
AG_GetError(void)
{
#ifdef THREADS
	return ((const char *)AG_ThreadKeyGet(agErrorKey));
#else
	return ((const char *)agErrorKey);
#endif
}

/* Set the symbolic error code. */
void
AG_SetErrorCode(AG_ErrorCode code)
{
#ifdef THREADS
	AG_ThreadKeySet(agErrorCode, (void *)code);
#else
	agErrorCode = code;
#endif
}

/* Retrieve the symbolic error code. */
AG_ErrorCode
AG_GetErrorCode(void)
{
#ifdef THREADS
	return ((AG_ErrorCode)AG_ThreadKeyGet(agErrorCode));
#else
	return agErrorCode;
#endif
}

/* Issue a debug message. */
void
AG_Debug(void *p, const char *fmt, ...)
{
#ifdef DEBUG
	AG_Object *obj = p;
	va_list args;
	
	if (obj != NULL && obj->debugFn != NULL) {
		char *buf, *p;

		va_start(args, fmt);
		Vasprintf(&buf, fmt, args);
		p = &buf[strlen(buf)-1];
		if (*p == '\n') { *p = '\0'; }
		obj->debugFn(obj, obj->debugPtr, buf);
		Free(buf);
		va_end(args);
	} else {
		if (agDebugLvl >= 1 || (obj != NULL && OBJECT_DEBUG(obj))) {
			va_start(args, fmt);
			if (obj != NULL) {
				if (OBJECT(obj)->name[0] != '\0') {
					printf("%s: ", OBJECT(obj)->name);
				} else {
					printf("<%p>: ", obj);
				}
			}
			vprintf(fmt, args);
			va_end(args);
		}
	}
#endif /* DEBUG */
}

/* Issue a verbose message. */
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

/* Raise a fatal error condition. */
void
AG_FatalError(const char *fmt, ...)
{
	va_list args;
  char *buf;

	/* Use callback if defined. The callback must gracefully exit. */
	if (agErrorCallback != NULL) {
		if (fmt != NULL) {
			va_start(args, fmt);
			Vasprintf(&buf, fmt, args);
			va_end(args);
			agErrorCallback(buf);
			abort(); /* not reached */
		} else {
			agErrorCallback(AG_GetError());
			abort(); /* not reached */
		}
	} else {
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
}

void
AG_SetFatalCallback(void (*callback)(const char *))
{
	agErrorCallback = callback;
}

/*
 * Raise fatal error condition due to a type access mismatch in an event
 * handler routine.
 */
void *
AG_PtrMismatch(void)
{
	AG_FatalError("AG_PTR mismatch");
	return (NULL);
}
int
AG_IntMismatch(void)
{
	AG_FatalError("AG_INT mismatch");
	return (0);
}
float
AG_FloatMismatch(void)
{
	AG_FatalError("AG_FLOAT mismatch");
	return (0.0);
}
void *
AG_ObjectMismatch(const char *t1, const char *t2)
{
	AG_FatalError("Object type mismatch (%s != %s)", t1, t2);
	return (NULL);
}
