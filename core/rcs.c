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

/*
 * Built-in revision control system for AG_Object.
 */

#include <core/core.h>
#include <core/md5.h>
#include <core/sha1.h>
#include <core/rmd160.h>

#include <config/ag_network.h>
#include <config/version.h>

#include "rcs.h"

char agRcsHostname[64] = "localhost";
char agRcsUsername[32] = "anonymous";
char agRcsPassword[32] = "";
Uint agRcsPort = 6785;
int agRcsMode = 0;

#ifdef AG_NETWORK

#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "net.h"

static NC_Session rcs_client;
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
	NC_Init(&rcs_client, "agarrcs", "1.1");
}

void
AG_RcsDestroy(void)
{
	NC_Destroy(&rcs_client);
}

int
AG_RcsConnect(void)
{
	char port[12];

	if (++connected == 1) {
		StrlcpyUint(port, agRcsPort, sizeof(port));
		if (NC_Connect(&rcs_client, agRcsHostname, port,
		    agRcsUsername, agRcsPassword) == -1) {
			AG_SetError("RCS connection: %s", AG_GetError());
			return (-1);
		}
	}
	return (0);
}

void
AG_RcsDisconnect(void)
{
	if (--connected == 0)
		NC_Disconnect(&rcs_client);
}

/* Get the working revision of an object. */
int
AG_RcsGetWorkingRev(AG_Object *ob, Uint *pRev)
{
	char path[AG_PATHNAME_MAX];
	char buf[12];
	size_t rv;
	long rev;
	char *ep;
	FILE *f;

	AG_ObjectCopyFilename(ob, path, sizeof(path));
	Strlcat(path, ".revision", sizeof(path));

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
	if ((errno == ERANGE && (rev == AG_LONG_MAX || rev == AG_LONG_MIN)) ||
	    (rev > AG_INT_MAX || rev < AG_INT_MIN)) {
		AG_SetError("%s: out of range", path);
		return (-1);
	}
#endif
	*pRev = rev;
	return (0);
}

/* Write the working revision of an object. */
int
AG_RcsSetWorkingRev(AG_Object *ob, Uint rev)
{
	char path[AG_PATHNAME_MAX];
	FILE *f;

	AG_ObjectCopyFilename(ob, path, sizeof(path));
	Strlcat(path, ".revision", sizeof(path));
	
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
    char *name, char *type, Uint *repo_rev, Uint *working_rev)
{
	char *buf;
	int sum_match = 1;
	char *s;
	
	if (NC_Write(&rcs_client, "rcs-info\nobject-path=%s\n\n",
	    &objdir[1]) == -1) {
		return (AG_RCS_ERROR);
	}
	if (NC_Read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		return (AG_RCS_UNKNOWN);
	}

	buf = &rcs_client.read.buf[2];
	*repo_rev = 0;
	while ((s = Strsep(&buf, ":")) != NULL) {
		char *key = Strsep(&s, "=");
		char *val = Strsep(&s, "=");

		if (key == NULL || val == NULL)
			continue;
		
		if (strcmp(key, "d") == 0 &&
		    strcmp(val, digest) != 0) {
			sum_match = 0;
		} else if (strcmp(key, "r") == 0) {
			*repo_rev = (Uint)strtol(val, NULL, 10);
		} else if (strcmp(key, "t") == 0 && type != NULL) {
			Strlcpy(type, val, AG_OBJECT_HIER_MAX);
		} else if (strcmp(key, "n") == 0 && name != NULL) {
			Strlcpy(type, val, AG_OBJECT_NAME_MAX);
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
		AG_SetError("Working revision %u < %u", *working_rev,
		    *repo_rev);
		return (AG_RCS_ERROR);
	}
}

/* Import a new object into the repository. */
int
AG_RcsImport(AG_Object *ob)
{
	char buf[AG_BUFFER_MAX];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	Uint repo_rev, working_rev;
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
		/* AG_TextMsgFromError() */
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

	if (NC_Write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &objdir[1], ob->name, ob->cls->hier,
	    (Ulong)len, digest) == -1)
		goto fail_close;
	
	if (NC_Read(&rcs_client, 12) <= 2 ||
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
	if (NC_Read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}
#if 0
	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Object %s successfully imported to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (Ulong)wrote,
	      &rcs_client.read.buf[2]);
#endif
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
	char buf[AG_BUFFER_MAX];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	Uint repo_rev, working_rev;
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
		/* AG_TextMsgFromError() */
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

	if (NC_Write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &objdir[1], ob->name, ob->cls->hier,
	    (Ulong)len, digest) == -1)
		goto fail_close;
	
	if (NC_Read(&rcs_client, 12) <= 2 ||
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
	if (NC_Read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}
#if 0
	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Object %s successfully committed to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (Ulong)wrote,
	      &rcs_client.read.buf[2]);
