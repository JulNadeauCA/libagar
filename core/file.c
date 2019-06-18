/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#ifdef _WIN32
# include <agar/core/queue_close.h>			/* Conflicts */
# ifdef _XBOX
#  include <xtl.h>
# else
#  include <windows.h>
# endif
# include <agar/core/queue_close.h>
# include <agar/core/queue.h>
#else
# include <sys/types.h>
# include <agar/config/_mk_have_sys_stat_h.h>
# ifdef _MK_HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif
# include <unistd.h>
# include <string.h>
# include <errno.h>
#endif

#include <stdio.h>
#include <agar/config/have_getenv.h>
#ifdef HAVE_GETENV
# include <stdlib.h>
#endif

#include <agar/core/core.h>
#ifdef _XBOX
# include <agar/core/xbox.h>
# define INVALID_FILE_ATTRIBUTES -1
#endif

AG_FileExtMapping *agFileExtMap = NULL;
Uint               agFileExtCount = 0;

#ifdef _WIN32
int
AG_GetFileInfo(const char *path, AG_FileInfo *i)
{
	DWORD attrs;
	FILE *f;

#ifdef _XBOX
	if((strlen(path) >= 2) && (path[strlen(path) -1] == '.') && 
	   ((path[strlen(path) -2] == '.') ||
	   (path[strlen(path) -2] == AG_PATHSEPCHAR))) {
		i->type = AG_FILE_DIRECTORY;
		i->perms |= AG_FILE_EXECUTABLE;
		return (0);
	}
	else {
#endif

	if ((attrs = GetFileAttributesA(path)) == INVALID_FILE_ATTRIBUTES) {
		AG_SetError(_("%s: Failed to get information"), path);
		return (-1);
	}
	i->flags = 0;
	i->perms = 0;

	if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
		i->type = AG_FILE_DIRECTORY;
		i->perms |= AG_FILE_EXECUTABLE;
	} else {
		i->type = AG_FILE_REGULAR;
	}
	if (attrs & FILE_ATTRIBUTE_ARCHIVE) i->flags |= AG_FILE_ARCHIVE;
	if (attrs & FILE_ATTRIBUTE_HIDDEN) i->flags |= AG_FILE_HIDDEN;
	if (attrs & FILE_ATTRIBUTE_SYSTEM) i->flags |= AG_FILE_SYSTEM;
	if (attrs & FILE_ATTRIBUTE_TEMPORARY) i->flags |= AG_FILE_TEMPORARY;
	
#ifdef _XBOX
	} /* !if(path[strlen(path) -1] == '.') */
#endif

	if ((f = fopen(path, "rb")) != NULL) {
		fclose(f);
		i->perms |= AG_FILE_READABLE;
	}
	if (((attrs & FILE_ATTRIBUTE_READONLY) == 0) &&
	    (f = fopen(path, "a")) != NULL) {
		fclose(f);
		i->perms |= AG_FILE_WRITEABLE;
	}
	return (0);
}

#else /* !_WIN32 */

int
AG_GetFileInfo(const char *path, AG_FileInfo *i)
{
# ifdef _MK_HAVE_SYS_STAT_H
	struct stat sb;
	uid_t uid = geteuid();
	gid_t gid = getegid();

	if (stat(path, &sb) == -1) {
		AG_SetError(_("%s: Failed to get information (%s)"), path,
		    strerror(errno));
		return (-1);
	}
	i->type = AG_FILE_REGULAR;
	i->flags = 0;
	i->perms = 0;

	if ((sb.st_mode & S_IFDIR)==S_IFDIR) {
		i->type = AG_FILE_DIRECTORY;
	} else if ((sb.st_mode & S_IFLNK)==S_IFLNK) {
		i->type = AG_FILE_SYMLINK;
	} else if ((sb.st_mode & S_IFIFO)==S_IFIFO) {
		i->type = AG_FILE_FIFO;
	} else if ((sb.st_mode & S_IFSOCK)==S_IFSOCK) {
		i->type = AG_FILE_SOCKET;
	} else if ((sb.st_mode & S_IFCHR)==S_IFCHR) {
		i->type = AG_FILE_DEVICE;
	} else if ((sb.st_mode & S_IFBLK)==S_IFBLK) {
		i->type = AG_FILE_DEVICE;
	}
	if ((sb.st_mode & S_ISUID) == S_ISUID) i->flags |= AG_FILE_SUID;
	if ((sb.st_mode & S_ISGID) == S_ISGID) i->flags |= AG_FILE_SGID;
	
	if (sb.st_uid == uid) {
		if ((sb.st_mode & S_IRUSR) == S_IRUSR) { i->perms |= AG_FILE_READABLE; }
		if ((sb.st_mode & S_IWUSR) == S_IWUSR) { i->perms |= AG_FILE_WRITEABLE; }
		if ((sb.st_mode & S_IXUSR) == S_IXUSR) { i->perms |= AG_FILE_EXECUTABLE; }
	} else if (sb.st_gid == gid) {
		if ((sb.st_mode & S_IRGRP) == S_IRGRP) { i->perms |= AG_FILE_READABLE; }
		if ((sb.st_mode & S_IWGRP) == S_IWGRP) { i->perms |= AG_FILE_WRITEABLE; }
		if ((sb.st_mode & S_IXGRP) == S_IXGRP) { i->perms |= AG_FILE_EXECUTABLE; }
	} else {
		if ((sb.st_mode & S_IROTH) == S_IROTH) { i->perms |= AG_FILE_READABLE; }
		if ((sb.st_mode & S_IWOTH) == S_IWOTH) { i->perms |= AG_FILE_WRITEABLE; }
		if ((sb.st_mode & S_IXOTH) == S_IXOTH) { i->perms |= AG_FILE_EXECUTABLE; }
	}
#if 0	
	Verbose("stat[%s]: mode=%x(%s) perms=%x(%s) uid=%d:%d\n", path,
	    sb.st_mode, (sb.st_mode & S_IXUSR) ? "exec" : "",
	    i->perms, (i->perms & AG_FILE_EXECUTABLE) ? "exec" : "",
	    uid, gid);
#endif
	return (0);
# else
	AG_SetErrorS("No stat()");
	return (-1);
# endif
}

