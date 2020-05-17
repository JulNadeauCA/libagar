/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <sys/types.h>

#ifdef _WIN32

# include <agar/core/queue_close.h>			/* Conflicts */
# ifdef _XBOX
#  include <xtl.h>
# else
#  include <windows.h>
# endif
# include <agar/core/queue_close.h>
# include <agar/core/queue.h>

#else /* !_WIN32 */

# include <agar/config/_mk_have_sys_stat_h.h>
# ifdef _MK_HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif

# ifdef __APPLE__
#  ifndef _DARWIN_C_SOURCE
#   define _DARWIN_C_SOURCE /* for dirfd() */
#   define _AGAR_DEFINED_DARWIN_C_SOURCE
#  endif
# endif

# include <dirent.h>

# ifdef _AGAR_DEFINED_DARWIN_C_SOURCE
#  undef _DARWIN_C_SOURCE
# endif

# include <unistd.h>
#endif /* !_WIN32 */

#include <string.h>
#include <errno.h>

#include <agar/core/core.h>
#ifdef _XBOX
# include <agar/core/xbox.h>
#endif

int
AG_MkDir(const char *dir)
{
#ifdef _WIN32
	if (CreateDirectoryA(dir, NULL)) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to create directory (%d)"), dir,
		    (int)GetLastError());
		return (-1);
	}
#else
	if (mkdir(dir, 0700) == 0) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to create directory (%s)"), dir,
		    strerror(errno));
		return (-1);
	}
#endif
}

int
AG_RmDir(const char *dir)
{
#ifdef _WIN32
	if (RemoveDirectoryA(dir)) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to remove directory (%d)"), dir,
		    (int)GetLastError());
		return (-1);
	}
#else
	if (rmdir(dir) == 0) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to remove directory (%s)"), dir,
		    strerror(errno));
		return (-1);
	}
#endif
}

int
AG_ChDir(const char *dir)
{
#ifdef _XBOX
	AG_SetError("Changing directories is not currently supported on Xbox");
	return (-1);
#elif _WIN32
	if (SetCurrentDirectoryA(dir)) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to change directory (%d)"), dir,
		    (int)GetLastError());
		return (-1);
	}
#else
	if (chdir(dir) == 0) {
		return (0);
	} else {
		AG_SetError(_("%s: Failed to change directory (%s)"), dir,
		    strerror(errno));
		return (-1);
	}
#endif
}

AG_Dir *
AG_OpenDir(const char *path)
{
	AG_Dir *dir;

	dir = Malloc(sizeof(AG_Dir));
	dir->ents = NULL;
	dir->nents = 0;
	dir->fd = -1;

#ifdef _WIN32
	{
		char dpath[MAX_PATH];
		HANDLE h;
		WIN32_FIND_DATA fdata;
		DWORD rv;

# ifdef _XBOX
		if(!AG_XBOX_PathIsValid(path)) {
			AG_SetError(_("Invalid file handle (%d)"),
			    (int)GetLastError());
			goto fail;
		}
# endif

		Strlcpy(dpath, path, sizeof(dpath));
		if(dpath[strlen(dpath) - 1] != '\\') {
			Strlcat(dpath, "\\*", sizeof(dpath));
		} else {
			Strlcat(dpath, "*", sizeof(dpath));
		}

		if ((h = FindFirstFileA(dpath, &fdata))==INVALID_HANDLE_VALUE) {
# ifndef _XBOX
			AG_SetError(_("Invalid file handle (%d)"),
			    (int)GetLastError());
			goto fail;
# endif
		}

# ifdef _XBOX
		/* On Xbox we need to manually include "." and ".." */
		dir->ents = Realloc(dir->ents,
		    (dir->nents+2)*sizeof(char *));
		dir->ents[dir->nents++] = Strdup(".");
		dir->ents[dir->nents++] = Strdup("..");

		/* If the path was empty the handle will be invalid */
		if(h == INVALID_HANDLE_VALUE) {
			return dir;
		}
# endif
		do {
			dir->ents = Realloc(dir->ents,
			    (dir->nents+1)*sizeof(char *));
			dir->ents[dir->nents++] = Strdup(fdata.cFileName);
		} while (FindNextFileA(h, &fdata) != 0);
		rv = GetLastError();
		FindClose(h);
		if (rv != ERROR_NO_MORE_FILES) {
			AG_SetError("FindNextFile Error (%lu)", rv);
			goto fail;
		}
	}
#else /* !_WIN32 */
	{
		struct dirent *dent;
		
		if ((dir->dirp = (void *)opendir(path)) == NULL) {
			AG_SetError(_("%s: Failed to open directory (%s)"),
			    path, strerror(errno));
			goto fail;
		}
		while ((dent = readdir((DIR *)dir->dirp)) != NULL) {
			dir->ents = Realloc(dir->ents,
			    (dir->nents+1)*sizeof(char *));
			dir->ents[dir->nents++] = Strdup(dent->d_name);
		}
		dir->fd = dirfd((DIR *)dir->dirp);
	}
#endif /* !_WIN32 */

	return (dir);
fail:
	Free(dir);
	return (NULL);
}

void
AG_CloseDir(AG_Dir *dir)
{
	int i;

#ifndef _WIN32
	if (dir->dirp)
		closedir(dir->dirp);
#endif

	for (i = 0; i < dir->nents; i++) {
		free(dir->ents[i]);
	}
	Free(dir->ents);
	free(dir);
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
			AG_SetError(_("%s: Existing file"), pathp);
			goto fail;
		}

		*slash = AG_PATHSEPCHAR;
	}
	Free(pathp);
	return (0);
fail:
	Free(pathp);
	return (-1);
}

int
AG_GetCWD(char *buf, AG_Size len)
{
#ifdef _XBOX
	if(buf == NULL) {
		buf = TryStrdup("D:\\");
		len = 4;
	} else {
		if(len >= 4)
			Strlcpy(buf, "D:\\", len);
	}
	if(buf == NULL) {
		AG_SetError("Failed to get current directory");
		return (-1);
	}
	return (0);
#elif _WIN32
	DWORD rv;

	if ((rv = GetCurrentDirectoryA(len, buf)) == 0) {
		AG_SetError(_("Failed to get current directory (%d)"),
		    (int)GetLastError());
		return (-1);
	} else if (rv > len) {
		AG_SetError(_("Failed to get current directory (%s)"),
		    _("Path name is too long"));
		return (-1);
	}
	return (0);
#else
	if (getcwd(buf, len) == NULL) {
		AG_SetError(_("Failed to get current directory (%s)"),
		    strerror(errno));
		return (-1);
	}
	return (0);
#endif
}

#endif /* AG_SERIALIZATION */
