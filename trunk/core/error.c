/*
 * Copyright (c) 2002-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(_XBOX)
#undef SLIST_ENTRY
#include <windows.h>
#endif

char         *agErrorMsg = NULL;		/* Error message */
AG_ErrorCode  agErrorCode = AG_EUNDEFINED; 	/* Error code */
#ifdef AG_THREADS
AG_ThreadKey agErrorMsgKey;
AG_ThreadKey agErrorCodeKey;
#endif

/* Redirect Verbose() output to "foo-out.txt" file */
/* #define VERBOSE_TO_FILE */

/* Redirect Debug() output to "foo-debug.txt" file */
/* #define DEBUG_TO_FILE */

/* Use AttachConsole() on Windows */
/* #define USE_WIN32_CONSOLE */

int agDebugLvl = 1;			/* Default debug level */

static void (*agErrorCallback)(const char *) = NULL;
static int (*agVerboseCallback)(const char *) = NULL;
static int (*agDebugCallback)(const char *) = NULL;

#ifdef AG_THREADS
static void DestroyErrorMsg(void *msg) { Free(msg); }
#endif /* AG_THREADS */

int
AG_InitErrorSubsystem(void)
{
	agErrorMsg = NULL;
	agErrorCode = AG_EUNDEFINED;

#ifdef AG_THREADS
	if (AG_ThreadKeyTryCreate(&agErrorMsgKey, DestroyErrorMsg) == -1 ||
	    AG_ThreadKeyTryCreate(&agErrorCodeKey, NULL) == -1) {
		return (-1);
	}
	AG_ThreadKeySet(agErrorMsgKey, NULL);
	AG_ThreadKeySet(agErrorCodeKey, NULL);
#endif

#if defined(_WIN32) && defined(USE_WIN32_CONSOLE)
	AttachConsole(ATTACH_PARENT_PROCESS);
#endif
	return (0);
}

void
AG_DestroyErrorSubsystem(void)
{
#if defined(_WIN32) && defined(USE_WIN32_CONSOLE)
	FreeConsole();
#endif
	Free(agErrorMsg);
	agErrorMsg = NULL;
	agErrorCode = AG_EUNDEFINED;
#ifdef AG_THREADS
	AG_ThreadKeyDelete(agErrorMsgKey);
	AG_ThreadKeyDelete(agErrorCodeKey);
#endif
}

/* Set the error message string. */
void
AG_SetError(const char *fmt, ...)
{
	va_list args;
#ifdef AG_THREADS
	char *newMsg;
	va_start(args, fmt);
	Vasprintf(&newMsg, fmt, args);
	va_end(args);
	if ((agErrorMsg = (char *)AG_ThreadKeyGet(agErrorMsgKey)) != NULL) {
		free(agErrorMsg);
	}
	AG_ThreadKeySet(agErrorMsgKey, newMsg);
	agErrorMsg = newMsg;
#else
	Free(agErrorMsg);
	va_start(args, fmt);
	Vasprintf(&agErrorMsg, fmt, args);
	va_end(args);
#endif
}

/* Retrieve the error message string. */
const char *
AG_GetError(void)
{
#ifdef AG_THREADS
	return ((const char *)AG_ThreadKeyGet(agErrorMsgKey));
#else
	return ((const char *)agErrorMsg);
#endif
}

const char *
AG_Strerror(int error) 
{
#if defined(_WIN32) && !defined(_XBOX)
	static char *str = NULL;
	char *p;

	if(str != NULL) {
		AG_Free(str);
		str = NULL;
	}
	
	FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&str,
		0, NULL );

	p = strchr(str, '\r');
	if(p != NULL) {
		*p = '\0';
	}

	return (const char *)str;
#else
	return (const char *)strerror(error);
#endif
}

/* Set the symbolic error code. */
void
AG_SetErrorCode(AG_ErrorCode code)
{
#ifdef AG_THREADS
	AG_ThreadKeySet(agErrorCodeKey, (void *)code);
#else
	agErrorCode = code;
#endif
}

/* Retrieve the symbolic error code. */
AG_ErrorCode
AG_GetErrorCode(void)
{
#ifdef AG_THREADS
	return (AG_ErrorCode)AG_ThreadKeyGet(agErrorCodeKey);
#else
	return (agErrorCode);
#endif
}

