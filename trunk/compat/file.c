/*	$Csoft: dir.c,v 1.3 2004/04/23 12:44:45 vedge Exp $	*/

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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif

#include <core/core.h>
#include <compat/file.h>

int
AG_GetFileInfo(const char *path, AG_FileInfo *i)
{
#ifdef WIN32
	DWORD attrs, type;
	
	if ((attrs=GetFileAttributes(path))==INVALID_FILE_ATTRIBUTES) {
		AG_SetError("%s: cannot get attrs", path);
		return (-1);
	}
	i->flags = 0;
	if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
		i->type = AG_FILE_DIRECTORY;
	} else {
		i->type = AG_FILE_REGULAR;
	}
	if (attrs & FILE_ATTRIBUTE_ARCHIVE) i->flags |= AG_FILE_ARCHIVE;
	if (attrs & FILE_ATTRIBUTE_COMPRESSED) i->flags |= AG_FILE_COMPRESSED;
	if (attrs & FILE_ATTRIBUTE_ENCRYPTED) i->flags |= AG_FILE_ENCRYPTED;
	if (attrs & FILE_ATTRIBUTE_HIDDEN) i->flags |= AG_FILE_HIDDEN;
	if (attrs & FILE_ATTRIBUTE_READONLY) i->flags |= AG_FILE_READONLY;
	if (attrs & FILE_ATTRIBUTE_SPARSE_FILE) i->flags |= AG_FILE_SPARSE;
	if (attrs & FILE_ATTRIBUTE_SYSTEM) i->flags |= AG_FILE_SYSTEM;
	if (attrs & FILE_ATTRIBUTE_TEMPORARY) i->flags |= AG_FILE_TEMPORARY;
	if (attrs & FILE_ATTRIBUTE_REPARSE_POINT)
		i->flags |= AG_FILE_REPARSE_POINT;
	if (attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
		i->flags |= AG_FILE_NOT_CONTENT_IDX;
	return (0);

#else /* !WIN32 */

	struct stat sb;

	if (stat(path, &sb) == -1) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	i->type = AG_FILE_REGULAR;
	i->flags = 0;
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
	if ((sb.st_mode & S_ISUID)==S_ISUID) i->flags |= AG_FILE_SUID;
	if ((sb.st_mode & S_ISGID)==S_ISGID) i->flags |= AG_FILE_SGID;
	return (0);

#endif /* WIN32 */
}

int
AG_FileExists(const char *path)
{
#ifdef WIN32
	
#else
	struct stat sb;

	if (stat(path, &sb) == -1) {
		if (errno == ENOENT) {
			return (0);
		} else {
			AG_SetError("%s: %s", path, strerror(errno));
			return (-1);
		}
	} else {
		return (1);
	}
#endif /* WIN32 */
}
