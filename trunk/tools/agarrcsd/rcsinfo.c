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

#include "agarrcsd.h"
#include "rcs.h"
#include "fgetln.h"

static void
write_revision(FILE *f, u_int nrev, struct user *u, const char *msg,
    const char *digest, const char *type, const char *name)
{
	fprintf(f, "%u:%s:%s:%s:", nrev, u->name, type, name);
	fputs(digest != NULL ? digest : "(no digest)", f);
	fputc(':', f);
	fputs(msg != NULL ? msg : "(no message)", f);
	fputc('\n', f);
}

static int
count_revisions(FILE *f, u_int *nrevs)
{
	char c;

	while ((c = getc(f)) != EOF) {
		if (c == '\n')
			(*nrevs)++;
	}
	if (!feof(f)) {
		AG_SetError("error counting revisions");
		return (-1);
	}
	return (0);
}

int
rcsinfo_get_revision(const char *dirpath, u_int *revp, char *author,
    char *digest, char *msg, char *type, char *name)
{
	char path[AG_PATHNAME_MAX];
	char *buf, *lbuf;
	size_t len;
	FILE *f;
	int found = 0;

	Strlcpy(path, dirpath, sizeof(path));
	Strlcat(path, "/.RCSInfo", sizeof(path));

	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (*revp == RCS_CURRENT_REVISION) {
		if (count_revisions(f, revp) == -1) {
			goto fail;
		}
		rewind(f);
	}

	lbuf = NULL;
	while ((buf = MyFgetln(f, &len)) != NULL) {
		char *bufp = buf;
		char *rev_txt;

		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
		} else {
			if ((lbuf = malloc(len+1)) == NULL) {
				AG_SetError("out of memory");
				goto fail;
			}
			memcpy(lbuf, buf, len);
			lbuf[len] = '\0';
			buf = lbuf;
		}
		
		if ((rev_txt = Strsep(&bufp, ":")) != NULL &&
		    (u_int)strtol(rev_txt, NULL, 10) == *revp) {
			char *author_txt = Strsep(&bufp, ":");
			char *type_txt = Strsep(&bufp, ":");
			char *name_txt = Strsep(&bufp, ":");
			char *digest_txt = Strsep(&bufp, ":");
			char *msg_txt = Strsep(&bufp, ":");

			if (author != NULL)
				Strlcpy(author, author_txt, RCSINFO_AUTHOR_MAX);
			if (digest != NULL)
				Strlcpy(digest, digest_txt, RCSINFO_DIGEST_MAX);
			if (type_txt != NULL)
				Strlcpy(type, type_txt, RCSINFO_TYPE_MAX);
			if (name_txt != NULL)
				Strlcpy(name, name_txt, RCSINFO_NAME_MAX);
			if (msg != NULL)
				Strlcpy(msg, msg_txt, RCSINFO_MSG_MAX);

			found++;
			break;
		}
	}
	if (lbuf != NULL) {
		free(lbuf);
	}
	if (!found) {
		AG_SetError("%s: no such revision: %u", dirpath, *revp);
		goto fail;
	}
	fclose(f);
	return (0);
fail:
	fclose(f);
	return (-1);
}

int
rcsinfo_add_revision(const char *dirpath, struct user *u, u_int *rev,
    const char *msg, const char *digest, const char *type, const char *name)
{
	struct stat sb;
	char path[AG_PATHNAME_MAX];
	FILE *f;

	Strlcpy(path, dirpath, sizeof(path));
	Strlcat(path, "/.RCSInfo", sizeof(path));

	if (stat(path, &sb) == -1) {
		if ((f = fopen(path, "w")) == NULL) {
			AG_SetError("%s: %s", path, strerror(errno));
			return (-1);
		}
		NS_Log(NS_INFO, "%s: import %s", u->name, dirpath);
		write_revision(f, 1, u, "Initial revision", digest, type, name);
		fclose(f);
		*rev = 1;
	} else {
		u_int frev = 0;
		char c;

		if ((f = fopen(path, "a+")) == NULL) {
			AG_SetError("%s: %s", path, strerror(errno));
			return (-1);
		}
		rewind(f);
		if (count_revisions(f, &frev) == -1) {
			goto fail_close;
		}
		if (fseek(f, 0, SEEK_END) == -1) {
			AG_SetError("%s: seek: %s", path, strerror(errno));
			goto fail_close;
		}
		NS_Log(NS_INFO, "%s: commit %s <r%u -> r%u>: %s",
		    u->name, dirpath, frev, frev+1, msg);
		write_revision(f, frev+1, u, msg, digest, type, name);
		fclose(f);
		*rev = (frev+1);
	}
	return (0);
fail_close:
	fclose(f);
fail:
	NS_Log(NS_ERR, "%s", AG_GetError());
	return (-1);
}

