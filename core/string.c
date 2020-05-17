/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * C-string related routines.
 */

#include <agar/core/core.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include <agar/config/have_iconv.h>
#ifdef HAVE_ICONV
# include <agar/config/have_iconv_const.h>
# include <iconv.h>
#endif

/* Print buffer and table of format extensions for AG_Printf() */
#ifdef AG_ENABLE_STRING
static char *_Nullable agPrintBuf[AG_STRING_BUFFERS_MAX];
# ifdef AG_THREADS
static AG_ThreadKey agPrintBufKey[AG_STRING_BUFFERS_MAX];
# endif
static AG_FmtStringExt *_Nullable agFmtExtensions = NULL;
static Uint                       agFmtExtensionCount = 0;
# ifdef AG_THREADS
static _Nullable_Mutex AG_Mutex   agFmtExtensionsLock;
# endif
#endif /* AG_ENABLE_STRING */

/* Map character encodings to possible newlines and their values. */
const AG_NewlineFormat agNewlineFormats[] = {
  { "US-ASCII", "LF",    "\n",1 },   /* Unix, Amiga, BeOS, Multics */
  { "US-ASCII", "CR+LF", "\r\n",2 }, /* DOS/Windows, early non-Unix */
  { "US-ASCII", "CR",    "\r",1 },   /* Commodore 8-bit machines (C64/128) */
#if 0
  { "US-ASCII", "LF+CR", "\n\r",2 }, /* Acorn BBC and RISC OS */
  { "ATASCII",  "ATACR", "\x9b",1 }, /* Atari 8-bit machines */
  { "EBCDIC",   "NL",    "\x15",1 }, /* IBM mainframes */
#endif
  { NULL,       NULL,    NULL,0 }
};

#include <agar/core/string_strcasecmp.h>

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_string.h>

#ifdef AG_ENABLE_STRING
/*
 * Built-in extended format specifiers.
 */
static AG_Size
PrintU8(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Uint8 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyUint(dst, (Uint)*val, dstSize);
}
static AG_Size
PrintS8(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Sint8 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyInt(dst, (int)*val, dstSize);
}
static AG_Size
PrintU16(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Uint16 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyUint(dst, (Uint)*val, dstSize);
}
static AG_Size
PrintS16(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Sint16 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyInt(dst, (int)*val, dstSize);
}
static AG_Size
PrintU32(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Uint32 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyUint(dst, (Uint)*val, dstSize);
}
static AG_Size
PrintS32(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Sint32 *val = AG_FMTSTRING_ARG(fs);
	return StrlcpyInt(dst, (int)*val, dstSize);
}

#ifdef HAVE_64BIT
static AG_Size
PrintU64(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Uint64 *val = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "%llu", (unsigned long long)*val);
}
static AG_Size
PrintS64(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	Sint64 *val = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "%lld", (long long)*val);
}
#endif /* HAVE_64BIT */

static AG_Size
PrintObjName(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	AG_Object **ob = AG_FMTSTRING_ARG(fs);
	return Strlcpy(dst, (*ob != NULL) ? (*ob)->name : "NULL", dstSize);
}
static AG_Size
PrintObjType(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	AG_Object **ob = AG_FMTSTRING_ARG(fs);
	return Strlcpy(dst, (*ob != NULL) ? (*ob)->cls->name : "NULL", dstSize);
}
static AG_Size
PrintObjClassName(AG_FmtString *_Nonnull fs, char *_Nonnull dst, AG_Size dstSize)
{
	AG_ObjectClass **cls = AG_FMTSTRING_ARG(fs);
	return Strlcpy(dst, (*cls != NULL) ? (*cls)->name : "NULL", dstSize);
}

/* Register a new extended format specifier. */
void
AG_RegisterFmtStringExt(const char *fmt, AG_FmtStringExtFn fn)
{
	AG_FmtStringExt *fs;

	AG_MutexLock(&agFmtExtensionsLock);
	agFmtExtensions = Realloc(agFmtExtensions,
	    (agFmtExtensionCount+1)*sizeof(AG_FmtStringExt));
	fs = &agFmtExtensions[agFmtExtensionCount++];
	fs->fmt = Strdup(fmt);
	fs->fmtLen = strlen(fmt);
	fs->fn = fn;
	AG_MutexUnlock(&agFmtExtensionsLock);
}

