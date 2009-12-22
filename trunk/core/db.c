/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Simple key/value database class.
 */

#include <config/have_db4.h>
#include <core/core.h>

#ifdef HAVE_DB4
#include <db4/db.h>
#endif

/* Create a new database. */
AG_Db *
AG_DbNew(enum ag_db_type type)
{
	AG_Db *db;

	if ((db = TryMalloc(sizeof(AG_Db))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(db, &agDbClass);
	db->type = type;
	return (db);
}

/* Create a new DB4 database. */
AG_Db *
AG_DbNewDB4(const char *path, enum ag_db4_type db4_type, Uint32 db4_flags)
{
#ifdef HAVE_DB4
	AG_Db *db;
	int rv;
	DB *dbp;
	
	if ((db = AG_DbNew(AG_DB_DB4)) == NULL) {
		return (NULL);
	}
	if ((rv = db_create(&dbp, NULL, 0)) != 0) {
		AG_SetError("db_create: %s", db_strerror(rv));
		AG_ObjectDestroy(db);
		return (NULL);
	}
	db->db4 = (void *)dbp;
	rv = ((DB *)db->db4)->open((DB *)db->db4, NULL, path,
	    (DBTYPE)db4_type, db4_flags, 0);
	if (rv != 0) {
		AG_SetError("%s: %s", path, db_strerror(rv));
		return (NULL);
	}
	return (db);
#else
	AG_SetError("Berkeley DB4 support was not compiled in");
	return (NULL);
#endif /* HAVE_DB4 */
}

/* Return the list of keys (as strings) in a given database. */
AG_List *
AG_DbListKeys(AG_Db *db)
{
	AG_List *L;

	if ((L = AG_ListNew()) == NULL)
		return (NULL);

	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		DBC *c;
		DBT key, data;
		int rv;

		((DB *)db->db4)->cursor((DB *)db->db4, NULL, &c, 0);
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));
		while ((rv = c->c_get(c, &key, &data, DB_NEXT)) == 0) {
			AG_Variable Vnew;
			AG_InitString(&Vnew, key.data);
			AG_ListAppend(L, &Vnew);
		}
		if (rv != DB_NOTFOUND) {
			AG_SetError("Retrieving db keys: %s", db_strerror(rv));
			AG_ListDestroy(L);
			return (NULL);
		}
		c->c_close(c);
		break;
	}
#endif
	default:
		break;
	}
	return (L);
}

/* Returns whether a key exists (string). */
int
AG_DbExists(AG_Db *db, const char *s)
{
	return AG_DbExistsDK(db, (void *)s, strlen(s)+1);
}

/* Returns whether a key exists (data). */
int
AG_DbExistsDK(AG_Db *db, void *keyData, size_t keySize)
{
	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		DBT key, data;
		int rv;

		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));
		key.data = keyData;
		key.size = keySize;
		if ((rv = ((DB *)db->db4)->get((DB *)db->db4, NULL,
		    &key, &data, 0)) != DB_NOTFOUND) {
			return (1);
		}
		break;
	}
#endif
	default:
		break;
	}
	return (0);
}

/* Lookup a database entry by key (string). */
int
AG_DbLookup(AG_Db *db, AG_DbEntry *dbe, const char *s)
{
	return AG_DbLookupDK(db, dbe, (void *)s, strlen(s)+1);
}

/* Lookup a database entry by key (data). */
int
AG_DbLookupDK(AG_Db *db, AG_DbEntry *dbe, void *keyData, size_t keySize)
{
	dbe->db = db;

	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		DBT key, data;
		int rv;

		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));
		key.data = keyData;
		key.size = keySize;
		if ((rv = ((DB *)db->db4)->get((DB *)db->db4, NULL,
		    &key, &data, 0)) != 0) {
			AG_SetError("DB Lookup: %s", db_strerror(rv));
			return (-1);
		}
		dbe->key = (void *)key.data;
		dbe->keySize = (size_t)key.size;
		dbe->data = (void *)data.data;
		dbe->dataSize = (size_t)data.size;
		break;
	}
#endif
	default:
		AG_SetError("Unsupported operation");
		return (-1);
	}
	return (0);
}

/* Put an entry onto the database, overwrite if exists. */
int
AG_DbPut(AG_Db *db, AG_DbEntry *dbe)
{
	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		DBT key, data;
		int rv;

		memset(&key, 0, sizeof(DBT));
		key.data = dbe->key;
		key.size = dbe->keySize;
		memset(&data, 0, sizeof(DBT));
		data.data = dbe->data;
		data.size = dbe->dataSize;
		if ((rv = ((DB *)db->db4)->put((DB *)db->db4, NULL,
		    &key, &data, 0)) != 0) {
			AG_SetError("DB Put: %s", db_strerror(rv));
			return (-1);
		}
		break;
	}
#endif
	default:
		AG_SetError("Unsupported operation");
		return (-1);
	}
	return (0);
}

/* Sync the database */
int
AG_DbSync(AG_Db *db)
{
	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		int rv;
		if ((rv = ((DB *)db->db4)->sync((DB *)db->db4, 0)) != 0) {
			AG_SetError("DB Sync: %s", db_strerror(rv));
			return (-1);
		}
		break;
	}
#endif
	default:
		break;
	}
	return (0);
}

/* Remove an entry from the database by key (string). */
int
AG_DbDelete(AG_Db *db, const char *key)
{
	return AG_DbDeleteDK(db, (void *)key, strlen(key)+1);
}

/* Remove an entry from the database by key (data). */
int
AG_DbDeleteDK(AG_Db *db, void *keyData, size_t keySize)
{
	switch (db->type) {
#ifdef HAVE_DB4
	case AG_DB_DB4: {
		DBT key, data;
		int rv;

		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));
		key.data = keyData;
		key.size = keySize;
		if ((rv = ((DB *)db->db4)->get((DB *)db->db4, NULL,
		    &key, &data, 0)) == DB_NOTFOUND) {
			AG_SetError("DB Delete: No such entry");
			return (-1);
		}
		if ((rv = ((DB *)db->db4)->del((DB *)db->db4, NULL, &key, 0))
		    != 0) {
			AG_SetError("DB Delete: %s", db_strerror(rv));
			return (-1);
		}
		break;
	}
#endif /* HAVE_DB4 */
	default:
		AG_SetError("Unsupported operation");
		return (-1);
	}
	return (0);
}

static void
Init(void *obj)
{
	AG_Db *db = obj;
	db->type = AG_DB_DUMMY;
}

static void
Destroy(void *obj)
{
#ifdef HAVE_DB4
	AG_Db *db = obj;
	int rv;

	if ((rv = ((DB *)db->db4)->close((DB *)db->db4, 0)) != 0)
		AG_FatalError("Closing database: %s", db_strerror(rv));
#endif
}

AG_ObjectClass agDbClass = {
	"AG_Db",
	sizeof(AG_Db),
	{ 0, 0 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
