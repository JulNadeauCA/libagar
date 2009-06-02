/*	$Csoft: mkpath.c,v 1.5 2004/04/24 04:33:33 vedge Exp $	*/
/*	$OpenBSD: mkdir.c,v 1.14 2002/07/04 04:26:40 deraadt Exp $	*/
/*	$NetBSD: mkdir.c,v 1.14 1995/06/25 21:59:21 mycroft Exp $	*/

/*
 * Copyright (c) 1983, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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

#ifndef lint
char mkpath_copyright[] =
"@(#) Copyright (c) 1983, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>

#include <agar/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mkpath.h"

/*
 * mkpath -- create directories.
 */
int
mkpath(char *path, mode_t mode)
{
	struct stat sb;
	char *pathp, *slash;
	int done = 0;

	slash = pathp = strdup(path);

	while (!done) {
		slash += strspn(slash, "/");
		slash += strcspn(slash, "/");

		done = (*slash == '\0');
		*slash = '\0';

		if (stat(pathp, &sb)) {
			if (errno != ENOENT ||
			    (mkdir(pathp, mode) && errno != EEXIST)) {
				AG_SetError("%s: %s", pathp, strerror(errno));
				return (-1);
			}
		} else if (!S_ISDIR(sb.st_mode)) {
			AG_SetError("%s: %s", pathp, strerror(ENOTDIR));
			return (-1);
		}

		*slash = '/';
	}
	free(pathp);
	return (0);
}

