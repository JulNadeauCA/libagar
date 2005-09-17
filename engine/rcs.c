/*	$Csoft: rcs.c,v 1.12 2005/06/16 02:26:29 vedge Exp $	*/

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

char rcs_hostname[64] = "localhost";
char rcs_username[32] = "anonymous";
char rcs_password[32] = "";
u_int rcs_port = 6785;
int rcs = 0;

#ifdef NETWORK

#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <qnet/qnet.h>
#include <qnet/client.h>

static struct client rcs_client;
static int connected = 0;
const char *rcs_status_strings[] = {
	N_("Error"),
	N_("Not in repository"),
	N_("Up-to-date"),
	N_("Modified locally"),
	N_("Desynchronized")
};

void
rcs_init(void)
{
	client_init(&rcs_client, "agar", VERSION);
}

void
rcs_destroy(void)
{
	client_destroy(&rcs_client);
}

int
rcs_connect(void)
{
	char port[12];

	if (++connected == 1) {
		snprintf(port, sizeof(port), "%u", rcs_port);
		if (client_connect(&rcs_client, rcs_hostname, port,
		    rcs_username, rcs_password) == -1) {
			error_set("RCS connection: %s", qerror_get());
			return (-1);
		}
	}
	return (0);
}

void
rcs_disconnect(void)
{
	if (--connected == 0)
		client_disconnect(&rcs_client);
}