/* Unregister an extended format specifier. */
void
AG_UnregisterFmtStringExt(const char *fmt)
{
	Uint i;

	AG_MutexLock(&agFmtExtensionsLock);
	for (i = 0; i < agFmtExtensionCount; i++) {
		if (strcmp(agFmtExtensions[i].fmt, fmt) == 0)
			break;
	}
	if (i < agFmtExtensionCount) {
		free(agFmtExtensions[i].fmt);
		if (i < agFmtExtensionCount-1) {
			memmove(&agFmtExtensions[i], &agFmtExtensions[i+1],
			    (agFmtExtensionCount-i-1)*sizeof(AG_FmtStringExt));
		}
		agFmtExtensionCount--;
	}
	AG_MutexUnlock(&agFmtExtensionsLock);
}

/* Release all resources allocated by a format string. */
void
AG_FreeFmtString(AG_FmtString *fs)
{
	Free(fs->s);
	free(fs);
}

/*
 * Implementation of AG_ProcessFmtString().
 */

#undef FSARG
#define FSARG(fs,_type) (*(_type *)(fs)->p[fs->curArg++])

#ifdef HAVE_64BIT
static AG_Size
ProcessFmtString64(AG_FmtString *_Nonnull fs, const char *_Nonnull f,
    char *_Nonnull dst, AG_Size dstSize)
{
	switch (*f) {
	case 'd':
	case 'i':
		return Snprintf(dst, dstSize, "%lld", (long long)FSARG(fs,Sint64));
	case 'o':
		return Snprintf(dst, dstSize, "%llo", (unsigned long long)FSARG(fs,Uint64));
	case 'u':
		return Snprintf(dst, dstSize, "%llu", (unsigned long long)FSARG(fs,Uint64));
	case 'x':
		return Snprintf(dst, dstSize, "%llx", (unsigned long long)FSARG(fs,Uint64));
	case 'X':
		return Snprintf(dst, dstSize, "%llX", (unsigned long long)FSARG(fs,Uint64));
	}
	return (0);
}
#endif /* HAVE_64BIT */

/*
 * Construct a string from the given AG_FmtString. The arguments are
 * dereferenced, and the resulting string is written to a fixed-size
 * buffer dst of dstSize bytes. Returns the number of characters that
 * would have been copied were dstSize unlimited.
 */
AG_Size
AG_ProcessFmtString(AG_FmtString *fs, char *dst, AG_Size dstSize)
{
	char *pDst = &dst[0];
	char *pEnd = &dst[dstSize-1];
	char *f;
	AG_Size rv;
	Uint i;

	fs->curArg = 0;

	if (dstSize < 1) {
		return (0);
	}
	*pDst = '\0';

	for (f = &fs->s[0]; *f != '\0'; f++) {
		if (f[0] != '%' || f[1] == '\0') {
			*pDst = *f;
			if (++pDst >= pEnd) {
				*pEnd = '\0';			/* Truncate */
				goto out;
			}
			continue;
		}
		rv = 0;
		switch (f[1]) {
		case '[':
			for (i = 0; i < agFmtExtensionCount; i++) {
				AG_FmtStringExt *fExt = &agFmtExtensions[i];

				if (strncmp(fExt->fmt, &f[2], fExt->fmtLen) != 0) {
					continue;
				}
				rv = fExt->fn(fs, pDst, (pEnd-pDst));
				f += fExt->fmtLen + 1;	/* Closing "]" */
				break;
			}
			break;
#if defined(HAVE_FLOAT) || defined(HAVE_64BIT)
		case 'l':
			switch (f[2]) {
# ifdef HAVE_FLOAT
			case 'f':
				rv = Snprintf(pDst, (pEnd-pDst), "%.2f", FSARG(fs,double));
				f++;
				break;
			case 'g':
				rv = Snprintf(pDst, (pEnd-pDst), "%g", FSARG(fs,double));
				f++;
				break;
# endif
# ifdef HAVE_64BIT
			case 'l':
				rv = ProcessFmtString64(fs, &f[3], pDst, (pEnd-pDst));
				f+=2;
				break;
# endif
			}
			break;
#endif /* HAVE_FLOAT or HAVE_64BIT */
		case 'd':
		case 'i':
			rv = StrlcpyInt(pDst, FSARG(fs,int), (pEnd-pDst));
			break;
		case 'u':
			rv = StrlcpyUint(pDst, FSARG(fs,Uint), (pEnd-pDst));
			break;
#ifdef HAVE_FLOAT
		case 'f':
			rv = Snprintf(pDst, (pEnd-pDst), "%.2f", FSARG(fs,float));
			break;
		case 'g':
			rv = Snprintf(pDst, (pEnd-pDst), "%g", FSARG(fs,float));
			break;
#endif
		case 's':
			rv = Strlcpy(pDst, &FSARG(fs,char), (pEnd-pDst));
			break;
		case 'o':
			rv = Snprintf(pDst, (pEnd-pDst), "%o", FSARG(fs,Uint));
			break;
		case 'x':
			rv = Snprintf(pDst, (pEnd-pDst), "%x", FSARG(fs,Uint));
			break;
		case 'X':
			rv = Snprintf(pDst, (pEnd-pDst), "%X", FSARG(fs,Uint));
			break;
		case 'p':
			rv = Snprintf(pDst, (pEnd-pDst), "%p", FSARG(fs,void *));
			break;
		case 'c':
			*pDst = FSARG(fs,char);
			rv = 1;
			break;
		case '%':
			*pDst = '%';
			rv = 1;
			break;
		}
		if ((pDst += rv) > pEnd) {
			*pEnd = '\0';				/* Truncate */
			goto out;
		}
		f++;
	}
out:
	if (pDst < pEnd) {
		*pDst = '\0';
	} else {
		*pEnd = '\0';
	}
	return (pDst - dst);
}

