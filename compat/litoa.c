/*	$Csoft: dir.c,v 1.3 2004/04/23 12:44:45 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include "litoa.h"

size_t
litoa(int n, char *s, size_t bufsz)
{
	int sign, size;
	int i, j, c;

	if ((sign = n) < 0)
		n = -n;

	size = 0;
	i = 0;
	do {
		if ((++size + 2) >= bufsz) {   /* Leave room for sign and NUL */
			return (size+1);
		}
		s[i++] = n%10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0) {
		s[i++] = '-';
		size++;
	}
	s[i] = '\0';

	for (i = 0, j = size-1;
	     i < j;
	     i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
	return (size);
}
