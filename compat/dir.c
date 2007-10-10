/*	$Csoft: dir.c,v 1.3 2004/04/23 12:44:45 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#include <sys/types.h>

#ifdef __WIN32__
#include <windows.h>
#undef SLIST_ENTRY
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include <core/core.h>
#include <compat/dir.h>
#include <compat/file.h>

int
AG_MkDir(const char *dir)
{
#ifdef __WIN32__
	if (CreateDirectory(dir, NULL)) {
		return (0);
	} else {
		AG_SetError("%s: cannot create dir (%lu)", dir,
		    (Ulong)GetLastError());
		return (-1);
	}
#else
	if (mkdir(dir, 0700) == 0) {
		return (0);
	} else {
		AG_SetError("mk %s: %s", dir, strerror(errno));
		return (-1);
	}
#endif
}

int
AG_RmDir(const char *dir)
{
#ifdef __WIN32__
	if (RemoveDirectory(dir)) {
		return (0);
	} else {
		AG_SetError("%s: cannot remove dir (%lu)", dir,
		    (Ulong)GetLastError());
		return (-1);
	}
#else
	if (rmdir(dir) == 0) {
		return (0);
	} else {
		AG_SetError("rm %s: %s", dir, strerror(errno));
		return (-1);
	}
#endif
}

int
AG_ChDir(const char *dir)
{
#ifdef __WIN32__
	if (SetCurrentDirectory(dir)) {
		return (0);
	} else {
		AG_SetError("%s: cannot set cwd (%lu)", dir,
		    (Ulong)GetLastError());
		return (-1);
	}
#else
	if (chdir(dir) == 0) {
		return (0);
	} else {
		AG_SetError("cd %s: %s", dir, strerror(errno));
		return (-1);
	}
#endif
}

AG_Dir *
AG_OpenDir(const char *path)
{
	AG_Dir *dir;

	dir = Malloc(sizeof(AG_Dir), 0);
	dir->ents = NULL;
	dir->nents = 0;

#ifdef __WIN32__
	{
		HANDLE h;
		WIN32_FIND_DATA fdata;
		DWORD rv;
		char dpath[MAXPATHLEN];

		strlcpy(dpath, path, sizeof(dpath));
		strlcat(dpath, "\\*", sizeof(dpath));
		if ((h = FindFirstFile(dpath, &fdata))==INVALID_HANDLE_VALUE) {
			AG_SetError("Invalid file handle (%lu)",
			    (Ulong)GetLastError());
			goto fail;
		}
		while (FindNextFile(h, &fdata) != 0) {
			dir->ents = Realloc(dir->ents,
			    (dir->nents+1)*sizeof(char *));
			dir->ents[dir->nents++] = Strdup(fdata.cFileName);
		}
		rv = GetLastError();
		FindClose(h);
		if (rv != ERROR_NO_MORE_FILES) {
			AG_SetError("FindNextFileError (%lu)", rv);
			goto fail;
		}
	}
#else /* !__WIN32__ */
	{
		DIR *dp;
		struct dirent *dent;
		
		if ((dp = opendir(path)) == NULL) {
			AG_SetError("%s: %s", path, strerror(errno));
			goto fail;
		}
		while ((dent = readdir(dp)) != NULL) {
			dir->ents = Realloc(dir->ents,
			    (dir->nents+1)*sizeof(char *));
			dir->ents[dir->nents++] = Strdup(dent->d_name);
		}
		closedir(dp);
	}
#endif /* __WIN32__ */

	return (dir);
fail:
	Free(dir, 0);
	return (NULL);
}

void
AG_CloseDir(AG_Dir *dir)
{
	int i;

	for (i = 0; i < dir->nents; i++) {
		Free(dir->ents[i], 0);
	}
	Free(dir->ents, 0);
	Free(dir, 0);
}

int
AG_MkPath(const char *path)
{
	AG_FileInfo info;
	char *pathp, *slash;
	int done = 0;
	int rv;

	slash = pathp = Strdup(path);

	while (!done) {
		slash += strspn(slash, AG_PATHSEP);
		slash += strcspn(slash, AG_PATHSEP);

		done = (*slash == '\0');
		*slash = '\0';

		if (AG_GetFileInfo(pathp, &info) == -1) {
			if ((rv = AG_FileExists(pathp)) == -1) {
				goto fail;
			} else if (rv == 0) {
				if (AG_MkDir(pathp) == -1)
					goto fail;
			}
		} else if (info.type != AG_FILE_DIRECTORY) {
			AG_SetError("%s: existing non-directory", pathp);
			goto fail;
		}

		*slash = AG_PATHSEPC;
	}
	Free(pathp, 0);
	return (0);
fail:
	Free(pathp, 0);
	return (-1);
}