#undef FSARG

#undef CAT_SPEC
#define CAT_SPEC(c) \
	if ((pSpec+2) >= pSpecEnd) { AG_FatalError("Format"); } \
	pSpec[0] = (c); \
	pSpec[1] = '\0'; \
	pSpec++;

/*
 * Build a C string from a format string (see AG_String(3) for details on
 * the format). AG_DoPrintf() writes the formatted output to a fixed-size
 * buffer (if the buffer is too small, the string is truncated and the
 * function returns the number of characters that would have been copied
 * were dstSize unlimited).
 */
static AG_Size
AG_DoPrintf(char *_Nonnull dst, AG_Size dstSize, const char *_Nonnull fmt,
    va_list ap)
{
	char spec[32], *pSpec, *pSpecEnd = &spec[32];
	AG_FmtString fs;
	char *pDst = &dst[0];
	char *pEnd = &dst[dstSize-1];
	const char *f;
	AG_Size rv;
	Uint i;

	if (dstSize < 1) {
		return (1);
	}
	*pDst = '\0';

	fs.s = NULL;

	for (f = &fmt[0]; *f != '\0'; f++) {
		if (f[0] != '%' || f[1] == '\0') {
			*pDst = *f;
			if (++pDst >= pEnd) {
				*pEnd = '\0';			/* Truncate */
				goto out;
			}
			continue;
		}
		spec[0] = '%';
		spec[1] = f[1];
		spec[2] = '\0';
		pSpec = &spec[1];
next_char:
		rv = 0;
		switch (f[1]) {
		case '[':
			for (i = 0; i < agFmtExtensionCount; i++) {
				AG_FmtStringExt *fExt = &agFmtExtensions[i];

				if (strncmp(fExt->fmt, &f[2], fExt->fmtLen) != 0) {
					continue;
				}
				fs.curArg = 0;
				fs.p[0] = va_arg(ap, void *);
				rv = fExt->fn(&fs, pDst, (pEnd-pDst));
				f += fExt->fmtLen + 1;	/* Closing "]" */
				break;
			}
			break;
		case 'd':
		case 'i':
			CAT_SPEC(f[1]);
			if (pSpec == &spec[2]) {	/* Optimized (%d) */
				rv = StrlcpyInt(pDst, va_arg(ap,int), (pEnd-pDst));
			} else {
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,Uint));
			}
			break;
		case 'u':
			CAT_SPEC(f[1]);
			if (pSpec == &spec[2]) {	/* Optimized (%u) */
				rv = StrlcpyUint(pDst, va_arg(ap,Uint), (pEnd-pDst));
			} else {
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,Uint));
			}
			break;
		case 'o':
		case 'x':
		case 'X':
			CAT_SPEC(f[1]);
			rv = Snprintf(pDst, (pEnd-pDst), spec, va_arg(ap,Uint));
			break;
		case 'c':
			CAT_SPEC(f[1]);
			if (pSpec == &spec[2]) {	/* Optimized (%c) */
				if ((pDst+1) > pEnd) {
					*pEnd = '\0';		/* Truncate */
					goto out;
				}
				*pDst = (char)va_arg(ap,int);
				rv = 1;
			} else {
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    (char)va_arg(ap,int));
			}
			break;
#ifdef HAVE_FLOAT
		case 'f':
		case 'g':
			CAT_SPEC(f[1]);
			rv = Snprintf(pDst, (pEnd-pDst), spec, va_arg(ap,double));
			break;
#endif
		case 's':
			CAT_SPEC(f[1]);
			if (pSpec == &spec[2]) {	/* Optimized (%s) */
				rv = Strlcpy(pDst, va_arg(ap,char *), (pEnd-pDst));
			} else {
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,char *));
			}
			break;
		case 'l':
			CAT_SPEC(f[1]);
			switch (f[2]) {
			case 'd':
			case 'i':
				CAT_SPEC(f[2]);
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,long));
				break;
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				CAT_SPEC(f[2]);
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,Ulong));
				break;
