/*	$Csoft: ucslcat.c,v 1.1 2003/06/14 11:28:04 vedge Exp $	*/

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

#include <engine/engine.h>

#include <sys/types.h>
#include <string.h>

#include "unicode.h"

/*
 * Appends src to string dst of size bytes. At most (bytes-1)/sizeof(u16)
 * Unicode characters will be copied.
 *
 * Always NUL terminates (unless bytes <= ucslen(dst)*sizeof(u16)).
 * Returns (ucslen(src) + MIN(bytes, ucslen(initial dst))) * sizeof(u16)
 * If retval >= bytes, truncation occurred.
 */
size_t
ucslcat(Uint16 *dst, const Uint16 *src, size_t bytes)
{
	Uint16 *d = dst;
	const Uint16 *s = src;
	size_t siz = bytes / sizeof(Uint16);
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end. */
	while (n-- != 0 && *d != '\0') {
		d++;
	}
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0) {
		return ((dlen + ucslen(s))*sizeof(Uint16));
	}
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return ((dlen + (s - src))*sizeof(Uint16));   /* Does not include NUL */
}