#endif
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
	char type[AG_OBJECT_HIER_MAX];
	char objdir[AG_OBJECT_PATH_MAX];
	char objpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	Uint working_rev, repo_rev;
	NC_Result *res;
	size_t len;
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
	if (strcmp(type, ob->cls->hier) != 0) {
		AG_SetError(_("Repository has different object type (%s/%s)"),
		    type, ob->cls->hier);
		goto fail;
	}

	res = NC_QueryBinary(&rcs_client, "rcs-update\n"
			                   "object-path=%s\n"
			                   "object-name=%s\n"
			                   "object-type=%s\n"
					   "revision=%u\n",
					   &objdir[1], ob->name,
					   ob->cls->hier, repo_rev);
	if (res == NULL || res->argc < 1) {
		AG_SetError("RCS update error: %s", AG_GetError());
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
#if 0
	AG_TextTmsg(AG_MSG_INFO, 1000, "%s: r#%u -> r#%u (%lu bytes)", ob->name,
	    working_rev, repo_rev, (unsigned long)res->argv_len[0]);
#endif
	NC_FreeResult(res);
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
	NC_FreeResult(res);
fail:
	AG_RcsDisconnect();
	return (-1);
}

int
AG_RcsImportAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockVFS(obj);
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
	AG_UnlockVFS(obj);
	return (0);
fail:
	AG_UnlockVFS(obj);
	return (-1);
}

int
AG_RcsUpdateAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockVFS(obj);
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
	AG_UnlockVFS(obj);
	return (0);
fail:
	AG_UnlockVFS(obj);
	return (-1);
}

int
AG_RcsCommitAll(AG_Object *obj)
{
	AG_Object *cobj;

	AG_LockVFS(obj);
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
	AG_UnlockVFS(obj);
	return (0);
fail:
	AG_UnlockVFS(obj);
	return (-1);
}

int
AG_RcsGetLog(const char *objdir, AG_RCSLog *log)
{
	NC_Result *res;
	int i;

	res = NC_Query(&rcs_client, "rcs-log\n"
	                            "object-path=%s\n", &objdir[1]);
	if (res == NULL) {
		return (-1);
	}
	log->ents = NULL;
	log->nEnts = 0;

	for (i = 0; i < res->argc; i++) {
		char *s = res->argv[i];
		char *rev = Strsep(&s, ":");
		char *author = Strsep(&s, ":");
		char *type = Strsep(&s, ":");
		char *name = Strsep(&s, ":");
		char *sum = Strsep(&s, ":");
		char *msg = Strsep(&s, ":");
		AG_RCSLogEntry *lent;

		if (rev == NULL || author == NULL || sum == NULL)
			continue;
		
		log->ents = Realloc(log->ents, (log->nEnts+1) *
		                               sizeof(AG_RCSLogEntry));
		lent = &log->ents[log->nEnts++];
		lent->rev = Strdup(rev);
		lent->author = Strdup(author);
		lent->type = type!=NULL ? Strdup(type) : NULL;
		lent->name = name!=NULL ? Strdup(name) : NULL;
		lent->sum = Strdup(sum);
		lent->msg = msg!=NULL ? Strdup(msg) : NULL;
	}
	NC_FreeResult(res);
	return (0);
}

void
AG_RcsFreeLog(AG_RCSLog *log)
{
	int i;

	for (i = 0; i < log->nEnts; i++) {
		AG_RCSLogEntry *lent = &log->ents[i];

		Free(lent->rev);
		Free(lent->author);
		Free(lent->type);
		Free(lent->name);
		Free(lent->sum);
		Free(lent->msg);
	}
	log->ents = NULL;
	log->nEnts = 0;
}

int
AG_RcsGetList(AG_RCSList *list)
{
	NC_Result *res;
	int i;

	if ((res = NC_Query(&rcs_client, "rcs-list\n")) == NULL) {
		return (-1);
	}
	list->ents = NULL;
	list->nEnts = 0;
	for (i = 0; i < res->argc; i++) {
		char *s = res->argv[i];
		char *name = Strsep(&s, ":");
		char *rev = Strsep(&s, ":");
		char *author = Strsep(&s, ":");
		char *type = Strsep(&s, ":");
		AG_RCSListEntry *lent;

		if (name == NULL || rev == NULL || author == NULL ||
		    type == NULL)
			continue;
		
		list->ents = Realloc(list->ents, (list->nEnts+1) *
		                                 sizeof(AG_RCSListEntry));
		lent = &list->ents[list->nEnts++];
		lent->name = Strdup(name);
		lent->rev = Strdup(rev);
		lent->author = Strdup(author);
		lent->type = Strdup(type);
	}
	NC_FreeResult(res);
	return (0);
}