#ifdef HAVE_FLOAT
			case 'f':
			case 'g':
				CAT_SPEC(f[2]);
				rv = Snprintf(pDst, (pEnd-pDst), spec,
				    va_arg(ap,double));
				break;
#endif
#ifdef HAVE_64BIT
			case 'l':
				CAT_SPEC(f[2]);
				switch (f[3]) {
				case 'd':
				case 'i':
					CAT_SPEC(f[3]);
					rv = Snprintf(pDst, (pEnd-pDst), spec,
					    (long long)va_arg(ap,Sint64));
					break;
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					CAT_SPEC(f[3]);
					rv = Snprintf(pDst, (pEnd-pDst), spec,
					    (unsigned long long)va_arg(ap,Uint64));
					break;
				}
				f++;
#endif /* HAVE_64BIT */
			}
			f++;
			break;
		case '#':
		case '0':
		case '-':
		case ' ':
		case '+':
		case '\'':
		case '.':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			CAT_SPEC(f[1]);
			f++;
			goto next_char;
		case '%':
			if ((pDst+1) > pEnd) {
				*pEnd = '\0';			/* Truncate */
				goto out;
			}
			*pDst = '%';
			rv = 1;
			break;
		}
		if ((pDst += rv) > pEnd) {
			*pEnd = '\0';				/* Truncate */
			goto out;
		}
		f++;
	}
out:
	if (pDst < pEnd) {
		*pDst = '\0';
	} else {
		*pEnd = '\0';
	}
	return (pDst - dst);
}

#undef CAT_SPEC

/*
 * AG_Printf() performs formatted output conversion and returns a pointer
 * to an internally-managed buffer (the caller must never free() this buffer).
 * The AG_PrintfN() variant allows a buffer index to be specified.
 *
 * In multi-threaded mode, the AG_Printf() buffers are allocated as
 * thread-local storage. They will remain valid until the application
 * or thread exits (or a subsequent AG_Printf() call is made, which
 * will overwrite it).
 */
char *
AG_Printf(const char *fmt, ...)
{
	va_list ap;
	char *dst, *dstNew;
	AG_Size dstSize = AG_FMTSTRING_BUFFER_INIT, rv;

	dst = Malloc(dstSize);
restart:
	dst[0] = '\0';
	va_start(ap, fmt);
	if ((rv = AG_DoPrintf(dst, dstSize, fmt, ap)) >= dstSize) {
		va_end(ap);
		dstNew = TryRealloc(dst, (rv+AG_FMTSTRING_BUFFER_GROW));
		if (dstNew == NULL) { AG_FatalError(NULL); }
		dst = dstNew;
		dstSize = (rv+AG_FMTSTRING_BUFFER_GROW);
		goto restart;
	}
	va_end(ap);

#ifdef AG_THREADS
	if ((agPrintBuf[0] = (char *)AG_ThreadKeyGet(agPrintBufKey[0])) != NULL) {
		free(agPrintBuf[0]);
	}
	agPrintBuf[0] = dst;
	AG_ThreadKeySet(agPrintBufKey[0], agPrintBuf[0]);
#else
	Free(agPrintBuf[0]);
	agPrintBuf[0] = dst;
#endif
	return (dst);
}
char *
AG_PrintfN(Uint idx, const char *fmt, ...)
{
	va_list ap;
	char *dst, *dstNew;
	AG_Size dstSize = AG_FMTSTRING_BUFFER_INIT, rv;

	dst = Malloc(dstSize);
restart:
	dst[0] = '\0';
	va_start(ap, fmt);
	if ((rv = AG_DoPrintf(dst, dstSize, fmt, ap)) >= dstSize) {
		va_end(ap);
		dstNew = TryRealloc(dst, (rv+AG_FMTSTRING_BUFFER_GROW));
		if (dstNew == NULL) { AG_FatalError(NULL); }
		dst = dstNew;
		dstSize = (rv+AG_FMTSTRING_BUFFER_GROW);
		goto restart;
	}
	va_end(ap);

#ifdef AG_THREADS
	if ((agPrintBuf[idx] = (char *)AG_ThreadKeyGet(agPrintBufKey[idx])) != NULL) {
		free(agPrintBuf[idx]);
	}
	agPrintBuf[idx] = dst;
	AG_ThreadKeySet(agPrintBufKey[idx], agPrintBuf[idx]);
#else
	Free(agPrintBuf[idx]);
	agPrintBuf[idx] = dst;
#endif
	return (dst);
}

