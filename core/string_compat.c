/*
 * Copyright (c) 2003-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TODD C. MILLER DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL TODD C. MILLER BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <core/core.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ASCII character sequences.
 */
const unsigned char agStrcasecmpMapASCII[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
AG_Strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0) {
				break;
			}
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return (s - src - 1);	/* count does not include NUL */
}

/* UCS-4 version of Strlcpy() */
size_t
AG_StrlcpyUCS4(Uint32 *dst, const Uint32 *src, size_t bytes)
{
	Uint32 *d = dst;
	const Uint32 *s = src;
	size_t n = bytes / sizeof(Uint32);

	/* Copy as many characters as will fit. */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0) {
				break;
			}
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (n == 0) {
		if (bytes != 0) {
			*d = '\0';			 /* NUL-terminate dst */
		}
		while (*s++)
			;
	}

	return ((s - src - 1) * sizeof(Uint32));      /* Does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
AG_Strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t dlen, n = siz;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0') {
		d++;
	}
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0) {
		return (dlen + strlen(s));
	}
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return (dlen + (s - src));	/* count does not include NUL */
}

/* UCS-4 version of Strlcat() */
size_t
AG_StrlcatUCS4(Uint32 *dst, const Uint32 *src, size_t bytes)
{
	Uint32 *d = dst;
	const Uint32 *s = src;
	size_t siz = bytes / sizeof(Uint32);
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *d != '\0') {
		d++;
	}
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0) {
		return ((dlen + AG_LengthUCS4(s))*sizeof(Uint32));
	}
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return ((dlen + (s - src))*sizeof(Uint32));   /* Does not include NUL */
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

/* UCS-4 version of Strsep() */
Uint32 *
AG_StrsepUCS4(Uint32 **stringp, const Uint32 *delim)
{
	Uint32 *s;
	const Uint32 *spanp;
	Uint32 c, sc;
	Uint32 *tok;

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
	size_t buflen;
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
	size_t buflen;
	char *ns;
	
	buflen = strlen(s)+1;
	if ((ns = TryMalloc(buflen)) == NULL) {
		return (NULL);
	}
	memcpy(ns, s, buflen);
	return (ns);
}

/* Duplicate a UCS-4 string. */
Uint32 *
AG_StrdupUCS4(const Uint32 *ucs)
{
	size_t buflen;
	Uint32 *ns;
	
	buflen = (AG_LengthUCS4(ucs) + 1)*sizeof(Uint32);
	ns = Malloc(buflen);
	memcpy(ns, ucs, buflen);
	return (ns);
}

/* Duplicate a UCS-4 string or fail. */
Uint32 *
AG_TryStrdupUCS4(const Uint32 *ucs)
{
	size_t buflen;
	Uint32 *ns;
	
	buflen = (AG_LengthUCS4(ucs) + 1)*sizeof(Uint32);
	if ((ns = TryMalloc(buflen)) == NULL) {
		return (NULL);
	}
	memcpy(ns, ucs, buflen);
	return (ns);
}

/*
 * Locate a substring ignoring case.
 */
const char *
AG_Strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

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

/*
 * Returns a buffer containing a UCS-4 representation of the given
 * string/encoding. If len is 0, enough memory to hold the string is
 * allocated. Otherwise, a buffer of the specified size is allocated.
 */
