/*
 * Copyright (c) 2008-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef	_AGAR_CORE_STRING_H_
#define	_AGAR_CORE_STRING_H_

#define AG_STRING_BUFFERS_MAX	8			/* For AG_Printf() */
#define AG_STRING_POINTERS_MAX	32			/* For AG_Printf */

#include <agar/core/begin.h>

typedef struct ag_fmt_string {
	char *s;				/* Format string */
	void *p[AG_STRING_POINTERS_MAX];	/* Variable references */
	AG_Mutex *mu[AG_STRING_POINTERS_MAX];	/* Protecting variables */
	Uint n;
	int curArg;				/* For internal parser use */
} AG_FmtString;

/* Extended format specifier for polled labels. */
typedef size_t (*AG_FmtStringExtFn)(struct ag_fmt_string *, char *, size_t);
typedef struct ag_fmt_string_ext {
	char *fmt;
	size_t fmtLen;
	AG_FmtStringExtFn fn;
} AG_FmtStringExt;

#define AG_FMTSTRING_ARG(fs) ((fs)->p[fs->curArg++])
#define AG_FMTSTRING_BUFFER_INIT 128
#define AG_FMTSTRING_BUFFER_GROW 128

__BEGIN_DECLS
extern const unsigned char agStrcasecmpMapASCII[];

char         *AG_Printf(const char *, ...);
char         *AG_PrintfN(Uint, const char *, ...);
AG_FmtString *AG_PrintfP(const char *, ...);
void          AG_RegisterFmtStringExt(const char *, AG_FmtStringExtFn);
void          AG_UnregisterFmtStringExt(const char *);
size_t        AG_ProcessFmtString(AG_FmtString *, char *, size_t);
void          AG_FreeFmtString(AG_FmtString *);

char  *AG_Strsep(char **, const char *);
char  *AG_Strdup(const char *);
char  *AG_TryStrdup(const char *);
size_t AG_Strlcpy(char *, const char *, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);
size_t AG_Strlcat(char *, const char *, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);
size_t AG_StrlcpyInt(char *, int, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);
size_t AG_StrlcatInt(char *, int, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);
size_t AG_StrlcpyUint(char *, Uint, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);
size_t AG_StrlcatUint(char *, Uint, size_t) BOUNDED_ATTRIBUTE(__string__,1,3);

const char *AG_Strcasestr(const char *, const char *);
void        AG_StrReverse(char *);

Uint32	*AG_ImportUnicode(const char *, const char *, size_t *, size_t *);
int      AG_ExportUnicode(const char *, char *, const Uint32 *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 4);

int    AG_InitStringSubsystem(void);
void   AG_DestroyStringSubsystem(void);

/*
 * Return the length of a UCS-4 string in characters, without the
 * terminating NUL.
 */
static __inline__ size_t
AG_LengthUCS4(const Uint32 *ucs)
{
	size_t len;

	for (len = 0; *ucs != '\0'; ucs++) {
		len++;
	}
	return (len);
}

/*
 * Return the number of bytes that would be needed to encode the given
 * UCS-4 character in UTF-8.
 */
static __inline__ int
AG_CharLengthUTF8FromUCS4(Uint32 ch)
{
	if      (ch <  0x80)		{ return (1); }
	else if (ch <  0x800)		{ return (2); }
	else if (ch <  0x10000)		{ return (3); }
	else if (ch <  0x200000)	{ return (4); }
	else if (ch <  0x4000000)	{ return (5); }
	else if (ch <= 0x7fffffff)	{ return (6); }

	AG_SetError("Bad UCS-4 character");
	return (-1);
}

/*
 * Return the number of bytes (not including the terminating NUL) that would
 * be needed to encode the given UCS-4 string in UTF-8.
 */
static __inline__ int
AG_LengthUTF8FromUCS4(const Uint32 *ucs4, size_t *rv)
{
	const Uint32 *c;
	int cLen;

	*rv = 0;
	for (c = &ucs4[0]; *c != '\0'; c++) {
		if ((cLen = AG_CharLengthUTF8FromUCS4(*c)) == -1) {
			return (-1);
		}
		(*rv) += cLen;
	}
	return (0);
}

/*
 * Parse the first byte of a possible UTF-8 sequence and return the length
 * of the sequence in bytes (or 1 if there is none).
 */
static __inline__ int
AG_CharLengthUTF8(unsigned char ch)
{
	int rv;

	if ((ch >> 7) == 0) {
		rv = 1;
	} else if (((ch & 0xe0) >> 5) == 0x6) {
		rv = 2;
	} else if (((ch & 0xf0) >> 4) == 0xe) {
		rv = 3;
	} else if (((ch & 0xf8) >> 3) == 0x1e) {
		rv = 4;
	} else if (((ch & 0xfc) >> 2) == 0x3e) {
		rv = 5;
	} else if (((ch & 0xfe) >> 1) == 0x7e) {
		rv = 6;
	} else {
		AG_SetError("Bad UTF-8 sequence");
		return (-1);
	}
	return (rv);
}

/*
 * Return the number of characters in the given UTF-8 string, not counting
 * the terminating NUL. If the string is invalid, fail and return -1.
 */
static __inline__ int
AG_LengthUTF8(const char *s, size_t *rv)
{
	const char *c = &s[0];
	int i, cLen;

	*rv = 0;
	if (s[0] == '\0') {
		return (0);
	}
	for (;;) {
		if ((cLen = AG_CharLengthUTF8((unsigned char)*c)) == -1) {
			return (-1);
		}
		for (i = 0; i < cLen; i++) {
			if (c[i] == '\0')
				return (0);
		}
		(*rv)++;
		c += cLen;
	}
	return (0);
}

/*
 * Compare two strings ignoring case.
 */
static __inline__ int
AG_Strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *cm = agStrcasecmpMapASCII;
	const unsigned char *us1 = (const unsigned char *)s1;
	const unsigned char *us2 = (const unsigned char *)s2;

	while (cm[*us1] == cm[*us2++]) {
		if (*us1++ == '\0')
			return (0);
	}
	return (cm[*us1] - cm[*--us2]);
}

/*
 * Compare the first n-characters of two strings ignoring case.
 */
static __inline__ int
AG_Strncasecmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *cm = agStrcasecmpMapASCII;
	const unsigned char *us1 = (const unsigned char *)s1;
	const unsigned char *us2 = (const unsigned char *)s2;
	size_t i;

	for (i = 0; i < n; i++) {
		if (cm[us1[i]] != cm[us2[i]])
			break;
	}
	return i == n ? 0 : cm[us1[i]] - cm[us2[i]];
}
__END_DECLS

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
#define Strlcat AG_Strlcat
#define Strlcpy AG_Strlcpy
#define Strsep AG_Strsep
#define Strdup AG_Strdup
#define TryStrdup AG_TryStrdup
#define Strcasecmp AG_Strcasecmp
#define Strncasecmp AG_Strncasecmp
#define Strcasestr AG_Strcasestr
#define StrReverse AG_StrReverse
#define StrlcpyInt AG_StrlcpyInt
#define StrlcatInt AG_StrlcatInt
#define StrlcpyUint AG_StrlcpyUint
#define StrlcatUint AG_StrlcatUint
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_STRING_H_ */