/*
 * Allocate and return a new AG_FmtString (which describes a string built
 * from arguments to be later accessed, as the string is printed).
 *
 * Optionally, mutexes may be associated with arguments like so:
 *
 * 	AG_PrintfP("My string: %m<%s>", myMutex, myString);
 *
 * This instructs the formatting engine to acquire myMutex before
 * accessing the contents of myString.
 */
AG_FmtString *_Nonnull
AG_PrintfP(const char *_Nonnull fmt, ...)
{
	const char *p;
	AG_FmtString *fs;
	va_list ap;
	AG_Mutex *mu = NULL;

	if ((fs = TryMalloc(sizeof(AG_FmtString))) == NULL) {
		AG_FatalError(NULL);
	}
	fs->s = Strdup(fmt);
	fs->n = 0;

	va_start(ap, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			continue;
		}
		switch (p[1]) {
		case '%':
		case ' ':
			p++;
			break;
		case 'm':
			mu = (AG_Mutex *)va_arg(ap, void *);
			break;
		case '>':
			mu = NULL;
			break;
		case '\0':
			break;
		default:
			if (fs->n+1 >= AG_STRING_POINTERS_MAX) {
				AG_FatalError("Too many arguments");
			}
			fs->p[fs->n] = va_arg(ap, void *);
			fs->mu[fs->n] = mu;
			fs->n++;
			mu = NULL;
			break;
		}
	}
	va_end(ap);
	return (fs);
}

#endif /* AG_ENABLE_STRING */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
AG_Size
AG_Strlcpy(char *dst, const char *src, AG_Size dsize)
{
	const char *osrc = src;
	AG_Size nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return (src - osrc - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
AG_Size
AG_Strlcat(char *dst, const char *src, AG_Size dsize)
{
	const char *odst = dst;
	const char *osrc = src;
	AG_Size n = dsize;
	AG_Size dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *dst != '\0') {
		dst++;
	}
	dlen = dst - odst;
	n = dsize - dlen;

	if (n-- == 0) {
		return (dlen + strlen(src));
	}
	while (*src != '\0') {
		if (n != 0) {
			*dst++ = *src;
			n--;
		}
		src++;
	}
	*dst = '\0';

	return (dlen + (src - osrc));	/* count does not include NUL */
}

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, AG_Strsep returns NULL.
 */
char *
AG_Strsep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL) {
		return (NULL);
	}
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0) {
					s = NULL;
				} else {
					s[-1] = 0;
				}
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
}

/* Duplicate a string. */
char *
AG_Strdup(const char *s)
{
	AG_Size buflen;
	char *ns;
	
	buflen = strlen(s)+1;
	ns = Malloc(buflen);
	memcpy(ns, s, buflen);
	return (ns);
}

/* Duplicate a string or fail. */
char *
AG_TryStrdup(const char *s)
{
	AG_Size buflen;
	char *ns;
	
	buflen = strlen(s)+1;
	if ((ns = TryMalloc(buflen)) == NULL) {
		return (NULL);
	}
	memcpy(ns, s, buflen);
	return (ns);
}

/*
 * Locate a substring ignoring case.
 */
const char *
AG_Strcasestr(const char *s, const char *find)
{
	char c, sc;
	AG_Size len;

	if ((c = *find++) != 0) {
		c = (char)tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (Strncasecmp(s, find, len) != 0);
		s--;
	}
	return (s);
}

#ifdef AG_UNICODE

# ifdef HAVE_ICONV

static AG_Char *_Nullable
ImportUnicodeICONV(const char *_Nonnull encoding,
    const char *_Nonnull s, AG_Size sLen,
    AG_Size *_Nullable pOutLen,
    AG_Size *_Nullable pOutSize)
{
	AG_Char *ucs, *ucsNew;
	const char *inPtr;
	char *wrPtr;
	AG_Size outSize = (sLen+1)*sizeof(AG_Char);
	iconv_t cd;

	if ((ucs = TryMalloc(outSize)) == NULL) {
		return (NULL);
	}
	if ((cd = iconv_open("UCS-4-INTERNAL", encoding)) == (iconv_t)-1) {
		AG_SetError("iconv_open: %s", strerror(errno));
		goto fail;
	}
	wrPtr = (char *)ucs;

	inPtr = s;
#  ifdef HAVE_ICONV_CONST
	if (iconv(cd, &inPtr, &sLen, &wrPtr, &outSize) == (AG_Size)-1) {
		AG_SetError("iconv: %s", strerror(errno));
		iconv_close(cd);
		goto fail;
	}
#  else
	{
		char *tmpBuf;
		if ((tmpBuf = AG_TryStrdup(inPtr)) == NULL) {
			iconv_close(cd);
			goto fail;
		}
		if (iconv(cd, &tmpBuf, &sLen, &wrPtr, &outSize) == (AG_Size)-1) {
			AG_SetError("iconv: %s", strerror(errno));
			iconv_close(cd);
			free(tmpBuf);
			goto fail;
		}
		free(tmpBuf);
	}
#  endif /* !HAVE_ICONV_CONST */

	iconv_close(cd);

	outSize = (wrPtr - (char *)ucs) / sizeof(AG_Char);
	if (pOutLen != NULL) { *pOutLen = outSize; }
		
	/* Shrink the buffer down to the actual string length. */
	ucsNew = TryRealloc(ucs, (outSize+1)*sizeof(AG_Char));
	if (ucsNew == NULL) {
		goto fail;
	}
	ucs = ucsNew;
	ucs[outSize] = '\0';
	if (pOutSize != NULL) { *pOutSize = (outSize+1)*sizeof(AG_Char); }
	return (ucs);
fail:
	Free(ucs);
	return (NULL);
}

