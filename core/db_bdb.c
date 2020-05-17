/*
 * Copyright (c) 2012-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Berkeley DB database access.
 */

#include <db5/db.h>
#include <agar/core/core.h>

typedef struct ag_db_hash_bt {
	struct ag_db _inherit;
	DB *pDB;
} AG_DbHashBT;

static const struct {
	const char *_Nonnull name;
	Uint32 mask;
} bdbOptions[] = {
	{ "db-create",			DB_CREATE },
	{ "db-create-excl",		DB_EXCL },
	{ "db-auto-commit",		DB_AUTO_COMMIT },
	{ "db-threaded",		DB_THREAD },
	{ "db-truncate",		DB_TRUNCATE },
#ifdef DB_MULTIVERSION
	{ "db-multiversion",		DB_MULTIVERSION },
#endif
#ifdef DB_NOMMAP
	{ "db-no-mmap",			DB_NOMMAP },
#endif
#ifdef DB_READ_UNCOMMITTED
	{ "db-read-uncommitted",	DB_READ_UNCOMMITTED },
#endif
};
static const int bdbOptionCount = sizeof(bdbOptions)/sizeof(bdbOptions[0]);

static void
Init(void *obj)
{
	AG_DbHashBT *db = obj;
	int i;

	for (i = 0; i < bdbOptionCount; i++)
		AG_SetInt(db, bdbOptions[i].name, 0);
	
	AG_SetInt(db, "db-create", 1);
}

static int
Open(void *obj, const char *path, Uint flags)
{
	AG_DbHashBT *db = obj;
	Uint32 dbFlags = 0;
	int i, rv;
	DBTYPE dbtype;

	if (strcmp(AGDB_CLASS(db)->name, "hash") == 0) {
		dbtype = DB_HASH;
	} else {
		dbtype = DB_BTREE;
	}
	if ((rv = db_create(&db->pDB, NULL, 0)) != 0) {
		AG_SetError("db_create: %s", db_strerror(rv));
		return (-1);
	}
	for (i = 0; i < bdbOptionCount; i++) {
		if (AG_GetInt(db, bdbOptions[i].name))
			dbFlags |= bdbOptions[i].mask;
	}
	if (flags & AG_DB_READONLY) {
		dbFlags |= DB_RDONLY;
	}
	rv = db->pDB->open(db->pDB, NULL, path, NULL, dbtype, dbFlags, 0);
	if (rv != 0) {
		AG_SetError("db_open %s: %s", path, db_strerror(rv));
		return (-1);
	}
	return (0);
}

static void
Close(void *obj)
{
	AG_DbHashBT *db = obj;
	int rv;
	
	if ((rv = db->pDB->close(db->pDB, 0)) != 0)
		AG_Verbose("db_close: %s; ignoring\n", db_strerror(rv));
}

static int
Sync(void *obj)
{
	AG_DbHashBT *db = obj;
	int rv;
	
	if ((rv = db->pDB->sync(db->pDB, 0)) != 0) {
		AG_SetError("db_sync: %s", db_strerror(rv));
		return (-1);
	}
	return (0);
}

static int
Exists(void *obj, const AG_Dbt *key)
{
	AG_DbHashBT *db = obj;
	DBT dbKey;
	int rv;
	
	memset(&dbKey, 0, sizeof(DBT));
	dbKey.data = Malloc(key->size);
	memcpy(dbKey.data, key->data, key->size);
	dbKey.size = key->size;

#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR >= 6)
	rv = db->pDB->exists(db->pDB, NULL, &dbKey, 0);
#else
	{
		DBT dbVal;
	
		memset(&dbVal, 0, sizeof(DBT));
		rv = db->pDB->get(db->pDB, NULL, &dbKey, &dbVal, 0);
		Free(dbVal.data);
	}
#endif
	Free(dbKey.data);
	return (rv != DB_NOTFOUND);
}

