/*
 * Copyright (c) 2012-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/have_db4.h>
#include <agar/config/have_db5.h>
#include <agar/core/core.h>

/* Create a new database handle for the given database backend. */
AG_Db *_Nullable
AG_DbNew(const char *_Nonnull backend)
{
	AG_Db *db;
	AG_DbClass *dbc = NULL;

#if defined(HAVE_DB4) || defined(HAVE_DB5)
	if (strcmp(backend, "hash")) {
		dbc = &agDbHashClass;
	} else if (strcmp(backend, "btree")) {
		dbc = &agDbBtreeClass;
	}
#endif
	if (dbc == NULL) {
		AG_SetError("No such database backend: %s", backend);
		return (NULL);
	}
	if ((db = TryMalloc(sizeof(AG_Db))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(db, dbc);
	return (db);
}

/* Open a database. */
int
AG_DbOpen(AG_Db *db, const char *path, Uint flags)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	if (db->flags & AG_DB_OPEN) {
		AG_SetError(_("Database is already open"));
		goto fail;
	}
	rv = (dbc->open != NULL) ? dbc->open(db, path, flags) : 0;
	if (rv != 0) {
		goto fail;
	}
	db->flags |= AG_DB_OPEN;
	AG_ObjectUnlock(db);
	return (0);
fail:
	AG_ObjectUnlock(db);
	return (-1);
}

/* Close a database. */
void
AG_DbClose(AG_Db *db)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	
	AG_ObjectLock(db);
	if (db->flags & AG_DB_OPEN) {
		if (dbc->close != NULL) {
			dbc->close(db);
		}
		db->flags &= ~(AG_DB_OPEN);
	}
	AG_ObjectUnlock(db);
}

/* Synchronize a database. */
int
AG_DbSync(AG_Db *db)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;
	
	AG_ObjectLock(db);
	rv = (dbc->sync != NULL) ? dbc->sync(db) : 0;
	AG_ObjectUnlock(db);
	return (rv);
}

/* Test for existence of a key. */
int
AG_DbExists(AG_Db *db, AG_Dbt *key)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->exists(db, key);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Retrieve a database entry. */
int
AG_DbGet(AG_Db *_Nonnull db, const AG_Dbt *_Nonnull key, AG_Dbt *_Nonnull val)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->get(db, key, val);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Write a database entry. */
int
AG_DbPut(AG_Db *db, const AG_Dbt *key, const AG_Dbt *val)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->put(db, key, val);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Delete a database entry. */
int
AG_DbDel(AG_Db *_Nonnull db, const AG_Dbt *_Nonnull key)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->del(db, key);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Iterate over all entries. */
int
AG_DbIterate(AG_Db *db, AG_DbIterateFn fn, void *arg)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->iterate(db, fn, arg);
	AG_ObjectUnlock(db);
	return (rv);
}

static void
Init(void *_Nonnull obj)
{
	AG_Db *db = obj;

	db->flags = 0;
}

AG_DbClass agDbClass = {
	{
		"AG_Db",
		sizeof(AG_Db),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	"dummy",
	N_("Dummy database backend"),
	AG_DB_KEY_DATA,		/* Key is variable data */
	AG_DB_REC_VARIABLE,	/* Variable-sized records */
	NULL,			/* open */
	NULL,			/* close */
	NULL,			/* sync */
	NULL,			/* exists */
	NULL,			/* get */
	NULL,			/* put */
	NULL,			/* del */
	NULL			/* iterate */
};

#endif /* AG_SERIALIZATION */
