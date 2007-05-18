/*	$Csoft: agar-config.c,v 1.18 2005/09/18 04:59:10 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <config/version.h>
#include <config/prefix.h>
#include <config/sysconfdir.h>
#include <config/incldir.h>
#include <config/libdir.h>
#include <config/sharedir.h>

#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--version") == 0) {
			printf("%s\n", VERSION);
		} else if (strcmp(argv[i], "--prefix") == 0) {
			printf("%s\n", PREFIX);
		} else if (strcmp(argv[i], "--sysconfdir") == 0) {
			printf("%s\n", SYSCONFDIR);
		} else if (strcmp(argv[i], "--incldir") == 0) {
			printf("%s\n", INCLDIR);
		} else if (strcmp(argv[i], "--libdir") == 0) {
			printf("%s\n", LIBDIR);
		} else if (strcmp(argv[i], "--sharedir") == 0) {
			printf("%s\n", SHAREDIR);
		} else if (strcmp(argv[i], "--cflags") == 0) {
			printf("-I%s ", INCLDIR);
			printf("\n");
		} else if (strcmp(argv[i], "--libs") == 0) {
			printf("-L%s ", LIBDIR);
			printf("-lag_sc");
			printf("\n");
		}
	}
	if (i <= 1) {
		fprintf(stderr,
		    "Usage: %s [--version] [--prefix] [--sysconfdir] "
		    "[--incldir] [--libdir] [--sharedir] "
		    "[--cflags] [--libs]\n", argv[0]);
		return (1);
	}
	return (0);
}

