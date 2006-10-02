/*	$Csoft: unicode.c,v 1.12 2005/05/12 06:38:36 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#include "unicode.h"

/* Parse the first byte of a possible UTF8 sequence. */
static __inline__ int
utf8_length(Uint8 ch)
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

/* Return the UCS-4 representation of the given string/encoding. */
Uint32 *
AG_ImportUnicode(enum ag_unicode_conv conv, const char *s)
{
	Uint32 *ucs;
	size_t len;
	size_t i, j;

	len = strlen(s);
	ucs = Malloc((len + 1) * sizeof(Uint32), 0);		/* XXX */

	switch (conv) {
	case AG_UNICODE_FROM_US_ASCII:
		for (i = 0; i < len; i++) {
			ucs[i] = ((const unsigned char *)s)[i];
		}
		ucs[i] = '\0';
		break;
	case AG_UNICODE_FROM_UTF8:
		for (i = 0, j = 0; i < len; i++, j++) {
			switch (utf8_length(s[i])) {
			case 1:
				ucs[j] = (Uint32)s[i];
				break;
			case 2:
				ucs[j]  = (Uint32)(s[i]   & 0x3f) << 6;
				ucs[j] |= (Uint32)(s[++i] & 0x3f);
				break;
			case 3:
				ucs[j]  = (Uint32)(s[i]   & 0x3f) << 12;
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
	case AG_UNICODE_FROM_US_ASCII:
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
			switch (utf8_length(s[i])) {
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
ssize_t
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
				return ((ssize_t)len+chlen);
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
		return (len);
	default:
		return (-1);
	}
}

/*
 * Return the length of a UCS-4 string in characters, without the
 * terminating NUL.
 */
size_t
AG_UCS4Len(const Uint32 *ucs)
{
	size_t len;

	for (len = 0; *ucs != '\0'; ucs++) {
		len++;
	}
	return (len);
}

/* Duplicate a UCS-4 string. */
Uint32 *
AG_UCS4Dup(const Uint32 *ucs)
{
	size_t buflen;
	Uint32 *ns;
	
	buflen = (AG_UCS4Len(ucs)+1) * sizeof(Uint32);
	ns = Malloc(buflen, 0);
	memcpy(ns, ucs, buflen);
	return (ns);
}

