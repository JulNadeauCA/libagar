/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#ifdef __CC65__
# include <conio.h>
#endif

#if defined(_WIN32) && !defined(_XBOX)
# undef SLIST_ENTRY
# include <windows.h>
#endif

char *_Nullable agErrorMsg  = NULL;		/* Last error message (UTF-8) */
AG_ErrorCode    agErrorCode = AG_EUNDEFINED; 	/* Last error code */
#ifdef AG_THREADS
AG_ThreadKey    agErrorMsgKey;    /* Last error message (UTF-8; thread-local) */
AG_ThreadKey    agErrorCodeKey;   /* Last error code (thread-local) */
#endif

int agDebugLvl = 1;				/* Default debug level */

/* Redirect Verbose() output to "foo-out.txt" file */
/* #define VERBOSE_TO_FILE */

/* Redirect Debug() output to "foo-debug.txt" file */
/* #define DEBUG_TO_FILE */

/* Use AttachConsole() on Windows */
/* #define USE_WIN32_CONSOLE */

static void (*_Nullable agErrorCallback)(const char *_Nonnull) = NULL;
static int  (*_Nullable agVerboseCallback)(const char *_Nonnull) = NULL;
static int  (*_Nullable agDebugCallback)(const char *_Nonnull) = NULL;

#ifdef __CC65__
int ag65consoleX = 0, ag65consoleY = 0;
#endif

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_error.h>

#ifdef AG_THREADS
static void
DestroyErrorMsg(void *_Nullable msg)
{
	Free(msg);
}
#endif

