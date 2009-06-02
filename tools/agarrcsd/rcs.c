/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "agarrcsd.h"
#include "rcs.h"
#include "pathnames.h"
#include "fgetln.h"
#include "mkpath.h"

static char *
parse_path(NS_Command *cmd, const char *key)
{
	char *path = NS_CommandString(cmd, key);

	if (path[0] == '/' ||
	    strstr(path, "..") != NULL) {
		AG_SetError("%s: bad object path: `%s'", key, path);
		return (NULL);
	}
	return (path);
}

static char *
parse_name(NS_Command *cmd, const char *key)
{
	char *name = NS_CommandString(cmd, key);
	const char *p;

	for (p = &key[0]; *p != '\0'; p++) {
		if (*p == '/' || *p == '.' || *p == ' ') {
			AG_SetError("%s: bad name/type: `%s'", key, name);
			return (NULL);
		}
	}
	if ((p - &key[0]) > RCSINFO_NAME_MAX) {
		AG_SetError("%s: name too large", key);
		return (NULL);
	}
	return (name);
}

static char *
parse_msg(NS_Command *cmd, const char *key)
{
	char *name = NS_CommandString(cmd, key);
	const char *p;

	for (p = &key[0]; *p != '\0'; p++) {
		if (!isprint(*p)) {
			AG_SetError("%s: non-printable character", key);
			return (NULL);
		}
	}
	if ((p - &key[0]) > RCSINFO_MSG_MAX) {
		AG_SetError("%s: name/type too large", key);
		return (NULL);
	}
	return (name);
}

static size_t
parse_size(NS_Command *cmd, const char *key)
{
	long lval;

	if (NS_CommandLong(cmd, key, &lval) == -1)
		return (0);

	/* Agar signatures are at least 8 bytes. */
	if (lval < 8) {
		AG_SetError("%s: data file is too small (%lu)", key, lval);
		return (0);
	}
	return ((size_t)lval);
}

static int
parse_revision(NS_Command *cmd, const char *key, Uint *rv)
{
	long lval;

	if (NS_CommandLong(cmd, key, &lval) == -1) {
		return (-1);
	}
	if (lval < 1) {
		AG_SetError("%s: invalid revision number", key);
		return (-1);
	}
	*rv = (Uint)lval;
	return (0);
}

int
rcs_commit(NS_Server *ns, NS_Command *cmd, void *p)
{
	char buf[AG_BUFFER_MAX];
	char filepath[AG_PATHNAME_MAX];
	char revno[12];
	char *dirpath, *objname, *objtype;
	size_t nread = 0, size;
	Uint rev;
	int fd;

	if ((dirpath = parse_path(cmd, "object-path")) == NULL ||
	    (size = parse_size(cmd, "object-size")) == 0 ||
	    (objname = parse_name(cmd, "object-name")) == NULL ||
	    (objtype = parse_name(cmd, "object-type")) == NULL) {
		return (-1);
	}

	if (mkpath(dirpath, 0755) == -1) {
		return (-1);
	}
	if (rcsinfo_add_revision(dirpath, user, &rev,
	    parse_msg(cmd, "commit-msg"),
	    parse_msg(cmd, "object-digest"),
	    objtype, objname) == -1)
		return (-1);

	Strlcpy(filepath, dirpath, sizeof(filepath));
	Strlcat(filepath, "/", sizeof(filepath));
	Strlcat(filepath, objname, sizeof(filepath));
	Strlcat(filepath, ".", sizeof(filepath));
	Strlcat(filepath, objtype, sizeof(filepath));
	Strlcat(filepath, ".", sizeof(filepath));
	Snprintf(revno, sizeof(revno), "%u", rev);
	Strlcat(filepath, revno, sizeof(filepath));

	if ((fd = open(filepath, O_CREAT|O_WRONLY|O_EXCL, 0644)) == -1) {
		AG_SetError("%s: %s", filepath, strerror(errno));
		NS_Log(NS_ERR, "%s", AG_GetError());
		return (-1);
	}
	
	NS_Log(NS_INFO, "Importing %s (%lu bytes)", filepath,
	    (u_long)size);

	NS_Message(ns, 0, "go ahead");

	while (nread < size) {
		size_t csize, rv;

		if ((csize = (size - nread)) > sizeof(buf))
			csize = sizeof(buf);

		if ((rv = fread(buf, 1, csize, stdin)) < csize) {
			AG_SetError("incomplete chunk");
			goto fail;
		}
		if (write(fd, buf, rv) < rv) {
			AG_SetError("write error");
			goto fail;
		}
		nread += rv;
	}

	close(fd);
	NS_Message(ns, 0, "New revision: #%u", rev);
	return (0);
fail:
	close(fd);
	NS_Log(NS_ERR, "Import failed: %s: %s", filepath, AG_GetError());
	return (-1);
}

