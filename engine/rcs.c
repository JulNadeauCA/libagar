/*	$Csoft: rcs.c,v 1.14 2005/09/20 13:46:29 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/typesw.h>

#include <compat/md5.h>
#include <compat/sha1.h>
#include <compat/rmd160.h>

#include <config/network.h>
#include <config/version.h>

#include <engine/widget/text.h>
#include <engine/widget/tlist.h>
#include <engine/widget/tableview.h>

#include "rcs.h"

char agRcsHostname[64] = "localhost";
char agRcsUsername[32] = "anonymous";
char agRcsPassword[32] = "";
u_int agRcsPort = 6785;
int agRcsMode = 0;

#ifdef NETWORK

#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <qnet/qnet.h>
#include <qnet/client.h>

static struct client rcs_client;
static int connected = 0;
const char *agRcsStatusStrings[] = {
	N_("Error"),
	N_("Not in repository"),
	N_("Up-to-date"),
	N_("Modified locally"),
	N_("Desynchronized")
};

void
AG_RcsInit(void)
{
	client_init(&rcs_client, "agar", VERSION);
}

void
AG_RcsDestroy(void)
{
	client_destroy(&rcs_client);
}

int
AG_RcsConnect(void)
{
	char port[12];

	if (++connected == 1) {
		snprintf(port, sizeof(port), "%u", agRcsPort);
		if (client_connect(&rcs_client, agRcsHostname, port,
		    agRcsUsername, agRcsPassword) == -1) {
			AG_SetError("RCS connection: %s", qerror_get());
			return (-1);
		}
	}
	return (0);
}

void
AG_RcsDisconnect(void)
{
	if (--connected == 0)
		client_disconnect(&rcs_client);
}

/* Get the working revision of an object. */
int
AG_RcsGetWorkingRev(AG_Object *ob, u_int *pRev)
{
	char path[MAXPATHLEN];
	char buf[12];
	size_t rv;
	long rev;
	char *ep;
	FILE *f;

	AG_ObjectCopyFilename(ob, path, sizeof(path));
	strlcat(path, ".revision", sizeof(path));

	/* TODO locking */
	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	rv = fread(buf, 1, sizeof(buf), f);
	buf[rv-1] = '\0';
	fclose(f);

	errno = 0;
	rev = strtol(buf, &ep, 10);
	if (buf[0] == '\0' || *ep != '\0') {
		AG_SetError("%s: not a number", path);
		return (-1);
	}
#if 0
	if ((errno == ERANGE && (rev == LONG_MAX || rev == LONG_MIN)) ||
	    (rev > INT_MAX || rev < INT_MIN)) {
		AG_SetError("%s: out of range", path);
		return (-1);
	}
#endif
	*pRev = rev;
	return (0);
}