void
AG_RcsFreeList(AG_RCSList *list)
{
	int i;

	for (i = 0; i < list->nEnts; i++) {
		AG_RCSListEntry *lent = &list->ents[i];

		Free(lent->name);
		Free(lent->rev);
		Free(lent->author);
		Free(lent->type);
	}
	list->ents = NULL;
	list->nEnts = 0;
}

int
AG_RcsDelete(const char *path)
{
	NC_Result *res;

	if (AG_RcsConnect() == -1)
		return (-1);
	
	res = NC_Query(&rcs_client, "rcs-delete\n"
	                             "object-path=%s\n", path);
	if (res == NULL) {
		AG_RcsDisconnect();
		return (-1);
	}
	NC_FreeResult(res);
	AG_RcsDisconnect();
	return (0);
}

int
AG_RcsRename(const char *from, const char *to)
{
	NC_Result *res;

	if (AG_RcsConnect() == -1)
		return (-1);
	
	res = NC_Query(&rcs_client, "rcs-rename\n"
	                             "from-path=%s\n"
	                             "to-path=%s\n", from, to);
	if (res == NULL) {
		AG_RcsDisconnect();
		return (-1);
	}
	NC_FreeResult(res);
	AG_RcsDisconnect();
	return (0);
}

int
AG_RcsCheckout(void *vfsRoot, const char *path)
{
	char localpath[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	char name[AG_OBJECT_NAME_MAX];
	char type[AG_OBJECT_HIER_MAX];
	char *buf, *s;
	Uint rev = 0;
	AG_Object *obj;
	AG_ObjectClass *cls = NULL;

	if (AG_RcsConnect() == -1)
		goto fail;

	/* Fetch the object information from the repository. */
	if (NC_Write(&rcs_client, "rcs-info\nobject-path=%s\n\n", path)
	    == -1) {
		goto fail;
	}
	if (NC_Read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		AG_SetError("RCS info: %s", rcs_client.read.buf);
		goto fail;
	}
	buf = &rcs_client.read.buf[2];
	while ((s = Strsep(&buf, ":")) != NULL) {
		char *key = Strsep(&s, "=");
		char *val = Strsep(&s, "=");

		if (key == NULL || val == NULL)
			continue;
		
		switch (key[0]) {
		case 'd':
			Strlcpy(digest, val, AG_OBJECT_DIGEST_MAX);
			break;
		case 'r':
			rev = (Uint)strtol(val, NULL, 10);
			break;
		case 't':
			Strlcpy(type, val, AG_OBJECT_HIER_MAX);
			break;
		case 'n':
			Strlcpy(name, val, AG_OBJECT_NAME_MAX);
			break;
		}
	}
	if ((cls = AG_LoadClass(type)) == NULL)
		goto fail;

	/* Create the working copy if it does not exist. */
	localpath[0] = AG_PATHSEPCHAR;
	localpath[1] = '\0';
	Strlcat(localpath, path, sizeof(localpath));
	if ((obj = AG_ObjectFindS(vfsRoot, localpath)) == NULL) {
#if 0
		AG_TextTmsg(AG_MSG_INFO, 750,
		    _("Creating working copy of %s (%s)."),
		    name, type);
#endif
		obj = Malloc(cls->size);
		AG_ObjectInit(obj, cls);
		AG_ObjectSetNameS(obj, name);
		if (AG_ObjectAttachToNamed(vfsRoot, localpath, obj) == -1) {
			AG_ObjectDestroy(obj);
			goto fail;
		}
		AG_ObjectUnlinkDatafiles(obj);
		if (AG_ObjectSave(obj) == -1) {
#if 0
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", name, AG_GetError());
#endif
		}
		if (AG_RcsSetWorkingRev(obj, 0) == -1) {
			AG_ObjectDetach(obj);
			AG_ObjectDestroy(obj);
			goto fail;
		}
	} else {
		if (strcmp(type, obj->cls->hier) != 0) {
			AG_SetError(
			    "%s: existing object of different type (%s)",
			    localpath, obj->cls->hier);
			goto fail;
		}
	}

	if (AG_RcsUpdate(obj) == -1) {
		goto fail;
	}
#if 0
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Object %s updated."), name);
#endif
	AG_RcsDisconnect();
	return (0);
fail:
	AG_RcsDisconnect();
	if (cls != NULL) { AG_UnloadClass(cls); }
	return (-1);
}

#endif /* AG_NETWORK */