static int
Get(void *obj, const AG_Dbt *key, AG_Dbt *val)
{
	AG_DbHashBT *db = obj;
	DBT dbKey, dbVal;
	int rv;

	memset(&dbKey, 0, sizeof(DBT));
	if ((dbKey.data = TryMalloc(key->size)) == NULL) {
		return (-1);
	}
	memcpy(dbKey.data, key->data, key->size);
	dbKey.size = key->size;
	
	memset(&dbVal, 0, sizeof(DBT));
	if ((rv = db->pDB->get(db->pDB, NULL, &dbKey, &dbVal, 0)) != 0) {
		AG_SetError("db_get: %s", db_strerror(rv));
	}
	Free(dbKey.data);
	val->data = dbVal.data;
	val->size = dbVal.size;
	return (rv != 0) ? -1 : 0;
}
		
static int
Put(void *_Nonnull obj, const AG_Dbt *_Nonnull key, const AG_Dbt *_Nonnull val)
{
	AG_DbHashBT *db = obj;
	DBT dbKey, dbVal;
	int rv;

	memset(&dbKey, 0, sizeof(DBT));
	if ((dbKey.data = TryMalloc(key->size)) == NULL) {
		return (-1);
	}
	memcpy(dbKey.data, key->data, key->size);
	dbKey.size = key->size;

	memset(&dbVal, 0, sizeof(DBT));
	if ((dbVal.data = TryMalloc(val->size)) == NULL) {
		Free(dbKey.data);
		return (-1);
	}
	memcpy(dbVal.data, val->data, val->size);
	dbVal.size = val->size;

	if ((rv = db->pDB->put(db->pDB, NULL, &dbKey, &dbVal, 0)) != 0)
		AG_SetError("db_put: %s", db_strerror(rv));

	Free(dbKey.data);
	Free(dbVal.data);
	return (rv != 0) ? -1 : 0;
}

static int
Del(void *_Nonnull obj, const AG_Dbt *_Nonnull key)
{
	AG_DbHashBT *db = obj;
	DBT dbKey;
	int rv;

	memset(&dbKey, 0, sizeof(DBT));
	if ((dbKey.data = TryMalloc(key->size)) == NULL) {
		return (-1);
	}
	memcpy(dbKey.data, key->data, key->size);
	dbKey.size = key->size;

	if ((rv = db->pDB->del(db->pDB, NULL, &dbKey, 0)) != 0) {
		AG_SetError("DB Delete: %s", db_strerror(rv));
	}
	Free(dbKey.data);
	return (rv != 0) ? -1 : 0;
}

static int
Iterate(void *_Nonnull obj, AG_DbIterateFn fn, void *_Nullable arg)
{
	AG_DbHashBT *db = obj;
	DBC *c;
	DBT dbk, dbv;
	int rv;

	db->pDB->cursor(db->pDB, NULL, &c, 0);
	memset(&dbk, 0, sizeof(DBT));
	memset(&dbv, 0, sizeof(DBT));
	while ((rv = c->c_get(c, &dbk, &dbv, DB_NEXT)) == 0) {
		AG_Dbt key, val;

		key.data = dbk.data;
		key.size = dbk.size;
		val.data = dbv.data;
		val.size = dbv.size;

		if (fn(&key, &val, arg) == -1) {
			c->c_close(c);
			return (-1);
		}
	}
	c->c_close(c);

	if (rv != DB_NOTFOUND) {
		AG_SetError("c_get: %s", db_strerror(rv));
		return (-1);
	} else {
		return (0);
	}
}

AG_DbClass agDbHashClass = {
	{
		"AG_Db:AG_DbHash",
		sizeof(AG_DbHashBT),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	"hash",
	N_("Extended Linear Hashing"),
	AG_DB_KEY_DATA,		/* Key is variable data */
	AG_DB_REC_VARIABLE,	/* Variable-sized records */
	Open,
	Close,
	Sync,
	Exists,
	Get,
	Put,
	Del,
	Iterate
};
AG_DbClass agDbBtreeClass = {
	{
		"AG_Db:AG_DbBtree",
		sizeof(AG_DbHashBT),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	"btree",
	N_("Sorted, Balanced Tree Structure"),
	AG_DB_KEY_DATA,		/* Key is variable data */
	AG_DB_REC_VARIABLE,	/* Variable-sized records */
	Open,
	Close,
	Sync,
	Exists,
	Get,
	Put,
	Del,
	Iterate
};