# endif /* HAVE_ICONV */

/*
 * Return an internal UCS-4 buffer from the given string and specified
 * encoding. Optionally returns number of characters converted in
 * pOutLen, and allocated buffer size in pOutSize.
 */
AG_Char *
AG_ImportUnicode(const char *encoding, const char *s, AG_Size *pOutLen,
    AG_Size *pOutSize)
{
	AG_Char *ucs;
	AG_Size i, j;
	AG_Size sLen = strlen(s);
	AG_Size bufLen, utf8len;

	if (strcmp(encoding, "UTF-8") == 0) {
		utf8len = AG_LengthUTF8(s);
		bufLen = (utf8len + 1)*sizeof(AG_Char);
		if ((ucs = TryMalloc(bufLen)) == NULL) {
			return (NULL);
		}
		for (i = 0, j = 0; i < sLen; i++, j++) {
			switch (AG_CharLengthUTF8(s[i])) {
			case 1:
				ucs[j] = (AG_Char)s[i];
				break;
			case 2:
				ucs[j]  = (AG_Char)(s[i]   & 0x1f) << 6;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f);
				break;
			case 3:
				ucs[j]  = (AG_Char)(s[i]   & 0x0f) << 12;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 6;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f);
				break;
			case 4:
				ucs[j]  = (AG_Char)(s[i]   & 0x07) << 18;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 12;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 6;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f);
				break;
			case 5:
				ucs[j]  = (AG_Char)(s[i]   & 0x03) << 24;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 18;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 12;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 6;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f);
				break;
			case 6:
				ucs[j]  = (AG_Char)(s[i]   & 0x01) << 30;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 24;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 18;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 12;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f) << 6;
				ucs[j] |= (AG_Char)(s[++i] & 0x3f);
				break;
			case -1:
				Free(ucs);
				return (NULL);
			}
		}
		ucs[j] = '\0';
		if (pOutLen != NULL) { *pOutLen = j; }
		if (pOutSize != NULL) { *pOutSize = bufLen; }
	} else if (strcmp(encoding, "US-ASCII") == 0) {
		bufLen = (sLen + 1)*sizeof(AG_Char);
		if ((ucs = TryMalloc(bufLen)) == NULL) {
			return (NULL);
		}
		for (i = 0; i < sLen; i++) {
			ucs[i] = ((const unsigned char *)s)[i];
		}
		ucs[i] = '\0';
		if (pOutLen != NULL) { *pOutLen = i; }
		if (pOutSize != NULL) { *pOutSize = bufLen; }
	} else {
# ifdef HAVE_ICONV
		ucs = ImportUnicodeICONV(encoding, s, sLen, pOutLen, pOutSize);
# else
		AG_SetError("No such encoding: \"%s\"", encoding);
		return (NULL);
# endif
	}
	return (ucs);
}

# ifdef HAVE_ICONV