/* Issue a debug message. */
void
AG_Debug(void *p, const char *fmt, ...)
{
#ifdef AG_DEBUG
	AG_Object *obj = p;
	va_list args;
	
	if (agDebugCallback != NULL) {
		char *buf;

		if (obj != NULL) {
			if (obj->name[0] != '\0') {
				Asprintf(&buf, "%s: ", obj->name);
			} else {
				Asprintf(&buf, "<%p>: ", obj);
			}
			agDebugCallback(buf);
			free(buf);
		}
		va_start(args, fmt);
		Vasprintf(&buf, fmt, args);
		va_end(args);
		if (agDebugCallback(buf) == 1) {
			free(buf);
			return;
		}
		free(buf);
	}
	if (agDebugLvl >= 1) {
		va_start(args, fmt);
# if defined(DEBUG_TO_FILE)
		/* Redirect output to foo-debug.txt */
		{
			char path[AG_FILENAME_MAX];
			FILE *f;

			if (agProgName != NULL) {
				Strlcpy(path, agProgName, sizeof(path));
				Strlcat(path, "-debug.txt", sizeof(path));
			} else {
				Strlcpy(path, "debug.txt", sizeof(path));
			}
			if ((f = fopen(path, "a")) != NULL) {
				if (obj != NULL) {
					if (obj->name[0] != '\0') {
						fprintf(f, "%s: ", obj->name);
					} else {
						fprintf(f, "<%p>: ", obj);
					}
				}
				vfprintf(f, fmt, args);
				fclose(f);
			}
		}
# elif defined(_WIN32) && defined(USE_WIN32_CONSOLE)
		{
			HANDLE cons;
			char *buf;
		
			cons = GetStdHandle(STD_ERROR_HANDLE);
			if (cons != NULL && cons != INVALID_HANDLE_VALUE) {
				if (obj != NULL && obj->name[0] != '\0') {
					WriteConsole(cons, obj->name, strlen(obj->name), NULL, NULL);
					WriteConsole(cons, ": ", 2, NULL, NULL);
				}
				Vasprintf(&buf, fmt, args);
				WriteConsole(cons, buf, strlen(buf), NULL, NULL);
				free(buf);
			}
		}
# else /* _WIN32 */
		if (obj != NULL) {
			if (obj->name[0] != '\0') {
				printf("%s: ", obj->name);
			} else {
				printf("<%p>: ", obj);
			}
		}
		vprintf(fmt, args);
#endif
		va_end(args);
	}
#endif /* AG_DEBUG */
}

/* Issue a verbose message. */
void
AG_Verbose(const char *fmt, ...)
{
	va_list args;

	if (!agVerbose || fmt == NULL)
		return;
	
	if (agVerboseCallback != NULL) {
		char *buf;
		va_start(args, fmt);
		Vasprintf(&buf, fmt, args);
		va_end(args);
		if (agVerboseCallback(buf) == 1) {
			free(buf);
			return;
		}
		free(buf);
	}

	va_start(args, fmt);
#if defined(VERBOSE_TO_FILE)
	/* Redirect output to foo-out.txt */
	{
		char path[AG_FILENAME_MAX];
		FILE *f;

		if (agProgName != NULL) {
			Strlcpy(path, agProgName, sizeof(path));
			Strlcat(path, "-out.txt", sizeof(path));
		} else {
			Strlcpy(path, "output.txt", sizeof(path));
		}
		if ((f = fopen(path, "a")) != NULL) {
			vfprintf(f, fmt, args);
			fclose(f);
		}
	}
#elif defined(_WIN32) && defined(USE_WIN32_CONSOLE)
	{
		HANDLE cons;
		char *buf;
		
		cons = GetStdHandle(STD_ERROR_HANDLE);
		if (cons != NULL && cons != INVALID_HANDLE_VALUE) {
			Vasprintf(&buf, fmt, args);
			WriteConsole(cons, buf, strlen(buf), NULL, NULL);
			free(buf);
		}
	}
#else
	vprintf(fmt, args);
#endif
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
			free(buf);
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
AG_SetFatalCallback(void (*fn)(const char *))
{
	agErrorCallback = fn;
}

void
AG_SetVerboseCallback(int (*fn)(const char *))
{
	agVerboseCallback = fn;
}

void
AG_SetDebugCallback(int (*fn)(const char *))
{
	agDebugCallback = fn;
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
