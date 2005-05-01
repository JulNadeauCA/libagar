/*	$Csoft$	*/

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

#include <compat/md5.h>
#include <compat/sha1.h>
#include <compat/rmd160.h>

#include <config/network.h>
#include <config/version.h>

#include <engine/widget/text.h>

#include "rcs.h"

char rcs_hostname[64] = "localhost";
char rcs_username[32] = "anonymous";
char rcs_password[32] = "";
u_int rcs_port = 6785;

#ifdef NETWORK

#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <qnet/qnet.h>
#include <qnet/client.h>

static struct client rcs_client;
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

	snprintf(port, sizeof(port), "%u", rcs_port);
	if (client_connect(&rcs_client, rcs_hostname, port,
	    rcs_username, rcs_password) == -1) {
		error_set("RCS connection: %s", qerror_get());
		return (-1);
	}
	return (0);
}

void
rcs_disconnect(void)
{
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
	if ((errno == ERANGE && (rev == LONG_MAX || rev == LONG_MIN)) ||
	    (rev > INT_MAX || rev < INT_MIN)) {
		error_set("%s: out of range", path);
		return (-1);
	}
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
rcs_status(struct object *ob, const char *obj_path, const char *digest,
    u_int *repo_rev, u_int *working_rev)
{
	enum rcs_status rv = RCS_UPTODATE;
	char *buf, *bufp;
	struct response *res;
	int sum_match = 1;
	int i;
	char *s;
	
	if (client_write(&rcs_client, "rcs-info\nobject-path=%s\n\n",
	    &obj_path[1]) == -1) {
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
	char obj_path[OBJECT_PATH_MAX];
	char save_path[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	struct response *res;
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;
	enum rcs_status status;

	if (object_copy_name(ob, obj_path, sizeof(obj_path)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);

	switch ((status = rcs_status(ob, obj_path, digest, &repo_rev,
	                             &working_rev))) {
	case RCS_ERROR:
		text_msg(MSG_ERROR, "%s", error_get());
		break;
	case RCS_UNKNOWN:
		break;
	default:
		text_tmsg(MSG_INFO, 3000,
		    _("Object %s is already in repository (r#%u):\n"
		      "Status: %s\n"),
		    ob->name, repo_rev, rcs_status_strings[status]);
		goto out;
	}
	
	object_copy_filename(ob, save_path, sizeof(save_path));

	/* TODO locking */
	if ((f = fopen(save_path, "r")) == NULL) {
		error_set("%s: %s", save_path, strerror(errno));
		rcs_disconnect();
		return (-1);
	}

	if (client_write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &obj_path[1], ob->name, ob->type,
	    (u_long)len, digest) == -1)
		goto fail;
	
	if (client_read(&rcs_client, 12) <= 2 ||
	    rcs_client.read.buf[0] != '0') {
		error_set(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			error_set(_("EOF from server"));
			goto fail;
		} else if (nw < 0) {
			error_set(_("Write error"));
			goto fail;
		}
		wrote += nw;
	}
	if (wrote < len) {
		error_set(_("Upload incomplete"));
		goto fail;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		error_set(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail;
	}

	text_tmsg(MSG_INFO, 4000,
	    _("Object %s successfully imported to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	rcs_set_working_rev(ob, 1);
	fclose(f);
out:
	rcs_disconnect();
	return (0);
fail:
	fclose(f);
	rcs_disconnect();
	return (-1);
}

/* Commit changes to an object. */
int
rcs_commit(struct object *ob)
{
	char buf[BUFSIZ];
	char obj_path[OBJECT_PATH_MAX];
	char save_path[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	struct response *res;
	size_t wrote = 0, len, rv;
	u_int repo_rev, working_rev;
	FILE *f;

	if (object_copy_name(ob, obj_path, sizeof(obj_path)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);

	switch (rcs_status(ob, obj_path, digest, &repo_rev, &working_rev)) {
	case RCS_LOCALMOD:
		break;
	case RCS_ERROR:
		text_msg(MSG_ERROR, "%s", error_get());
		break;
	case RCS_UNKNOWN:
		text_msg(MSG_ERROR,
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"), ob->name);
		goto out;
	case RCS_UPTODATE:
		text_tmsg(MSG_INFO, 3000,
		    _("Working copy of %s is up-to-date (r#%u)."), ob->name,
		        repo_rev);
		goto out;
	case RCS_DESYNCH:
		text_msg(MSG_ERROR,
		    _("Your copy of %s is outdated (working=r#%u, repo=r#%u)\n"
		      "Please do a RCS update prior to committing.\n"),
		       ob->name, working_rev, repo_rev);
		goto out;
	}
	
	object_copy_filename(ob, save_path, sizeof(save_path));

	/* TODO locking */
	if ((f = fopen(save_path, "r")) == NULL) {
		error_set("%s: %s", save_path, strerror(errno));
		rcs_disconnect();
		return (-1);
	}

	if (client_write(&rcs_client, "rcs-commit\n" 
	    "object-path=%s\n"
	    "object-name=%s\n"
	    "object-type=%s\n"
	    "object-size=%lu\n"
	    "object-digest=%s\n\n",
	    &obj_path[1], ob->name, ob->type,
	    (u_long)len, digest) == -1)
		goto fail;
	
	if (client_read(&rcs_client, 12) <= 2 ||
	    rcs_client.read.buf[0] != '0') {
		error_set(_("Server refused data: %s"),
		    &rcs_client.read.buf[2]);
		goto fail;
	}
	
	while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
		size_t nw;

		nw = write(rcs_client.sock, buf, rv);
		if (nw == 0) {
			error_set(_("EOF from server"));
			goto fail;
		} else if (nw < 0) {
			error_set(_("Write error"));
			goto fail;
		}
		wrote += nw;
	}
	if (wrote < len) {
		error_set(_("Upload incomplete"));
		goto fail;
	}
	if (client_read(&rcs_client, 32) < 1 ||
	    rcs_client.read.buf[0] != '0' ||
	    rcs_client.read.buf[1] == '\0') {
		error_set(_("Commit failed: %s"), rcs_client.read.buf);
		goto fail;
	}

	text_tmsg(MSG_INFO, 4000,
	    _("Object %s successfully committed to repository.\n"
	      "Size: %lu bytes\n"
	      "%s\n"),
	      ob->name, (u_long)wrote,
	      &rcs_client.read.buf[2]);

	rcs_set_working_rev(ob, repo_rev+1);
	fclose(f);
out:
	rcs_disconnect();
	return (0);
fail:
	fclose(f);
	rcs_disconnect();
	return (-1);
}

/* Update a working copy of an object from the repository. */
int
rcs_update(struct object *ob)
{
	char obj_path[OBJECT_PATH_MAX];
	char digest[OBJECT_DIGEST_MAX];
	size_t len;
	u_int working_rev, repo_rev;
	
	if (object_copy_name(ob, obj_path, sizeof(obj_path)) == -1 ||
	    object_copy_digest(ob, &len, digest) == -1)
		return (-1);

	if (rcs_connect() == -1)
		return (-1);
	
	switch (rcs_status(ob, obj_path, digest, &repo_rev, &working_rev)) {
	case RCS_ERROR:
		goto fail;
	case RCS_UNKNOWN:
		text_msg(MSG_ERROR,
		    _("Object %s does not exist on the repository.\n"
		      "Please use the RCS import command.\n"), ob->name);
		goto out;
	case RCS_UPTODATE:
		text_tmsg(MSG_INFO, 2000,
		    _("Working copy of %s is already up-to-date (r#%u)."),
		    ob->name, repo_rev);
		goto out;
	case RCS_LOCALMOD:
		text_msg(MSG_ERROR,
		    _("Working copy of %s was modified locally.\n"
		      "Please do a RCS commit prior to updating.\n"),
		       ob->name);
		goto out;
	case RCS_DESYNCH:
		break;
	}
	text_tmsg(MSG_INFO, 500, _("Updating %s..."), ob->name);
out:
	rcs_disconnect();
	return (0);
fail:
	rcs_disconnect();
	return (-1);
}

int
rcs_checkout(struct object *ob)
{
	error_set("Unimplemented");
	return (-1);
}
#endif /* NETWORK */