static int
ExportUnicodeICONV(const char *_Nonnull encoding, char *_Nonnull dst,
    const AG_Char *_Nonnull ucs, AG_Size dstSize)
{
	const char *inPtr = (const char *)ucs;
	AG_Size inSize, outSize;
	char *wrPtr;
	iconv_t cd;

	inSize = AG_LengthUCS4(ucs) * sizeof(AG_Char)
	outSize = dstSize;
	wrPtr = dst;

	if ((cd = iconv_open(encoding, "UCS-4-INTERNAL")) == (iconv_t)-1) {
		AG_SetError("iconv_open: %s", strerror(errno));
		return (-1);
	}

#  ifdef HAVE_ICONV_CONST
	if (iconv(cd, &inPtr, &inSize, &wrPtr, &outSize) == (AG_Size)-1) {
		AG_SetError("iconv: %s", strerror(errno));
		iconv_close(cd);
		return (-1);
	}
#  else
	{
		char *tmpBuf;
		if ((tmpBuf = AG_TryStrdup(inPtr)) == NULL) {
			iconv_close(cd);
			return (-1);
		}
		if (iconv(cd, &tmpBuf, &inSize, &wrPtr, &outSize) == (AG_Size)-1) {
			AG_SetError("iconv: %s", strerror(errno));
			iconv_close(cd);
			free(tmpBuf);
			return (-1);
		}
		free(tmpBuf);
	}
#  endif /* !HAVE_ICONV_CONST */

	iconv_close(cd);

	if (outSize >= sizeof(char)) {
		outSize = wrPtr - dst;
		dst[outSize] = '\0';
	} else {
		AG_SetErrorS("iconv: No space for NUL");
		return (-1);
	}
	return (0);
}

# endif /* HAVE_ICONV */

/*
 * Convert an internal UCS-4 string to a fixed-size buffer using the specified
 * encoding. At most dstSize-1 bytes will be copied. The string is always
 * NUL-terminated.
 */
int
AG_ExportUnicode(const char *encoding, char *dst, const AG_Char *ucs,
    AG_Size dstSize)
{
	AG_Size len;

	if (strcmp(encoding, "UTF-8") == 0) {
		for (len = 0; *ucs != '\0' && len < dstSize; ucs++) {
			AG_Char uch = *ucs;
			int chlen, ch1, i;

			if (uch < 0x80) {
				chlen = 1;
				ch1 = 0;
			} else if (uch < 0x800) {	
				chlen = 2;
				ch1 = 0xc0;
			} else if (uch < 0x10000) {
				chlen = 3;
				ch1 = 0xe0;
			} else if (uch < 0x200000) {
				chlen = 4;
				ch1 = 0xf0;
			} else if (uch < 0x4000000) {
				chlen = 5;
				ch1 = 0xf8;
			} else if (uch <= 0x7fffffff) {
				chlen = 6;
				ch1 = 0xfc;
			} else {
				AG_SetErrorS("Bad UTF-8 sequence");
				return (-1);
			}
			if (len+chlen+1 > dstSize) {
				AG_SetErrorS("Out of space");
				return (-1);
			}
			for (i = chlen - 1; i > 0; i--) {
				dst[i] = (uch & 0x3f) | 0x80;
				uch >>= 6;
			}
			dst[0] = uch | ch1;
			dst += chlen;
			len += chlen;
		}
		*dst = '\0';
		return (0);
	} else if (strcmp(encoding, "US-ASCII") == 0) {
		for (len = 0; *ucs != '\0' && len < dstSize; ucs++) {
			if ((*ucs) & ~0x7f) {
				AG_SetErrorS("Non-ASCII character");
				return (-1);
			}
			*dst = (char)*ucs;
			dst++;
			len++;
		}
		*dst = '\0';
		return (0);
	} else {
# ifdef HAVE_ICONV
		return ExportUnicodeICONV(encoding, dst, ucs, dstSize);
# else
		AG_SetError("No such encoding: \"%s\"", encoding);
		return (-1);
# endif
	}
}

#endif /* AG_UNICODE */

/* Reverse the characters of a string. */
void
AG_StrReverse(char *s)
{
	char *p, *q, c;

	if (*s == '\0') {
		return;
	}
	q = s;
	while (*(++q) != '\0')
		;;
	for (p = s; p < --q; p++) {
		c = *p;
		*p = *q;
		*q = c;
	}
}

/*
 * Format an int (base 10) into a fixed-size buffer.
 * XXX on truncation, this returns the size of the buffer + 1 byte
 * (we should return the total length required instead).
 */