#endif /* _WIN32 */

int
AG_GetSystemTempDir(char *buf, AG_Size len)
{
#if defined(_XBOX)
	/* Use a cache partition if it is available */
	if(AG_XBOX_DriveIsMounted('Z')) {
		Strlcpy(buf, "Z:\\", len);
	} else if(AG_XBOX_DriveIsMounted('D')) {
		Strlcpy(buf, "D:\\", len);
	} else {
		return (-1);
	}
#elif defined(_WIN32)
	if (GetTempPathA((DWORD)len, buf) == 0) {
		AG_SetError("GetTempPath() failed");
		return (-1);
	}
#else
# ifdef HAVE_GETENV
	char *s;

	if ((s = getenv("TMPDIR")) != NULL && s[0] != '\0') {
		Strlcpy(buf, s, len);
		if (s[strlen(s) - 1] != AG_PATHSEPCHAR)
			Strlcat(buf, AG_PATHSEP, len);
	} else
# endif
	{
		Strlcpy(buf, "/tmp/", len);
	}
#endif
	return (0);
}

int
AG_FileExists(const char *path)
{
#ifdef _WIN32
	if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND ||
		    GetLastError() == ERROR_PATH_NOT_FOUND) {
			return (0);
		} else {
			AG_SetError(_("%s: Failed to determine existence of "
			              "file (%lu)"), path,
				      (Ulong)GetLastError());
			return (-1);
		}
	} else {
		return (1);
	}
#elif defined(_MK_HAVE_SYS_STAT_H)
	struct stat sb;

	if (stat(path, &sb) == -1) {
		if (errno != ENOENT) {
			AG_SetError(_("%s: Failed to determine existence of "
			              "file (%s)"), path, strerror(errno));
			return (-1);
		}
		return (0);
	} else {
		return (1);
	}
#else
	AG_SetErrorS("No stat()");
	return (-1);
#endif
}

int
AG_FileDelete(const char *path)
{
#ifdef _WIN32
	if (DeleteFileA(path) == 0) {
		AG_SetError(_("%s: Failed to delete file (%lu)"), path,
		    (Ulong)GetLastError());
		return (-1);
	}
	return (0);
#else
	if (unlink(path) == -1) {
		AG_SetError(_("%s: Failed to delete file (%s)"), path,
		    strerror(errno));
		return (-1);
	}
	return (0);
#endif
}

/* Return the last element in a pathname. */
const char *
AG_ShortFilename(const char *p)
{
	const char *s;

	s = (const char *)strrchr(p, AG_PATHSEPCHAR);
	return (s != NULL && s>p) ? &s[1] : p;
}

/* Register a set of file extensions mappings to object classes. */
void
AG_RegisterFileExtMappings(const AG_FileExtMapping *femNew, Uint count)
{
	AG_FileExtMapping *fem;
	Uint i;

	fem = Realloc(agFileExtMap,
	    (agFileExtCount + count)*sizeof(AG_FileExtMapping));
	for (i = 0; i < count; i++) {
		memcpy(&fem[agFileExtCount+i],
		       &femNew[i],
		       sizeof(AG_FileExtMapping));
	}
	agFileExtMap = fem;
	agFileExtCount += count;
}

#endif /* AG_SERIALIZATION */
