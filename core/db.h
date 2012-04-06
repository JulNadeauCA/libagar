/*	Public domain	*/

#ifndef _AGAR_CORE_DB_H_
#define _AGAR_CORE_DB_H_
#include <agar/core/begin.h>

struct ag_db;

/* Type of database */
enum ag_db_type {
	AG_DB_DUMMY,	/* No-op */
	AG_DB_BTREE,	/* BDB: Sorted, balanced tree structure */
	AG_DB_HASH,	/* BDB: Extended Linear Hashing */
	AG_DB_LAST
};

typedef struct ag_db_entry {
	struct ag_db *db;			/* Back pointer to Db */
	void *key, *data;
	size_t keySize, dataSize;
} AG_DbEntry;

typedef int (*AG_DbIterateFn)(AG_DbEntry *, void *);

typedef struct ag_db_class {
	struct ag_object_class _inherit;
	const char *name;			/* Database method name */
	const char *descr;			/* Short description */
	enum ag_db_key_mode {
		AG_DB_KEY_DATA,		/* Variable-length data */
		AG_DB_KEY_NUMBER,	/* Logical record number */
		AG_DB_KEY_STRING	/* Legal C string */
	} keyMode;
	enum ag_db_record_mode {
		AG_DB_REC_VARIABLE,	/* Variable-length records */
		AG_DB_REC_FIXED		/* Fixed-length records */
	} recMode;
	int  (*open)(void *, const char *, Uint);
	void (*close)(void *);
	int  (*sync)(void *);
	int  (*exists)(void *, AG_DbEntry *);
	int  (*get)(void *, AG_DbEntry *);
	int  (*put)(void *, AG_DbEntry *);
	int  (*del)(void *, AG_DbEntry *);
	int  (*iterate)(void *, AG_DbIterateFn, void *);
} AG_DbClass;

#define AGDB_CLASS(db) ((AG_DbClass *)AGOBJECT(db)->cls)

typedef struct ag_db {
	struct ag_object _inherit;
	Uint flags;
#define AG_DB_OPEN	0x01		/* Database is open */
#define AG_DB_READONLY	0x02		/* Open in read-only mode */
} AG_Db;

#define AGDB(p) ((AG_Db *)(p))

__BEGIN_DECLS
extern AG_DbClass agDbClass;
extern AG_DbClass agDbHashClass;
extern AG_DbClass agDbBtreeClass;
extern AG_DbClass agDbMySQLClass;

AG_Db       *AG_DbNew(const char *);
int          AG_DbOpen(AG_Db *, const char *, Uint);
void         AG_DbClose(AG_Db *);
int          AG_DbSync(AG_Db *);

/* Test for existence of a key. */
static __inline__ int
AG_DbExists(AG_Db *db, AG_DbEntry *dbe)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->exists(db, dbe);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Get operation (BDB-style argument). */
static __inline__ int
AG_DbGet(AG_Db *db, AG_DbEntry *dbe)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->get(db, dbe);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Put operation (BDB-style argument). */
static __inline__ int
AG_DbPut(AG_Db *db, AG_DbEntry *dbe)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->put(db, dbe);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Delete operation (BDB-style argument). */
static __inline__ int
AG_DbDel(AG_Db *db, AG_DbEntry *dbe)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->del(db, dbe);
	AG_ObjectUnlock(db);
	return (rv);
}

/* Iterate over all entries. */
static __inline__ int
AG_DbIterate(AG_Db *db, AG_DbIterateFn fn, void *arg)
{
	AG_DbClass *dbc = AGDB_CLASS(db);
	int rv;

	AG_ObjectLock(db);
	rv = dbc->iterate(db, fn, arg);
	AG_ObjectUnlock(db);
	return (rv);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DB_H_ */