int
AG_InitErrorSubsystem(void)
{
	agErrorMsg = Strdup("No error");
	agErrorCode = AG_EUNDEFINED;

#ifdef AG_THREADS
	if (AG_ThreadKeyTryCreate(&agErrorMsgKey, DestroyErrorMsg) == -1 ||
	    AG_ThreadKeyTryCreate(&agErrorCodeKey, NULL) == -1) {
		return (-1);
	}
	AG_ThreadKeySet(agErrorMsgKey, Strdup("No error"));
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

/* Set the error message string (C string). */
void
AG_SetErrorS(const char *msg)
{
	char *newMsg;
	
	if ((newMsg = TryStrdup(msg)) == NULL)
		return;
#ifdef AG_THREADS
	if ((agErrorMsg = (char *)AG_ThreadKeyGet(agErrorMsgKey)) != NULL) {
		free(agErrorMsg);
	}
	AG_ThreadKeySet(agErrorMsgKey, newMsg);
#else
	Free(agErrorMsg);
#endif
	agErrorMsg = newMsg;
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

/*
 * Print a debug message on the error console. Optionally, prefix the
 * message with a label containing the name/address of an object.
 */
void
AG_Debug(void *pObj, const char *fmt, ...)
{
#if defined(AG_DEBUG) || defined(__CC65__)
	AG_Object *obj = pObj;
	va_list args;
# ifdef __CC65__
	Uint8 colorSave, wScr,hScr;

	if (agDebugLvl < 1)
		return;
	
	va_start(args, fmt);
	colorSave = textcolor(COLOR_CYAN);
	gotoxy(ag65consoleX, ag65consoleY);
	if (obj != NULL) {
		fputs(obj->name, stdout);
		fputc(':', stdout);
	}
	vcprintf(fmt, args);
	textcolor(colorSave);
	va_end(args);

	screensize(&wScr, &hScr);
	if (++ag65consoleY > hScr) {
		clrscr();
		ag65consoleY = 0;
	}
# else /* !__CC65__ */

	if (agDebugLvl < 1) {
		return;
	}
	if (agDebugCallback != NULL) {
		char *buf;

		if (obj != NULL) {
			size_t bufLen;

			if (obj->name[0] != '\0') {
				bufLen = 5+strlen(obj->name)+6+1;
				buf = Malloc(bufLen);
				Strlcpy(buf, AGSI_ITALIC, bufLen);
				Strlcat(buf, obj->name, bufLen);
				Strlcat(buf, AGSI_RST ": ", bufLen);
			} else {
				bufLen = 6+2+(AG_MODEL >> 2)+7+1;
				buf = Malloc(bufLen);
				Snprintf(buf, bufLen, "<%p>: ", obj);
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
	va_start(args, fmt);
#  if defined(DEBUG_TO_FILE)
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
#  elif defined(_WIN32) && defined(USE_WIN32_CONSOLE)
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
#  else /* !_WIN32 */
	if (obj != NULL) {
		if (obj->name[0] != '\0') {
			printf(AGSI_ITALIC "%s" AGSI_RST ": ", obj->name);
		} else {
			printf("<" AGSI_ITALIC "%p" AGSI_RST ">: ", obj);
		}
	}
	vprintf(fmt, args);
#  endif
	va_end(args);
# endif /* !__CC65__ */
#endif /* AG_DEBUG */
}

/* Issue a verbose message. */
void
AG_Verbose(const char *fmt, ...)
{
#if defined(AG_VERBOSITY) || defined(__CC65__)
# ifdef __CC65__
	va_list args;
	Uint8 colorSave, wScr,hScr;
	
	va_start(args, fmt);
	colorSave = textcolor(COLOR_WHITE);
	gotoxy(ag65consoleX, ag65consoleY);
	vcprintf(fmt, args);
	textcolor(colorSave);
	va_end(args);

	screensize(&wScr, &hScr);
	if (++ag65consoleY > hScr) {
		clrscr();
		ag65consoleY = 0;
	}
# else /* !__CC65__ */

	va_list args;

	if (!agVerbose)
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
#  if defined(VERBOSE_TO_FILE)
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
#  elif defined(_WIN32) && defined(USE_WIN32_CONSOLE)
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
#  else
	vprintf(fmt, args);
#  endif
	va_end(args);
# endif /* !__CC65__ */
#endif /* AG_VERBOSITY */
}

/* Raise a fatal error condition (format string). */
void
AG_FatalErrorF(const char *fmt, ...)
{
	va_list args;
	char *s;

	if (fmt == NULL) {
		AG_FatalError(NULL);
	}
	va_start(args, fmt);
	Vasprintf(&s, fmt, args);
	va_end(args);
	AG_FatalError(s);
}

/* Raise a fatal error condition. */
void
AG_FatalError(const char *msg)
{
#ifdef __CC65__
	textcolor(COLOR_LIGHTRED);
#endif
	if (agErrorCallback != NULL) {
		agErrorCallback(msg ? msg : AG_GetError());
		abort(); /* not reached */
	} else {
		fputs("AG_FatalError: ", stderr);
		if (msg != NULL) {
			fputs(msg, stderr);
		} else {
			fputs(AG_GetError(), stderr);
		}
		fputc('\n', stderr);
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

#ifdef AG_TYPE_SAFETY
/*
 * Raise fatal error condition due to a runtime type checking error
 * (if compiled with either --enable-debug or --enable-type-safety).
 */
# ifdef AG_VERBOSITY
void  *AG_GenericMismatch(const char *s) { AG_FatalErrorF("Illegal access: %s", s); }
# else
void  *AG_GenericMismatch(const char *s) { AG_FatalErrorV("E29", s); }
# endif
void  *AG_PtrMismatch(void) { AG_FatalErrorV("E290", "Illegal AG_PTR() / AG_CONST_PTR() access"); }
char  *AG_StringMismatch(void) { AG_FatalErrorV("E291", "Illegal AG_STRING() access"); }
int    AG_IntMismatch(void) { AG_FatalErrorV("E292", "Illegal AG_INT() / AG_UINT() access"); }
long   AG_LongMismatch(void) { AG_FatalErrorV("E293", "Illegal AG_LONG() / AG_ULONG() access"); }
float  AG_FloatMismatch(void) { AG_FatalErrorV("E294", "Illegal AG_FLOAT() access"); }
double AG_DoubleMismatch(void) { AG_FatalErrorV("E295", "Illegal AG_DOUBLE() access"); }
void  *AG_ObjectMismatch(void) { AG_FatalErrorV("E296", "Illegal AG_OBJECT() / AG_CONST_OBJECT() access"); }
#endif /* AG_TYPE_SAFETY */