/* Write the working revision of an object. */
int
AG_RcsSetWorkingRev(AG_Object *ob, u_int rev)
{
	char path[MAXPATHLEN];
	FILE *f;

	AG_ObjectCopyFilename(ob, path, sizeof(path));
	strlcat(path, ".revision", sizeof(path));
	
	/* TODO locking */
	if ((f = fopen(path, "w")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	fprintf(f, "%u\n", rev);
	fclose(f);

	return (0);
}

/* Obtain the RCS status of an object. */
enum ag_rcs_status
AG_RcsStatus(AG_Object *ob, const char *objdir, const char *digest,
    char *name, char *type, u_int *repo_rev, u_int *working_rev)
{
	enum ag_rcs_status rv = AG_RCS_UPTODATE;
	char *buf, *bufp;
	int sum_match = 1;
	int i;
	char *s;
	
	if (client_write(&rcs_client, "rcs-info\nobject-path=%s\n\n",
	    &objdir[1]) == -1) {
		AG_SetError("%s", qerror_get());
		return (AG_RCS_ERROR);
	}
	if (client_read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		return (AG_RCS_UNKNOWN);
	}

	buf = &rcs_client.read.buf[2];
	*repo_rev = 0;
	while ((s = strsep(&buf, ":")) != NULL) {
		char *key = strsep(&s, "=");
		char *val = strsep(&s, "=");

		if (key == NULL || val == NULL)
			continue;
		
		if (strcmp(key, "d") == 0 &&
		    strcmp(val, digest) != 0) {
			sum_match = 0;
		} else if (strcmp(key, "r") == 0) {
			*repo_rev = (u_int)strtol(val, NULL, 10);
		} else if (strcmp(key, "t") == 0 && type != NULL) {
			strlcpy(type, val, AG_OBJECT_TYPE_MAX);
		} else if (strcmp(key, "n") == 0 && name != NULL) {
			strlcpy(type, val, AG_OBJECT_NAME_MAX);
		}
	}

	if (AG_RcsGetWorkingRev(ob, working_rev) == -1)
		return (AG_RCS_ERROR);

	if (*working_rev == *repo_rev) {
		if (sum_match) {
			return (AG_RCS_UPTODATE);
		} else {
			return (AG_RCS_LOCALMOD);
		}
	} else if (*working_rev < *repo_rev) {
		return (AG_RCS_DESYNCH);
	} else {
		AG_SetError("Working revision %u < %u", *working_rev, *repo_rev);
		return (AG_RCS_ERROR);
	}
}

/* Import a new object into the repository. */
int
AG_RcsImport(AG_Object *ob)
{
	char buf[BUFSIZ];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;
	enum ag_rcs_status status;

	if (AG_ObjectCopyName(ob, objdir, sizeof(objdir)) == -1 ||
	    AG_ObjectCopyDigest(ob, &len, digest) == -1 ||
	    AG_ObjectCopyFilename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (AG_RcsConnect() == -1)
		return (-1);

	switch ((status = AG_RcsStatus(ob, objdir, digest, NULL, NULL,
	    &repo_rev, &working_rev))) {
	case AG_RCS_ERROR:
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		break;
	case AG_RCS_UNKNOWN:
		break;
	default:
		AG_SetError(_("Object %s is already in repository (r#%u):\n"
		            "Status: %s\n"),
		    ob->name, repo_rev, agRcsStatusStrings[status]);
		goto fail;
	}
	
	/* TODO locking */
	if ((f = fopen(objpath, "r")) == NULL) {
		AG_SetError("%s: %s", objpath, strerror(errno));
		AG_RcsDisconnect();
		return (-1);
	}

	if (client_write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &objdir[1], ob->name, ob->type,
	    (u_long)len, digest) == -1)
		goto fail_close;
	
	if (client_read(&rcs_client, 12) <= 2 ||
	    rcs_client.read.buf[0] != '0') {
		AG_SetError(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail_close;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			AG_SetError(_("EOF from server"));
			goto fail_close;
		} else if (nw < 0) {
			AG_SetError(_("Write error"));
			goto fail_close;
		}
		wrote += nw;
	}
	if (wrote < len) {
		AG_SetError(_("Upload incomplete"));
		goto fail_close;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}

	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Object %s successfully imported to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	AG_RcsSetWorkingRev(ob, 1);
	fclose(f);
	AG_RcsDisconnect();
	return (0);
fail_close:
	fclose(f);
fail:
	AG_RcsDisconnect();
	return (-1);
}

/* Commit changes to an object. */
int
AG_RcsCommit(AG_Object *ob)
{
	char buf[BUFSIZ];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;

	if (AG_ObjectCopyName(ob, objdir, sizeof(objdir)) == -1 ||
	    AG_ObjectCopyDigest(ob, &len, digest) == -1 ||
	    AG_ObjectCopyFilename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (AG_RcsConnect() == -1)
		return (-1);

	switch (AG_RcsStatus(ob, objdir, digest, NULL, NULL, &repo_rev,
	    &working_rev)) {
	case AG_RCS_LOCALMOD:
		break;
	case AG_RCS_ERROR:
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		break;
	case AG_RCS_UNKNOWN:
		AG_SetError(
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"),
		      ob->name);
		goto fail;
	case AG_RCS_UPTODATE:
		AG_SetError(
		    _("Working copy of %s is up-to-date (r#%u)."),
		    ob->name, repo_rev);
		goto fail;
	case AG_RCS_DESYNCH:
		AG_SetError(
		    _("Your copy of %s is outdated (working=r#%u, repo=r#%u)\n"
		      "Please do a RCS update prior to committing.\n"),
		       ob->name, working_rev, repo_rev);
		goto fail;
	}
	
	/* TODO locking */
	if ((f = fopen(objpath, "r")) == NULL) {
		AG_SetError("%s: %s", objpath, strerror(errno));
		AG_RcsDisconnect();
		return (-1);
	}

	if (client_write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &objdir[1], ob->name, ob->type,
	    (u_long)len, digest) == -1)
		goto fail_close;
	
	if (client_read(&rcs_client, 12) <= 2 ||
	    rcs_client.read.buf[0] != '0') {
		AG_SetError(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail_close;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			AG_SetError(_("EOF from server"));
			goto fail_close;
		} else if (nw < 0) {
			AG_SetError(_("Write error"));
			goto fail_close;
		}
		wrote += nw;
	}
	if (wrote < len) {
		AG_SetError(_("Upload incomplete"));
		goto fail_close;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}

	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Object %s successfully committed to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	AG_RcsSetWorkingRev(ob, repo_rev+1);
	fclose(f);
	AG_RcsDisconnect();
	return (0);
fail_close:
	fclose(f);
fail:
	AG_RcsDisconnect();
	return (-1);
}

/* Update a working copy of an object from the repository. */
int
AG_RcsUpdate(AG_Object *ob)
{
	char buf[BUFSIZ];
	char type[AG_OBJECT_TYPE_MAX];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	u_int working_rev, repo_rev;
	struct response *res;
	size_t len, wrote = 0;
	FILE *f;
	
	if (AG_ObjectCopyName(ob, objdir, sizeof(objdir)) == -1 ||
	    AG_ObjectCopyDigest(ob, &len, digest) == -1 ||
	    AG_ObjectCopyFilename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (AG_RcsConnect() == -1)
		return (-1);
	
	switch (AG_RcsStatus(ob, objdir, digest, NULL, type, &repo_rev,
	    &working_rev)) {
	case AG_RCS_ERROR:
		goto fail;
	case AG_RCS_UNKNOWN:
		AG_SetError(
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"), ob->name);
		goto fail;
	case AG_RCS_UPTODATE:
		AG_SetError(
		    _("Working copy of %s is already up-to-date (r#%u)."),
		    ob->name, repo_rev);
		goto fail;
	case AG_RCS_LOCALMOD:
#if 1
		/* TODO move working version */
		AG_SetError(
		    _("Working copy of %s was modified locally.\n"
		      "Please do a RCS commit prior to updating.\n"),
		       ob->name);
		goto fail;
#endif
	case AG_RCS_DESYNCH:
		break;
	}
	if (strcmp(type, ob->type) != 0) {
		AG_SetError(_("Repository has different object type (%s/%s)"),
		    type, ob->type);
		goto fail;
	}

	res = client_query_binary(&rcs_client, "rcs-update\n"
			                       "object-path=%s\n"
			                       "object-name=%s\n"
			                       "object-type=%s\n"
					       "revision=%u\n",
					       &objdir[1], ob->name, ob->type,
					       repo_rev);
	if (res == NULL || res->argc < 1) {
		AG_SetError("RCS update error: %s", qerror_get());
		goto fail;
	}

	/* TODO locking, backup */
	if ((f = fopen(objpath, "w")) == NULL) {
		AG_SetError("%s: %s", objpath, strerror(errno));
		goto fail_res;
	}
	if (fwrite(res->argv[0], 1, res->argv_len[0], f) < res->argv_len[0]) {
		AG_SetError("%s: write error", objpath);
		fclose(f);
		goto fail_res;
	}
	fclose(f);
	AG_RcsSetWorkingRev(ob, repo_rev);
	AG_TextTmsg(AG_MSG_INFO, 1000, "%s: r#%u -> r#%u (%lu bytes)", ob->name,
	    working_rev, repo_rev, (unsigned long)res->argv_len[0]);
	response_free(res);
	AG_RcsDisconnect();

	/*
	 * Load the generic part of the object since the object manager
	 * only does a page-in.
	 */
	if (AG_ObjectLoad(ob) == -1 ||
	    AG_ObjectSave(ob) == -1) {
		AG_SetError("%s: %s", ob->name, AG_GetError());
		return (-1);
	}

	return (0);
fail_res:
	response_free(res);
fail:
	AG_RcsDisconnect();
	return (-1);
}

int
AG_RcsImportAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockLinkage();
	if (AG_RcsImport(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (AG_RcsImportAll(cobj) == -1)
			goto fail;
	}
	AG_UnlockLinkage();
	return (0);
fail:
	AG_UnlockLinkage();
	return (-1);
}

int
AG_RcsUpdateAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockLinkage();
	if (AG_RcsUpdate(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (AG_RcsUpdateAll(cobj) == -1)
			goto fail;
	}
	AG_UnlockLinkage();
	return (0);
fail:
	AG_UnlockLinkage();
	return (-1);
}

int
AG_RcsCommitAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockLinkage();
	if (AG_RcsCommit(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (AG_RcsCommitAll(cobj) == -1)
			goto fail;
	}
	AG_UnlockLinkage();
	return (0);
fail:
	AG_UnlockLinkage();
	return (-1);
}

int
AG_RcsLog(const char *objdir, AG_Tlist *tl)
{
	struct response *res;
	int i;

	res = client_query(&rcs_client, "rcs-log\n"
				        "object-path=%s\n", &objdir[1]);
	if (res == NULL) {
		AG_SetError("%s", qerror_get());
		return (-1);
	}

	for (i = 0; i < res->argc; i++) {
		char *s = res->argv[i];
		char *rev = strsep(&s, ":");
		char *author = strsep(&s, ":");
		char *type = strsep(&s, ":");
		char *name = strsep(&s, ":");
		char *sum = strsep(&s, ":");
		char *msg = strsep(&s, ":");
		AG_ObjectType *t;
		SDL_Surface *icon = NULL;

		if (rev == NULL || author == NULL || sum == NULL)
			continue;
		
		for (t = &agTypes[0]; t < &agTypes[agnTypes]; t++) {
			if (strcmp(type, t->type) == 0) {
				icon = t->icon >= 0 ? AGICON(t->icon) : NULL;
				break;
			}
		}
		AG_TlistAdd(tl, icon, "[#%s.%s] %s", rev, author, msg);
	}
	response_free(res);
	return (0);
}

int
AG_RcsList(AG_Tlist *tl)
{
	struct response *res;
	AG_TlistItem *it;
	int i;

	if ((res = client_query(&rcs_client, "rcs-list\n")) == NULL) {
		AG_SetError("%s", qerror_get());
		return (-1);
	}
	AG_TlistClear(tl);
	it = AG_TlistAdd(tl, NULL, "/");
	it->flags |= AG_TLIST_HAS_CHILDREN;
	it->class = "object";
	it->depth = 0;

	for (i = 0; i < res->argc; i++) {
		char *s = res->argv[i];
		char *name = strsep(&s, ":");
		char *rev = strsep(&s, ":");
		char *author = strsep(&s, ":");
		char *type = strsep(&s, ":");
		AG_ObjectType *t;
		SDL_Surface *icon = NULL;
		AG_TlistItem *it;
		char *c;
		int depth = 0;

		if (name == NULL || rev == NULL || author == NULL ||
		    type == NULL)
			continue;

		for (t = &agTypes[0]; t < &agTypes[agnTypes]; t++) {
			if (strcmp(type, t->type) == 0) {
				icon = t->icon >= 0 ? AGICON(t->icon) : NULL;
				break;
			}
		}
		for (c = &name[0]; *c != '\0'; c++) {
			if (*c == '/')
				depth++;
		}

		it = AG_TlistAdd(tl, icon, "%s", &name[1]);
		it->class = "object";
		it->depth = depth;
	}
out:
	AG_TlistRestore(tl);
	response_free(res);
	return (0);
}

int
AG_RcsDelete(const char *path)
{
	struct response *res;

	if (AG_RcsConnect() == -1)
		return (-1);
	
	res = client_query(&rcs_client, "rcs-delete\n"
				        "object-path=%s\n", path);
	if (res == NULL) {
		AG_SetError("%s", qerror_get());
		AG_RcsDisconnect();
		return (-1);
	}
	response_free(res);
	AG_RcsDisconnect();
	return (0);
}

int
AG_RcsRename(const char *from, const char *to)
{
	struct response *res;

	if (AG_RcsConnect() == -1)
		return (-1);
	
	res = client_query(&rcs_client, "rcs-rename\n"
				        "from-path=%s\n"
					"to-path=%s\n", from, to);
	if (res == NULL) {
		AG_SetError("%s", qerror_get());
		AG_RcsDisconnect();
		return (-1);
	}
	response_free(res);
	AG_RcsDisconnect();
	return (0);
}

int
AG_RcsCheckout(const char *path)
{
	char localpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	char name[AG_OBJECT_NAME_MAX];
	char type[AG_OBJECT_TYPE_MAX];
	char *buf, *s;
	int i;
	u_int rev = 0;
	AG_Object *obj;
	AG_ObjectType *t;

	if (AG_RcsConnect() == -1)
		goto fail;

	/* Fetch the object information from the repository. */
	if (client_write(&rcs_client, "rcs-info\nobject-path=%s\n\n", path)
	    == -1) {
		AG_SetError("%s", qerror_get());
		goto fail;
	}
	if (client_read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError("RCS info: %s", rcs_client.read.buf);
		goto fail;
	}
	buf = &rcs_client.read.buf[2];
	while ((s = strsep(&buf, ":")) != NULL) {
		char *key = strsep(&s, "=");
		char *val = strsep(&s, "=");

		if (key == NULL || val == NULL)
			continue;
		
		switch (key[0]) {
		case 'd':
			strlcpy(digest, val, AG_OBJECT_DIGEST_MAX);
			break;
		case 'r':
			rev = (u_int)strtol(val, NULL, 10);
			break;
		case 't':
			strlcpy(type, val, AG_OBJECT_TYPE_MAX);
			break;
		case 'n':
			strlcpy(name, val, AG_OBJECT_NAME_MAX);
			break;
		}
	}
	for (t = &agTypes[0]; t < &agTypes[agnTypes]; t++) {
		if (strcmp(type, t->type) == 0)
			break;
	}
	if (t == &agTypes[agnTypes]) {
		AG_SetError(_("Unimplemented object type: %s"), type);
		goto fail;
	}

	/* Create the working copy if it does not exist. */
	localpath[0] = '/';
	localpath[1] = '\0';
	strlcat(localpath, path, sizeof(localpath));
	if ((obj = AG_ObjectFind(localpath)) == NULL) {
		AG_TextTmsg(AG_MSG_INFO, 750,
		    _("Creating working copy of %s (%s)."),
		    name, type);

		obj = Malloc(t->size, M_OBJECT);
		if (t->ops->init != NULL) {
			t->ops->init(obj, name);
		} else {
			AG_ObjectInit(obj, t->type, name, NULL);
		}

		if (AG_ObjectAttachPath(localpath, obj) == -1) {
			AG_ObjectDestroy(obj);
			goto fail;
		}
		AG_ObjectUnlinkDatafiles(obj);
		if (AG_ObjectSave(obj) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", name, AG_GetError());
		}
		if (AG_RcsSetWorkingRev(obj, 0) == -1) {
			AG_ObjectDetach(obj);
			AG_ObjectDestroy(obj);
			goto fail;
		}
	} else {
		if (strcmp(type, obj->type) != 0) {
			AG_SetError("%s: existing object of different type (%s)",
			    localpath, obj->type);
			goto fail;
		}
	}

	if (AG_RcsUpdate(obj) == -1) {
		goto fail;
	}
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Object %s updated."), name);
	AG_RcsDisconnect();
	return (0);
fail:
	AG_RcsDisconnect();
	return (-1);
}

#endif /* NETWORK */