int
rcs_info(NS_Server *ns, NS_Command *cmd, void *p)
{
	char msg[RCSINFO_MSG_MAX];
	char author[RCSINFO_AUTHOR_MAX];
	char digest[RCSINFO_DIGEST_MAX];
	char type[RCSINFO_TYPE_MAX];
	char name[RCSINFO_NAME_MAX];
	char *dirpath;
	Uint rev;

	if ((dirpath = parse_path(cmd, "object-path")) == NULL) {
		return (-1);
	}
	if ((parse_revision(cmd, "revision", &rev)) == -1) {
		rev = RCS_CURRENT_REVISION;
	}
	if (rcsinfo_get_revision(dirpath, &rev, author, digest, msg, type, name)
	    == -1) {
		return (-1);
	}
	NS_Message(ns, 0, "r=%u:a=%s:d=%s:m=%s:t=%s:n=%s\n", rev, author,
	    digest, msg, type, name);
	return (0);
}

int
rcs_update(NS_Server *ns, NS_Command *cmd, void *p)
{
	char buf[AG_BUFFER_MAX];
	char filepath[AG_PATHNAME_MAX];
	char revtext[12];
	char *dirpath, *objname, *objtype;
	Uint rev;
	off_t len;
	ssize_t rv;
	int fd;
	
	if ((dirpath = parse_path(cmd, "object-path")) == NULL ||
	    (objname = parse_name(cmd, "object-name")) == NULL ||
	    (objtype = parse_name(cmd, "object-type")) == NULL ||
	    (parse_revision(cmd, "revision", &rev)) == -1) {
		return (-1);
	}
	
	Strlcpy(filepath, dirpath, sizeof(filepath));
	Strlcat(filepath, "/", sizeof(filepath));
	Strlcat(filepath, objname, sizeof(filepath));
	Strlcat(filepath, ".", sizeof(filepath));
	Strlcat(filepath, objtype, sizeof(filepath));
	Strlcat(filepath, ".", sizeof(filepath));
	Snprintf(revtext, sizeof(revtext), "%u", rev);
	Strlcat(filepath, revtext, sizeof(filepath));

	NS_Log(NS_INFO, "sending %s", filepath);

	if ((fd = open(filepath, O_RDONLY|O_EXCL)) == -1) {
		AG_SetError("%s: %s", filepath, strerror(errno));
		NS_Log(NS_ERR, "%s", AG_GetError());
		return (-1);
	}
	if ((len = lseek(fd, 0, SEEK_END)) == -1) {
		AG_SetError("%s: cannot seek end", filepath);
		goto fail;
	}
	if (lseek(fd, 0, SEEK_SET) == -1) {
		AG_SetError("%s: cannot seek set", filepath);
		goto fail;
	}

	NS_BeginData(ns, (size_t)len);

	while ((rv = read(fd, buf, sizeof(buf))) > 0) {
		if (NS_Data(ns, buf, rv) < rv) {
			AG_SetError("%s: error writing to socket", filepath);
			NS_Log(NS_ERR, "%s", AG_GetError());
			NS_EndData(ns);
			goto fail;
		}
	}
	if (rv < 0) {
		AG_SetError("%s: read error", filepath);
		NS_Log(NS_ERR, "%s", AG_GetError());
		NS_EndData(ns);
		goto fail;
	}
	close(fd);
	NS_EndData(ns);
	return (0);
fail:
	close(fd);
	return (-1);
}

