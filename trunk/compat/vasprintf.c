/*	$Csoft$	*/
/*	$OpenBSD: vasprintf.c,v 1.6 1998/10/16 16:11:56 millert Exp $	*/

/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int
vasprintf(char **str, const char *fmt, _BSD_VA_LIST_ ap) 
{
	int ret;
	FILE f;
	unsigned char *_base;

	f._file = -1;
	f._flags = __SWR | __SSTR | __SALC;
	f._bf._base = f._p = (unsigned char *)malloc(128);
	if (f._bf._base == NULL)
		goto err;
	f._bf._size = f._w = 127;		/* Leave room for the NUL */
	ret = vfprintf(&f, fmt, ap);
	if (ret == -1)
		goto err;
	*f._p = '\0';
	_base = realloc(f._bf._base, ret + 1);
	if (_base == NULL)
		goto err;
	*str = (char *)_base;
	return (ret);

err:
	if (f._bf._base) {
		free(f._bf._base);
		f._bf._base = NULL;
	}
	*str = NULL;
	errno = ENOMEM;
	return (-1);
}