AG_Size
AG_StrlcpyInt(char *s, int val, AG_Size size)
{
	static const char *digits = "0123456789";
	int sign = (val < 0);
	int i = 0;

	if (size < 1) {
		return (1);
	}
	val = (val < 0) ? -val : val;
	do {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (sign) {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = '-';
	}
	if (i+1 > size) {
		goto trunc;
	}
	s[i] = '\0';
	AG_StrReverse(s);
	return (i);
trunc:
	s[size-1] = '\0';
	AG_StrReverse(s);
	return (i+1);
}

/*
 * Format an unsigned int (base 10) into a fixed-size buffer.
 * XXX on truncation, this returns the size of the buffer + 1 byte
 * (we should return the total length required instead).
 */
AG_Size
AG_StrlcpyUint(char *s, Uint val, AG_Size size)
{
	static const char *digits = "0123456789";
	int i = 0;

	if (size < 1) {
		return (1);
	}
	do {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (i+1 > size) {
		goto trunc;
	}
	s[i] = '\0';
	AG_StrReverse(s);
	return (i);
trunc:
	s[size-1] = '\0';
	AG_StrReverse(s);
	return (i+1);
}

/*
 * Format an int (base 10) into a fixed-size buffer (concatenate).
 * XXX on truncation, this returns the size of the buffer + 1 byte
 * (we should return the total length required instead).
 */
AG_Size
AG_StrlcatInt(char *s, int val, AG_Size size)
{
	static const char *digits = "0123456789";
	int sign = (val < 0);
	int i = strlen(s);
	int iStart = i;

	if (size < 1) {
		return (1);
	}
	val = (val < 0) ? -val : val;
	do {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (sign) {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = '-';
	}
	if (i+1 > size) {
		goto trunc;
	}
	s[i] = '\0';
	AG_StrReverse(&s[iStart]);
	return (i);
trunc:
	s[size-1] = '\0';
	AG_StrReverse(&s[iStart]);
	return (i+1);
}

/*
 * Format an unsigned int (base 10) into a fixed-size buffer (concatenate).
 * XXX on truncation, this returns the size of the buffer + 1 byte
 * (we should return the total length required instead).
 */
AG_Size
AG_StrlcatUint(char *s, Uint val, AG_Size size)
{
	static const char *digits = "0123456789";
	int i = strlen(s);
	int iStart = i;

	if (size < 1) {
		return (1);
	}
	do {
		if (i+1 > size) {
			goto trunc;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (i+1 > size) {
		goto trunc;
	}
	s[i] = '\0';
	AG_StrReverse(&s[iStart]);
	return (i);
trunc:
	s[size-1] = '\0';
	AG_StrReverse(&s[iStart]);
	return (i+1);
}

#if defined(AG_ENABLE_STRING) && defined(AG_THREADS)
static void
DestroyPrintBuffer(void *_Nullable buf)
{
	Free(buf);
}
#endif

int
AG_InitStringSubsystem(void)
{
#ifdef AG_ENABLE_STRING
	Uint i;

	/* Initialize the AG_Printf() buffers. */
	for (i = 0; i < AG_STRING_BUFFERS_MAX; i++) {
		agPrintBuf[i] = NULL;
# ifdef AG_THREADS
		if (AG_ThreadKeyTryCreate(&agPrintBufKey[i], DestroyPrintBuffer) == -1) {
			return (-1);
		}
		AG_ThreadKeySet(agPrintBufKey[i], NULL);
# endif
	}
	/* Initialize the formatting engine extensions. */
	AG_MutexInit(&agFmtExtensionsLock);
	AG_RegisterFmtStringExt("u8", PrintU8);
	AG_RegisterFmtStringExt("s8", PrintS8);
	AG_RegisterFmtStringExt("u16", PrintU16);
	AG_RegisterFmtStringExt("s16", PrintS16);
	AG_RegisterFmtStringExt("u32", PrintU32);
	AG_RegisterFmtStringExt("s32", PrintS32);
# ifdef HAVE_64BIT
	AG_RegisterFmtStringExt("u64", PrintU64);
	AG_RegisterFmtStringExt("s64", PrintS64);
# endif
	AG_RegisterFmtStringExt("objName", PrintObjName);
	AG_RegisterFmtStringExt("objType", PrintObjType);
	AG_RegisterFmtStringExt("objClassName", PrintObjClassName);
#endif /* AG_ENABLE_STRING */

	return (0);
}

void
AG_DestroyStringSubsystem(void)
{
#ifdef AG_ENABLE_STRING
	Uint i;

	/* Free the AG_Printf() buffers. */
	for (i = 0; i < AG_STRING_BUFFERS_MAX; i++) {
# ifdef AG_THREADS
		if ((agPrintBuf[i] = (char *)AG_ThreadKeyGet(agPrintBufKey[i]))
		    != NULL) {
			free(agPrintBuf[i]);
		}
		AG_ThreadKeyDelete(agPrintBufKey[i]);
# else
		Free(agPrintBuf[i]);
# endif
		agPrintBuf[i] = NULL;
	}
	/* Free the formatting engine extensions. */
	for (i = 0; i < agFmtExtensionCount; i++) {
		Free(agFmtExtensions[i].fmt);
	}
	Free(agFmtExtensions);
	agFmtExtensions = NULL;
	agFmtExtensionCount = 0;
	AG_MutexDestroy(&agFmtExtensionsLock);
#endif /* AG_ENABLE_STRING */
}
