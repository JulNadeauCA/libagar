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

#include <agar/core/begin.h>

#ifdef AG_ENABLE_STRING
# ifndef AG_STRING_BUFFERS_MAX
# define AG_STRING_BUFFERS_MAX	8			/* For AG_Printf() */
# endif
# ifndef AG_STRING_POINTERS_MAX
# define AG_STRING_POINTERS_MAX	32			/* For AG_Printf */
# endif

typedef struct ag_fmt_string {
	char *_Nonnull  s;			       /* Format string */
	void *_Nullable p[AG_STRING_POINTERS_MAX];     /* Variable references */
	_Nullable_Mutex AG_Mutex *_Nullable mu[AG_STRING_POINTERS_MAX];
	Uint n;
	int curArg;				/* For internal parser use */
} AG_FmtString;

/* Extended format specifier for polled labels. */
typedef AG_Size (*AG_FmtStringExtFn)(struct ag_fmt_string *_Nonnull,
                                     char *_Nonnull, AG_Size);
typedef struct ag_fmt_string_ext {
	char *_Nonnull fmt;		/* Format specifier string */
	AG_Size        fmtLen;		/* Specifier length */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
	_Nonnull AG_FmtStringExtFn fn;	/* Callback function */
} AG_FmtStringExt;

# define AG_FMTSTRING_ARG(fs) ((fs)->p[fs->curArg++])
# define AG_FMTSTRING_BUFFER_INIT 128
# define AG_FMTSTRING_BUFFER_GROW 128
#endif /* AG_ENABLE_STRING */

/* Possible newline value in a given character encoding */
typedef struct ag_newline_format {
	const char *encoding;		/* Character encoding */
	const char *abbr;		/* Abbreviation */
	const char *s;			/* The sequence */
	int len;			/* Length of sequence (1..2) */
	Uint32 _pad;
} AG_NewlineFormat;

enum ag_newline_type {
	AG_NEWLINE_LF,			/* Unix, Amiga, BeOS, Multics, RISC OS */
	AG_NEWLINE_CR_LF,		/* DOS/Windows, early non-Unix */
	AG_NEWLINE_CR,			/* Commodore 8-bit machines (C64/128) */
#if 0
	AG_NEWLINE_LF_CR,		/* Acorn BBC and RISC OS */
	AG_NEWLINE_ATA_CR,		/* Atari 8-bit machines */
	AG_NEWLINE_EBCDIC,		/* IBM mainframes */
#endif
	AG_NEWLINE_LAST
};
#define AG_NEWLINE_DOS  AG_NEWLINE_CR_LF
#define AG_NEWLINE_UNIX AG_NEWLINE_LF

#if defined(__MSDOS__) || defined(__WIN32__) || defined(__BEOS__) || defined(__RISCOS__)
# define AG_NEWLINE_NATIVE AG_NEWLINE_CR_LF
#else
# define AG_NEWLINE_NATIVE AG_NEWLINE_LF
#endif

__BEGIN_DECLS
extern const unsigned char agStrcasecmpMapASCII[];
extern const AG_NewlineFormat agNewlineFormats[];

#ifdef AG_ENABLE_STRING
char *_Nonnull         AG_Printf(const char *_Nonnull, ...);
char *_Nonnull         AG_PrintfN(Uint, const char *_Nonnull, ...);
AG_FmtString *_Nonnull AG_PrintfP(const char *_Nonnull, ...);

void    AG_RegisterFmtStringExt(const char *_Nonnull, _Nonnull AG_FmtStringExtFn);
void    AG_UnregisterFmtStringExt(const char *_Nonnull);
AG_Size AG_ProcessFmtString(AG_FmtString *_Nonnull, char *_Nonnull, AG_Size);
void    AG_FreeFmtString(AG_FmtString *_Nonnull);
#endif /* AG_ENABLE_STRING */

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

#ifdef AG_UNICODE
AG_Char *_Nullable AG_ImportUnicode(const char *_Nonnull, const char *_Nonnull,
                                    AG_Size *_Nullable, AG_Size *_Nullable);
int                AG_ExportUnicode(const char *_Nonnull, char *_Nonnull,
                                    const AG_Char *_Nonnull, AG_Size);
#endif

int  AG_InitStringSubsystem(void);
void AG_DestroyStringSubsystem(void);

/*
 * Inlinables
 */
#ifdef AG_UNICODE
AG_Size _Pure_Attribute ag_length_ucs4(const AG_Char *_Nonnull);
int _Const_Attribute    ag_char_length_utf8_from_ucs4(AG_Char);
int                     ag_length_utf8_from_ucs4(const AG_Char *_Nonnull,
                                                 AG_Size *_Nonnull);
int _Const_Attribute    ag_char_length_utf8(unsigned char);
AG_Size _Pure_Attribute ag_length_utf8(const char *_Nonnull);
#endif /* AG_UNICODE */

int _Pure_Attribute     ag_strcasecmp(const char *_Nonnull, const char *_Nonnull);
int _Pure_Attribute     ag_strncasecmp(const char *_Nonnull, const char *_Nonnull,
                                       AG_Size);
#ifdef AG_INLINE_STRING
# define AG_INLINE_HEADER
# include <agar/core/inline_string.h>
#else
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
# define Strlcat(dst,src,dsize)  AG_Strlcat((dst),(src),(dsize))
# define Strlcpy(dst,src,dsize)  AG_Strlcpy((dst),(src),(dsize))
# define Strsep(s,delim)         AG_Strsep((s),(delim))
# define Strdup(s)               AG_Strdup(s)
# define TryStrdup(s)            AG_TryStrdup(s)
# define Strcasecmp(s1,s2)       AG_Strcasecmp((s1),(s2))
# define Strncasecmp(s1,s2,n)    AG_Strncasecmp((s1),(s2),(n))
# define Strcasestr(s,find)      AG_Strcasestr((s),(find))
# define StrReverse(s)           AG_StrReverse(s)
# define StrlcpyInt(s,val,size)  AG_StrlcpyInt((s),(val),(size))
# define StrlcatInt(s,val,size)  AG_StrlcatInt((s),(val),(size))
# define StrlcpyUint(s,val,size) AG_StrlcpyUint((s),(val),(size))
# define StrlcatUint(s,val,size) AG_StrlcatUint((s),(val),(size))
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_STRING_H_ */
