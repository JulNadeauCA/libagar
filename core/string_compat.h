/*
 * Copyright (c) 2008-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#ifndef	_AGAR_CORE_STRING_COMPAT_H_
#define	_AGAR_CORE_STRING_COMPAT_H_

#include <agar/core/begin.h>

enum ag_unicode_conv {
	AG_UNICODE_FROM_USASCII,	/* US-ASCII -> UCS-4 */
	AG_UNICODE_FROM_UTF8,		/* UTF-8 -> UCS-4 */
	AG_UNICODE_TO_UTF8		/* UCS-4 -> UTF-8 */
};

__BEGIN_DECLS
extern const unsigned char agStrcasecmpMapASCII[];

size_t      AG_StrlcpyUCS4(Uint32 *, const Uint32 *, size_t);
size_t      AG_Strlcpy(char *, const char *, size_t)
                       BOUNDED_ATTRIBUTE(__string__, 1, 3);
size_t      AG_StrlcatUCS4(Uint32 *, const Uint32 *, size_t);
size_t      AG_Strlcat(char *, const char *, size_t)
                       BOUNDED_ATTRIBUTE(__string__, 1, 3);
Uint32     *AG_StrsepUCS4(Uint32 **, const Uint32 *);
char       *AG_Strsep(char **, const char *);
char       *AG_Strdup(const char *);
char       *AG_TryStrdup(const char *);
Uint32     *AG_StrdupUCS4(const Uint32 *);
Uint32     *AG_TryStrdupUCS4(const Uint32 *);
const char *AG_Strcasestr(const char *, const char *);
void        AG_StrReverse(char *);
int         AG_StrlcpyInt(char *, int, size_t)
                          BOUNDED_ATTRIBUTE(__string__, 1, 3);
int         AG_StrlcatInt(char *, int, size_t)
                          BOUNDED_ATTRIBUTE(__string__, 1, 3);
int         AG_StrlcpyUint(char *, Uint, size_t)
                           BOUNDED_ATTRIBUTE(__string__, 1, 3);
int         AG_StrlcatUint(char *, Uint, size_t)
                           BOUNDED_ATTRIBUTE(__string__, 1, 3);

Uint32	*AG_ImportUnicode(enum ag_unicode_conv, const char *, size_t);
long     AG_ExportUnicode(enum ag_unicode_conv, char *, const Uint32 *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 4);
size_t	 AG_CopyUnicode(enum ag_unicode_conv, const char *, Uint32 *, size_t);


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

	AG_FatalError("CharLengthUTF8");
	return (-1);
}

/*
 * Return the number of bytes (not including the terminating NUL) that would
 * be needed to encode the given UCS-4 string in UTF-8.
 */
static __inline__ size_t
AG_LengthUTF8FromUCS4(const Uint32 *ucs4)
{
	size_t rv = 0;
	const Uint32 *c;

	for (c = &ucs4[0]; *c != '\0'; c++) {
		rv += AG_CharLengthUTF8FromUCS4(*c);
	}
	return (rv);
}

/*
 * Parse the first byte of a possible UTF-8 sequence and return the length
 * of the sequence in bytes (or 1 if there is none).
 */
static __inline__ int
AG_CharLengthUTF8(unsigned char ch)
{
	if ((ch >> 7) == 0) {
		return (1);
	} else if (((ch & 0xe0) >> 5) == 0x6) {
		return (2);
	} else if (((ch & 0xf0) >> 4) == 0xe) {
		return (3);
	} else if (((ch & 0xf8) >> 3) == 0x1e) {
		return (4);
	} else if (((ch & 0xfc) >> 2) == 0x3e) {
		return (5);
	} else if (((ch & 0xfe) >> 1) == 0x7e) {
		return (6);
	} else {
		return (-1);
	}
}

/*
 * Return the number of characters in the given UTF-8 string, not counting
 * the terminating NUL.
 */
static __inline__ size_t
AG_LengthUTF8(const char *s)
{
	const char *c = &s[0];
	size_t rv = 0;
	int i, cLen;

	if (s[0] == '\0') {
		return (0);
	}
	for (;;) {
		if ((cLen = AG_CharLengthUTF8((unsigned char)*c)) == -1) {
			break;
		}
		for (i = 0; i < cLen; i++) {
			if (c[i] == '\0')
				return (rv);
		}
		rv++;
		c += cLen;
	}
	return (rv);
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
#define StrlcatUCS4 AG_StrlcatUCS4
#define StrlcpyUCS4 AG_StrlcpyUCS4
#define StrsepUCS4 AG_StrsepUCS4
#define StrdupUCS4 AG_StrdupUCS4
#define TryStrdupUCS4 AG_TryStrdupUCS4
#define StrReverse AG_StrReverse
#define StrlcpyInt AG_StrlcpyInt
#define StrlcatInt AG_StrlcatInt
#define StrlcpyUint AG_StrlcpyUint
#define StrlcatUint AG_StrlcatUint
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_STRING_COMPAT_H_ */