/* Get the working revision of an object. */
int
rcs_get_working_rev(struct object *ob, u_int *pRev)
{
	char path[MAXPATHLEN];
	char buf[12];
	size_t rv;
	long rev;
	char *ep;
	FILE *f;

	object_copy_filename(ob, path, sizeof(path));
	strlcat(path, ".revision", sizeof(path));

	/* TODO locking */
	if ((f = fopen(path, "r")) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	rv = fread(buf, 1, sizeof(buf), f);
	buf[rv-1] = '\0';
	fclose(f);

	errno = 0;
	rev = strtol(buf, &ep, 10);
	if (buf[0] == '\0' || *ep != '\0') {
		error_set("%s: not a number", path);
		return (-1);
	}
#if 0
	if ((errno == ERANGE && (rev == LONG_MAX || rev == LONG_MIN)) ||
	    (rev > INT_MAX || rev < INT_MIN)) {
		error_set("%s: out of range", path);
		return (-1);
	}
#endif
	*pRev = rev;
	return (0);
}

/* Write the working revision of an object. */
int
rcs_set_working_rev(struct object *ob, u_int rev)
{
	char path[MAXPATHLEN];
	FILE *f;

	object_copy_filename(ob, path, sizeof(path));
	strlcat(path, ".revision", sizeof(path));
	
	/* TODO locking */
	if ((f = fopen(path, "w")) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	fprintf(f, "%u\n", rev);
	fclose(f);

	return (0);
}

/* Obtain the RCS status of an object. */
enum rcs_status
rcs_status(struct object *ob, const char *objdir, const char *digest,
    char *name, char *type, u_int *repo_rev, u_int *working_rev)
{
	enum rcs_status rv = RCS_UPTODATE;
	char *buf, *bufp;
	int sum_match = 1;
	int i;
	char *s;
	
	if (client_write(&rcs_client, "rcs-info\nobject-path=%s\n\n",
	    &objdir[1]) == -1) {
		error_set("%s", qerror_get());
		return (RCS_ERROR);
	}
	if (client_read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		return (RCS_UNKNOWN);
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
			strlcpy(type, val, OBJECT_TYPE_MAX);
		} else if (strcmp(key, "n") == 0 && name != NULL) {
			strlcpy(type, val, OBJECT_NAME_MAX);
		}
	}

	if (rcs_get_working_rev(ob, working_rev) == -1)
		return (RCS_ERROR);

	if (*working_rev == *repo_rev) {
		if (sum_match) {
			return (RCS_UPTODATE);
		} else {
			return (RCS_LOCALMOD);
		}
	} else if (*working_rev < *repo_rev) {
		return (RCS_DESYNCH);
	} else {
		error_set("Working revision %u < %u", *working_rev, *repo_rev);
		return (RCS_ERROR);
	}
}

/* Import a new object into the repository. */
int
rcs_import(struct object *ob)
{
	char buf[BUFSIZ];
	char objdir[OBJECT_PATH_MAX];
	char objpath[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;
	enum rcs_status status;

	if (object_copy_name(ob, objdir, sizeof(objdir)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1 ||
	    object_copy_filename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);

	switch ((status = rcs_status(ob, objdir, digest, NULL, NULL, &repo_rev,
	                             &working_rev))) {
	case RCS_ERROR:
		text_msg(MSG_ERROR, "%s", error_get());
		break;
	case RCS_UNKNOWN:
		break;
	default:
		error_set(_("Object %s is already in repository (r#%u):\n"
		            "Status: %s\n"),
		    ob->name, repo_rev, rcs_status_strings[status]);
		goto fail;
	}
	
	/* TODO locking */
	if ((f = fopen(objpath, "r")) == NULL) {
		error_set("%s: %s", objpath, strerror(errno));
		rcs_disconnect();
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
		error_set(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail_close;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			error_set(_("EOF from server"));
			goto fail_close;
		} else if (nw < 0) {
			error_set(_("Write error"));
			goto fail_close;
		}
		wrote += nw;
	}
	if (wrote < len) {
		error_set(_("Upload incomplete"));
		goto fail_close;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		error_set(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}

	text_tmsg(MSG_INFO, 4000,
	    _("Object %s successfully imported to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	rcs_set_working_rev(ob, 1);
	fclose(f);
	rcs_disconnect();
	return (0);
fail_close:
	fclose(f);
fail:
	rcs_disconnect();
	return (-1);
}

/* Commit changes to an object. */
int
rcs_commit(struct object *ob)
{
	char buf[BUFSIZ];
	char objdir[OBJECT_PATH_MAX];
	char objpath[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;

	if (object_copy_name(ob, objdir, sizeof(objdir)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1 ||
	    object_copy_filename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);

	switch (rcs_status(ob, objdir, digest, NULL, NULL, &repo_rev,
	                   &working_rev)) {
	case RCS_LOCALMOD:
		break;
	case RCS_ERROR:
		text_msg(MSG_ERROR, "%s", error_get());
		break;
	case RCS_UNKNOWN:
		error_set(
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"),
		      ob->name);
		goto fail;
	case RCS_UPTODATE:
		error_set(
		    _("Working copy of %s is up-to-date (r#%u)."),
		    ob->name, repo_rev);
		goto fail;
	case RCS_DESYNCH:
		error_set(
		    _("Your copy of %s is outdated (working=r#%u, repo=r#%u)\n"
		      "Please do a RCS update prior to committing.\n"),
		       ob->name, working_rev, repo_rev);
		goto fail;
	}
	
	/* TODO locking */
	if ((f = fopen(objpath, "r")) == NULL) {
		error_set("%s: %s", objpath, strerror(errno));
		rcs_disconnect();
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
		error_set(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail_close;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			error_set(_("EOF from server"));
			goto fail_close;
		} else if (nw < 0) {
			error_set(_("Write error"));
			goto fail_close;
		}
		wrote += nw;
	}
	if (wrote < len) {
		error_set(_("Upload incomplete"));
		goto fail_close;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		error_set(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail_close;
	}

	text_tmsg(MSG_INFO, 4000,
	    _("Object %s successfully committed to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	rcs_set_working_rev(ob, repo_rev+1);
	fclose(f);
	rcs_disconnect();
	return (0);
fail_close:
	fclose(f);
fail:
	rcs_disconnect();
	return (-1);
}

/* Update a working copy of an object from the repository. */
int
rcs_update(struct object *ob)
{
	char buf[BUFSIZ];
	char type[OBJECT_TYPE_MAX];
	char objdir[OBJECT_PATH_MAX];
	char objpath[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	u_int working_rev, repo_rev;
	struct response *res;
	size_t len, wrote = 0;
	FILE *f;
	
	if (object_copy_name(ob, objdir, sizeof(objdir)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1 ||
	    object_copy_filename(ob, objpath, sizeof(objpath)) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);
	
	switch (rcs_status(ob, objdir, digest, NULL, type, &repo_rev,
	                   &working_rev)) {
	case RCS_ERROR:
		goto fail;
	case RCS_UNKNOWN:
		error_set(
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"), ob->name);
		goto fail;
	case RCS_UPTODATE:
		error_set(
		    _("Working copy of %s is already up-to-date (r#%u)."),
		    ob->name, repo_rev);
		goto fail;
	case RCS_LOCALMOD:
#if 1
		/* TODO move working version */
		error_set(
		    _("Working copy of %s was modified locally.\n"
		      "Please do a RCS commit prior to updating.\n"),
		       ob->name);
		goto fail;
#endif
	case RCS_DESYNCH:
		break;
	}
	if (strcmp(type, ob->type) != 0) {
		error_set(_("Repository has different object type (%s/%s)"),
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
		error_set("RCS update error: %s", qerror_get());
		goto fail;
	}

	/* TODO locking, backup */
	if ((f = fopen(objpath, "w")) == NULL) {
		error_set("%s: %s", objpath, strerror(errno));
		goto fail_res;
	}
	if (fwrite(res->argv[0], 1, res->argv_len[0], f) < res->argv_len[0]) {
		error_set("%s: write error", objpath);
		fclose(f);
		goto fail_res;
	}
	fclose(f);
	rcs_set_working_rev(ob, repo_rev);
	text_tmsg(MSG_INFO, 1000, "%s: r#%u -> r#%u (%lu bytes)", ob->name,
	    working_rev, repo_rev, (unsigned long)res->argv_len[0]);
	response_free(res);
	rcs_disconnect();

	/*
	 * Load the generic part of the object since the object manager
	 * only does a page-in.
	 */
	if (object_load(ob) == -1 ||
	    object_save(ob) == -1) {
		error_set("%s: %s", ob->name, error_get());
		return (-1);
	}

	return (0);
fail_res:
	response_free(res);
fail:
	rcs_disconnect();
	return (-1);
}

int
rcs_import_all(struct object *obj)
{
	struct object *cobj;

	lock_linkage();
	if (rcs_import(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (rcs_import_all(cobj) == -1)
			goto fail;
	}
	unlock_linkage();
	return (0);
fail:
	unlock_linkage();
	return (-1);
}

int
rcs_update_all(struct object *obj)
{
	struct object *cobj;

	lock_linkage();
	if (rcs_update(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (rcs_update_all(cobj) == -1)
			goto fail;
	}
	unlock_linkage();
	return (0);
fail:
	unlock_linkage();
	return (-1);
}

int
rcs_commit_all(struct object *obj)
{
	struct object *cobj;

	lock_linkage();
	if (rcs_commit(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (rcs_commit_all(cobj) == -1)
			goto fail;
	}
	unlock_linkage();
	return (0);
fail:
	unlock_linkage();
	return (-1);
}

int
rcs_log(const char *objdir, struct tlist *tl)
{
	struct response *res;
	int i;

	res = client_query(&rcs_client, "rcs-log\n"
				        "object-path=%s\n", &objdir[1]);
	if (res == NULL) {
		error_set("%s", qerror_get());
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
		struct object_type *t;
		SDL_Surface *icon = NULL;

		if (rev == NULL || author == NULL || sum == NULL)
			continue;
		
		for (t = &typesw[0]; t < &typesw[ntypesw]; t++) {
			if (strcmp(type, t->type) == 0) {
				icon = t->icon >= 0 ? ICON(t->icon) : NULL;
				break;
			}
		}
		tlist_insert(tl, icon, "[#%s.%s] %s", rev, author, msg);
	}
	response_free(res);
	return (0);
}

int
rcs_list(struct tlist *tl)
{
	struct response *res;
	struct tlist_item *it;
	int i;

	if ((res = client_query(&rcs_client, "rcs-list\n")) == NULL) {
		error_set("%s", qerror_get());
		return (-1);
	}
	tlist_clear_items(tl);
	it = tlist_insert(tl, NULL, "/");
	it->flags |= TLIST_HAS_CHILDREN;
	it->class = "object";
	it->depth = 0;

	for (i = 0; i < res->argc; i++) {
		char *s = res->argv[i];
		char *name = strsep(&s, ":");
		char *rev = strsep(&s, ":");
		char *author = strsep(&s, ":");
		char *type = strsep(&s, ":");
		struct object_type *t;
		SDL_Surface *icon = NULL;
		struct tlist_item *it;
		char *c;
		int depth = 0;

		if (name == NULL || rev == NULL || author == NULL ||
		    type == NULL)
			continue;

		for (t = &typesw[0]; t < &typesw[ntypesw]; t++) {
			if (strcmp(type, t->type) == 0) {
				icon = t->icon >= 0 ? ICON(t->icon) : NULL;
				break;
			}
		}
		for (c = &name[0]; *c != '\0'; c++) {
			if (*c == '/')
				depth++;
		}

		it = tlist_insert(tl, icon, "%s", &name[1]);
		it->class = "object";
		it->depth = depth;
	}
out:
	tlist_restore_selections(tl);
	response_free(res);
	return (0);
}

int
rcs_checkout(const char *path)
{
	char localpath[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	char *buf, *s;
	int i;
	u_int rev = 0;
	struct object *obj;
	struct object_type *t;

	if (rcs_connect() == -1)
		goto fail;

	/* Fetch the object information from the repository. */
	if (client_write(&rcs_client, "rcs-info\nobject-path=%s\n\n", path)
	    == -1) {
		error_set("%s", qerror_get());
		goto fail;
	}
	if (client_read(&rcs_client, 16) <= 2 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		error_set("RCS info: %s", rcs_client.read.buf);
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
			strlcpy(digest, val, OBJECT_DIGEST_MAX);
			break;
		case 'r':
			rev = (u_int)strtol(val, NULL, 10);
			break;
		case 't':
			strlcpy(type, val, OBJECT_TYPE_MAX);
			break;
		case 'n':
			strlcpy(name, val, OBJECT_NAME_MAX);
			break;
		}
	}
	for (t = &typesw[0]; t < &typesw[ntypesw]; t++) {
		if (strcmp(type, t->type) == 0)
			break;
	}
	if (t == &typesw[ntypesw]) {
		error_set(_("Unimplemented object type: %s"), type);
		goto fail;
	}

	/* Create the working copy if it does not exist. */
	localpath[0] = '/';
	localpath[1] = '\0';
	strlcat(localpath, path, sizeof(localpath));
	if ((obj = object_find(localpath)) == NULL) {
		text_tmsg(MSG_INFO, 750, _("Creating working copy of %s (%s)."),
		    name, type);

		obj = Malloc(t->size, M_OBJECT);
		if (t->ops->init != NULL) {
			t->ops->init(obj, name);
		} else {
			object_init(obj, t->type, name, NULL);
		}

		if (object_attach_path(localpath, obj) == -1) {
			object_destroy(obj);
			goto fail;
		}
		object_unlink_datafiles(obj);
		if (object_save(obj) == -1) {
			text_msg(MSG_ERROR, "%s: %s", name, error_get());
		}
		if (rcs_set_working_rev(obj, 0) == -1) {
			object_detach(obj);
			object_destroy(obj);
			goto fail;
		}
	} else {
		if (strcmp(type, obj->type) != 0) {
			error_set("%s: existing object of different type (%s)",
			    localpath, obj->type);
			goto fail;
		}
	}

	if (rcs_update(obj) == -1) {
		goto fail;
	}
	text_tmsg(MSG_INFO, 1000, _("Object %s updated."), name);
	rcs_disconnect();
	return (0);
fail:
	rcs_disconnect();
	return (-1);
}

#endif /* NETWORK */
