/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Database-bound object class.
 */

#include <agar/core/core.h>

/* Create a new database-bound object. */
AG_DbObject *
AG_DbObjectNew(void)
{
	AG_DbObject *dbo;

	if ((dbo = TryMalloc(sizeof(AG_DbObject))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(dbo, &agDbObjectClass);
	return (dbo);
}

/* Load the given object's data from the database. */
int
AG_DbObjectLoad(void *obj, AG_Db *db, const char *key)
{
	AG_DbObject *dbo = obj;
	AG_DataSource *ds;
	AG_DbEntry dbe;

#ifdef AG_DEBUG
	if (!AG_OfClass(dbo, "AG_DbObject:*")) {
		AG_SetError("Object is not an AG_DbObject");
		return (-1);
	}
#endif
	if (AG_DbLookup(db, &dbe, key) == -1) {
		return (-1);
	}
	if ((ds = AG_OpenCore(dbe.data, dbe.dataSize)) == NULL) {
		return (-1);
	}
	if (AG_ObjectUnserialize(dbo, ds) == -1) {
		AG_CloseCore(ds);
		return (-1);
	}
	AG_CloseCore(ds);
	return (0);
}

/* Save a database-bound object's state. Overwrite data if it exists. */
int
AG_DbObjectSave(void *pDbo, AG_Db *db)
{
	AG_DbObject *dbo = pDbo;
	AG_DataSource *ds;
	AG_DbEntry dbe;

	if ((ds = AG_OpenAutoCore()) == NULL)
		return (-1);
	if (AG_ObjectSerialize(dbo, ds) == -1)
		goto fail;

	dbe.key = AGOBJECT(dbo)->name;
	dbe.keySize = strlen(AGOBJECT(dbo)->name)+1;
	dbe.data = AG_CORE_SOURCE(ds)->data;
	dbe.dataSize = AG_CORE_SOURCE(ds)->size;

	if (AG_DbPut(db, &dbe) == -1 ||
	    AG_DbSync(db) == -1) {
		goto fail;
	}
	AG_CloseAutoCore(ds);
	return (0);
fail:
	AG_CloseAutoCore(ds);
	return (-1);
}

/* Insert a new object to the database, fail if key is taken. */
int
AG_DbObjectInsert(AG_Db *db, void *pDbo)
{
	AG_DbObject *dbo = pDbo;

	if (AG_DbExists(db, AGOBJECT(dbo)->name)) {
		AG_SetError("Existing db object: %s", AGOBJECT(dbo)->name);
		return (-1);
	}
	return AG_DbObjectSave(dbo, db);
}

/* Remove a database object the database by name. */
int
AG_DbObjectDelete(AG_Db *db, const char *name)
{
	if (AG_DbDelete(db, name) == -1 ||
	    AG_DbSync(db) == -1) {
		return (-1);
	}
	return (0);
}

static void
Init(void *_Nonnull obj)
{
	/* Nothing to do */
}

AG_ObjectClass agDbObjectClass = {
	"AG_DbObject",
	sizeof(AG_DbObject),
	{ 0, 0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
