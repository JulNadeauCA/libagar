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
	char *_Nonnull  s;			       /* Format string */
	void *_Nullable p[AG_STRING_POINTERS_MAX];     /* Variable references */
	_Nullable AG_Mutex *_Nullable mu[AG_STRING_POINTERS_MAX];
	Uint n;
	int curArg;				/* For internal parser use */
} AG_FmtString;

/* Extended format specifier for polled labels. */
typedef AG_Size (*AG_FmtStringExtFn)(struct ag_fmt_string *_Nonnull,
                                     char *_Nonnull, AG_Size);
typedef struct ag_fmt_string_ext {
	char *_Nonnull fmt;		/* Format specifier string */
	AG_Size        fmtLen;		/* Specifier length */
	_Nonnull AG_FmtStringExtFn fn;	/* Callback function */
} AG_FmtStringExt;

#define AG_FMTSTRING_ARG(fs) ((fs)->p[fs->curArg++])
#define AG_FMTSTRING_BUFFER_INIT 128
#define AG_FMTSTRING_BUFFER_GROW 128

__BEGIN_DECLS
extern const unsigned char agStrcasecmpMapASCII[];

char *_Nonnull         AG_Printf(const char *_Nonnull, ...);
char *_Nonnull         AG_PrintfN(Uint, const char *_Nonnull, ...);
AG_FmtString *_Nonnull AG_PrintfP(const char *_Nonnull, ...);

void    AG_RegisterFmtStringExt(const char *_Nonnull, _Nonnull AG_FmtStringExtFn);
void    AG_UnregisterFmtStringExt(const char *_Nonnull);
AG_Size AG_ProcessFmtString(AG_FmtString *_Nonnull, char *_Nonnull, AG_Size);
void    AG_FreeFmtString(AG_FmtString *_Nonnull);

char *_Nullable AG_Strsep(char *_Nonnull *_Nullable, const char *_Nonnull);
char *_Nonnull  AG_Strdup(const char *_Nonnull);
char *_Nullable AG_TryStrdup(const char *_Nonnull);

AG_Size AG_Strlcpy(char *_Nonnull _Restrict, const char *_Nonnull _Restrict,
                   AG_Size);
AG_Size AG_Strlcat(char *_Nonnull _Restrict, const char *_Nonnull _Restrict,
                   AG_Size);
AG_Size AG_StrlcpyInt(char *_Nonnull, int, AG_Size);
AG_Size AG_StrlcatInt(char *_Nonnull, int, AG_Size);
AG_Size AG_StrlcpyUint(char *_Nonnull, Uint, AG_Size);
AG_Size AG_StrlcatUint(char *_Nonnull, Uint, AG_Size);

const char *_Nullable AG_Strcasestr(const char *_Nonnull, const char *_Nonnull)
                                   _Pure_Attribute;

void AG_StrReverse(char *_Nonnull);

Uint32 *_Nullable AG_ImportUnicode(const char *_Nonnull, const char *_Nonnull,
                                   AG_Size *_Nullable, AG_Size *_Nullable);
int               AG_ExportUnicode(const char *_Nonnull, char *_Nonnull,
                                   const Uint32 *_Nonnull, AG_Size);

int  AG_InitStringSubsystem(void);
void AG_DestroyStringSubsystem(void);

#ifdef AG_INLINE_STRING
# include <agar/core/inline_string.h>
#else
AG_Size _Pure_Attribute ag_length_ucs4(const Uint32 *_Nonnull);
int _Const_Attribute    ag_char_length_utf8_from_ucs4(Uint32);
int                     ag_length_utf8_from_ucs4(const Uint32 *_Nonnull,
                                                 AG_Size *_Nonnull);
int _Const_Attribute    ag_char_length_utf8(unsigned char);
AG_Size _Pure_Attribute ag_length_utf8(const char *_Nonnull);
int _Pure_Attribute     ag_strcasecmp(const char *_Nonnull, const char *_Nonnull);
int _Pure_Attribute     ag_strncasecmp(const char *_Nonnull, const char *_Nonnull,
                                       AG_Size);

# define AG_LengthUCS4(s)             ag_length_ucs4(s)
# define AG_CharLengthUTF8FromUCS4(c) ag_char_length_utf8_from_ucs4(c)
# define AG_LengthUTF8FromUCS4(s,len) ag_length_utf8_from_ucs4((s),(len))
# define AG_CharLengthUTF8(c)         ag_char_length_utf8(c)
# define AG_LengthUTF8(s)             ag_length_utf8(s)
# define AG_Strcasecmp(a,b)           ag_strcasecmp((a),(b))
# define AG_Strncasecmp(a,b,n)        ag_strncasecmp((a),(b),(n))
#endif /* AG_INLINE_STRING */

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