static void
rcs_listdir(NS_Server *ns, const char *dirname, const char *path)
{
	char author[RCSINFO_AUTHOR_MAX];
	char type[RCSINFO_TYPE_MAX];
	char name[RCSINFO_NAME_MAX];
	char subpath[AG_PATHNAME_MAX];
	Uint rev;
	struct stat sb;
	struct dirent *dent;
	DIR *dir;

	if (chdir(dirname) == -1) {
		NS_Log(NS_ERR, "%s: %s", dirname, strerror(errno));
		return;
	}

	rev = RCS_CURRENT_REVISION;
	if (rcsinfo_get_revision(".", &rev, author, NULL, NULL, type, name)
	    == 0) {
		NS_ListString(ns, "%s:%u:%s:%s:%s", path, rev, author, type,
		    name);
	}

	if ((dir = opendir(".")) == NULL) {
		NS_Log(NS_ERR, "%s: %s", dirname, strerror(errno));
		goto out;
	}
	while ((dent = readdir(dir)) != NULL) {
		if (dent->d_name[0] == '.' ||
		    stat(dent->d_name, &sb) == -1) {
			continue;
		}
		if ((sb.st_mode & S_IFDIR) == S_IFDIR) {
			Strlcpy(subpath, path, sizeof(subpath));
			Strlcat(subpath, "/", sizeof(subpath));
			Strlcat(subpath, dent->d_name, sizeof(subpath));
			rcs_listdir(ns, dent->d_name, subpath);
		}
	}
	closedir(dir);
out:
	if (chdir("..") == -1) {
		NS_Log(NS_ERR, "%s/..: %s", dirname, strerror(errno));
		exit(1);
	}
}

int
rcs_list(NS_Server *ns, NS_Command *cmd, void *p)
{
	NS_BeginList(ns);
	rcs_listdir(ns, _PATH_DATA, "");
	NS_EndList(ns);

	if (chdir(_PATH_DATA) == -1) {
		NS_Log(NS_ERR, "%s: %s", _PATH_DATA, AG_GetError());
		return (-1);
	}
	return (0);
}

int
rcs_log(NS_Server *ns, NS_Command *cmd, void *p)
{
	char path[AG_PATHNAME_MAX];
	char *buf, *lbuf, *dirpath;
	FILE *f;
	size_t len;

	if ((dirpath = parse_path(cmd, "object-path")) == NULL) {
		return (-1);
	}
	Strlcpy(path, dirpath, sizeof(path));
	Strlcat(path, "/.RCSInfo", sizeof(path));

	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}

	NS_BeginList(ns);
	lbuf = NULL;
	while ((buf = MyFgetln(f, &len)) != NULL) {
		char *bufp = buf;
		char *rev, *author, *digest, *msg, *type, *name;
		char *c;

		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
		} else {
			if ((lbuf = malloc(len+1)) == NULL) {
				NS_EndList(ns);
				AG_SetError("out of memory");
				goto fail;
			}
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}
		
		rev = Strsep(&bufp, ":");
		author = Strsep(&bufp, ":");
		type = Strsep(&bufp, ":");
		name = Strsep(&bufp, ":");
		digest = Strsep(&bufp, ":");
		msg = Strsep(&bufp, ":");
		if (rev == NULL || author == NULL || type == NULL ||
		    name == NULL || digest == NULL || msg == NULL)
			continue;
	
		for (c = &msg[0]; *c != '\0'; c++) {
			if (*c == ':')
				*c = ';';
		}
		NS_ListString(ns, "%s:%s:%s:%s:%s:%s", rev, author, type,
		    name, digest, msg);
	}
	if (lbuf != NULL) {
		free(lbuf);
	}
	fclose(f);
	NS_EndList(ns);
	return (0);
fail:
	fclose(f);
	return (-1);
}