Uint32 *
AG_ImportUnicode(enum ag_unicode_conv conv, const char *s, size_t pLen)
{
	Uint32 *ucs;
	size_t i, j;
	size_t sLen = strlen(s);
	size_t bufLen = (pLen != 0) ? pLen : (sLen+1);

	ucs = Malloc(bufLen*sizeof(Uint32));

	switch (conv) {
	case AG_UNICODE_FROM_USASCII:
		for (i = 0; i < sLen; i++) {
			ucs[i] = ((const unsigned char *)s)[i];
		}
		ucs[i] = '\0';
		break;
	case AG_UNICODE_FROM_UTF8:
		for (i = 0, j = 0; i < sLen; i++, j++) {
			switch (AG_CharLengthUTF8(s[i])) {
			case 1:
				ucs[j] = (Uint32)s[i];
				break;
			case 2:
				ucs[j]  = (Uint32)(s[i]   & 0x1f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 3:
				ucs[j]  = (Uint32)(s[i]   & 0x0f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 4:
				ucs[j]  = (Uint32)(s[i]   & 0x07) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 5:
				ucs[j]  = (Uint32)(s[i]   & 0x03) << 24;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 6:
				ucs[j]  = (Uint32)(s[i]   & 0x01) << 30;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 24;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case -1:
				ucs[j] = '?';
				break;
			}
		}
		ucs[j] = '\0';
		break;
	default:
		break;
	}
	return (ucs);
}

size_t
AG_CopyUnicode(enum ag_unicode_conv conv, const char *s, Uint32 *ucs,
    size_t ucs_len)
{
	size_t len;
	size_t i, j;

	len = strlen(s);

	switch (conv) {
	case AG_UNICODE_FROM_USASCII:
		if (len > ucs_len) {
			len = ucs_len;
		}
		for (i = 0; i < len; i++) {
			ucs[i] = ((const unsigned char *)s)[i];
		}
		ucs[i] = '\0';
		return (i);
	case AG_UNICODE_FROM_UTF8:
		for (i = 0, j = 0; i < len; i++, j++) {
			switch (AG_CharLengthUTF8(s[i])) {
			case 1:
				if (i+1 >= ucs_len) {
					break;
				}
				ucs[j] = (Uint32)s[i];
				break;
			case 2:
				if (i+2 >= ucs_len) {
					break;
				}
				ucs[j]  = (Uint32)(s[i]   & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 3:
				if (i+3 >= ucs_len) {
					break;
				}
				ucs[j]  = (Uint32)(s[i]   & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 4:
				if (i+4 >= ucs_len) {
					break;
				}
				ucs[j]  = (Uint32)(s[i]   & 0x07) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 5:
				if (i+5 >= ucs_len) {
					break;
				}
				ucs[j]  = (Uint32)(s[i]   & 0x03) << 24;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 6:
				if (i+6 >= ucs_len) {
					break;
				}
				ucs[j]  = (Uint32)(s[i]   & 0x01) << 30;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 24;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 18;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 12;
				ucs[j] |= (Uint32)(s[++i] & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case -1:
				if (i+1 >= ucs_len) {
					break;
				}
				ucs[j] = '?';
				break;
			}
		}
		ucs[j] = '\0';
		return (j);
	default:
		break;
	}
	return (0);
}

/*
 * Convert a UCS-4 string to the given encoding.
 * At most dst_size-1 bytes will be copied. The string is NUL-terminated
 * unless dst_size == 0.
 *
 * If retval >= dst_size, truncation occurred. If retval == -1, a
 * conversion error has occurred.
 */
long
AG_ExportUnicode(enum ag_unicode_conv conv, char *dst, const Uint32 *ucs,
    size_t dst_size)
{
	size_t len;

	switch (conv) {
	case AG_UNICODE_TO_UTF8:
		for (len = 0; *ucs != '\0' && len < dst_size; ucs++) {
			Uint32 uch = *ucs;
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
				return (-1);
			}
			if (len+chlen+1 >= dst_size) {
				return ((long)len+chlen);
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
		return (long)len;
	default:
		return (-1);
	}
}

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
 */
int
AG_StrlcpyInt(char *s, int val, size_t len)
{
	static const char *digits = "0123456789";
	int sign = (val < 0) ? -val : val;
	int i = 0;

	if (len < 1)
		return (-1);

	do {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (sign < 0) {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = '-';
	}
	if (i+1 > len) {
		goto fail;
	}
	s[i] = '\0';
	AG_StrReverse(s);
	return (0);
fail:
	s[0] = '\0';
	return (-1);
}

/*
 * Format an unsigned int (base 10) into a fixed-size buffer.
 */
int
AG_StrlcpyUint(char *s, Uint val, size_t len)
{
	static const char *digits = "0123456789";
	int i = 0;

	if (len < 1)
		return (-1);

	do {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (i+1 > len) {
		goto fail;
	}
	s[i] = '\0';
	AG_StrReverse(s);
	return (0);
fail:
	s[0] = '\0';
	return (-1);
}

/*
 * Format an int (base 10) into a fixed-size buffer (concatenate).
 */
int
AG_StrlcatInt(char *s, int val, size_t len)
{
	static const char *digits = "0123456789";
	int sign = (val < 0) ? -val : val;
	int i = strlen(s);
	int iStart = i;

	if (len < 1)
		return (-1);

	do {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (sign < 0) {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = '-';
	}
	if (i+1 > len) {
		goto fail;
	}
	s[i] = '\0';
	AG_StrReverse(&s[iStart]);
	return (0);
fail:
	s[0] = '\0';
	return (-1);
}

/*
 * Format an unsigned int (base 10) into a fixed-size buffer (concatenate).
 */
int
AG_StrlcatUint(char *s, Uint val, size_t len)
{
	static const char *digits = "0123456789";
	int i = strlen(s);
	int iStart = i;

	if (len < 1)
		return (-1);

	do {
		if (i+1 > len) {
			goto fail;
		}
		s[i++] = digits[val % 10];
	} while ((val /= 10) > 0);

	if (i+1 > len) {
		goto fail;
	}
	s[i] = '\0';
	AG_StrReverse(&s[iStart]);
	return (0);
fail:
	s[0] = '\0';
	return (-1);
}
