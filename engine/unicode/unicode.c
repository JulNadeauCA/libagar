/*	$Csoft: unicode.c,v 1.2 2003/06/14 22:07:46 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

void
unicode_init(void)
{
}

void
unicode_destroy(void)
{
}

/* Convert a string of a specific encoding to UTF-16. */
void
unicode_convert(enum unicode_conv conv, Uint16 *unicode, const char *text,
    size_t len)
{
	size_t i, j;

	switch (conv) {
	case UNICODE_FROM_ASCII:
	case UNICODE_FROM_LATIN1:
		for (i = 0; i < len; i++) {
			unicode[i] = ((const unsigned char *)text)[i];
		}
		unicode[i] = 0;
		break;
	case UNICODE_FROM_UTF8:
		for (i = 0, j = 0; i < len; i++, j++) {
			Uint16 ch = ((const unsigned char *)text)[i];

			if (ch >= 0xf0) {
				ch  = (Uint16)(text[i]   & 0x07) << 18;
				ch |= (Uint16)(text[++i] & 0x3f) << 12;
				ch |= (Uint16)(text[++i] & 0x3f) << 6;
				ch |= (Uint16)(text[++i] & 0x3f);
			} else if (ch >= 0xe0) {
				ch  = (Uint16)(text[i]   & 0x3f) << 12;
				ch |= (Uint16)(text[++i] & 0x3f) << 6;
				ch |= (Uint16)(text[++i] & 0x3f);
			} else if (ch >= 0xc0) {
				ch  = (Uint16)(text[i]   & 0x3f) << 6;
				ch |= (Uint16)(text[++i] & 0x3f);
			}
			unicode[j] = ch;
		}
		unicode[j] = 0;
		break;
	}
}

/* Duplicate a string and convert to UTF-16. */
Uint16 *
unicode_import(enum unicode_conv conv, const char *text)
{
	Uint16 *unicode;
	size_t len;
	
	len = strlen(text);
	unicode = Malloc((len + 1) * sizeof(Uint16));
	unicode_convert(conv, unicode, text, len);
	return (unicode);
}

/*
 * Return the length of a Unicode text in characters, without the
 * terminating NUL.
 */
size_t
ucslen(const Uint16 *unicode)
{
	size_t len;

	for (len = 0; *unicode != '\0'; unicode++) {
		len++;
	}
	return (len);
}

/* Duplicate a Unicode string. */
Uint16 *
ucsdup(const Uint16 *unicode)
{
	size_t buflen;
	Uint16 *ns;
	
	buflen = (ucslen(unicode)+1) * sizeof(Uint16);
	ns = Malloc(buflen);
	memcpy(ns, unicode, buflen);
	return (ns);
}

