/*
 * Copyright (c) 2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Common code between AG_FileDlg and AG_DirDlg.
 */

#include <agar/config/have_glob.h>
#ifdef HAVE_GLOB
# include <glob.h>
#endif

/* Evaluate if path is a filesystem root. */
static __inline__ int
AG_PathIsFilesystemRoot(const char *path)
{
#ifdef _WIN32
	return isalpha(path[0]) && path[1] == ':' &&
	       (path[2] == '\0' || (path[2] == '\\' && path[3] == '\0'));
#else
	return (path[0] == AG_PATHSEPCHAR && path[1] == '\0');
#endif
}

/* Evaluate if path is absolute. */
static __inline__ int
AG_PathIsAbsolute(const char *path)
{
#ifdef _WIN32
	return isalpha(path[0]) && path[1] == ':' &&
	       (path[2] == '\0' || path[2] == '\\');
#else
	return (path[0] == AG_PATHSEPCHAR);
#endif
}

/* Compare two file names. */
static int
AG_FilenameCompare(const void *p1, const void *p2)
{
	const char *s1 = *(const void **)p1;
	const char *s2 = *(const void **)p2;

	return (strcmp(s1, s2));
}

/* Preprocess a user-specified path or filename. */
static int
ProcessFilename(char *file, size_t len)
{
	char *end = &file[strlen(file)-1];
	char *s;

	/* Remove trailing whitespaces. */
	while ((end >= file) && *end == ' ') {
		*end = '\0';
		end--;
	}
	if (file[0] == '\0')
		return (-1);

	/* Remove leading whitespaces. */
	for (s = file; *s == ' '; s++)
		;;
	if (s > file) {
		memmove(file, s, end-s+2);
		end -= (s-file);
	}
	if (file[0] == '\0')
		return (-1);

	/* Treat the root specially. */
	if (strcmp(file, AG_PATHSEP) == 0)
		return (0);

	/* Remove trailing path separators. */
	if (*end == AG_PATHSEPCHAR) {
		*end = '\0';
		end--;
	}
	return (0);
}
