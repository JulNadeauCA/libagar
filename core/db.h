/*	Public domain	*/

#ifndef _AGAR_CORE_DB_H_
#define _AGAR_CORE_DB_H_
#include <agar/core/begin.h>

struct ag_db;

/* Database data item (e.g., key or value) */
typedef struct ag_dbt {
	void *_Nonnull data;
	AG_Size        size;
} AG_Dbt;

typedef int (*AG_DbIterateFn)(const AG_Dbt *_Nonnull,
                              const AG_Dbt *_Nonnull,
			      void         *_Nullable arg);

typedef struct ag_db_class {
	struct ag_object_class _inherit;
	const char *_Nonnull name;	/* Database method name */
	const char *_Nonnull descr;	/* Short description */
	enum ag_db_key_mode {
		AG_DB_KEY_DATA,		/* Variable-length data */
		AG_DB_KEY_NUMBER,	/* Logical record number */
		AG_DB_KEY_STRING	/* Legal C string */
	} keyMode;
	enum ag_db_record_mode {
		AG_DB_REC_VARIABLE,	/* Variable-length records */
		AG_DB_REC_FIXED		/* Fixed-length records */
	} recMode;
	int  (*_Nullable open)(void *_Nonnull, const char *_Nonnull, Uint);
	void (*_Nullable close)(void *_Nonnull);
	int  (*_Nullable sync)(void *_Nonnull);
	int  (*_Nonnull  exists)(void *_Nonnull, const AG_Dbt *_Nonnull);
	int  (*_Nonnull  get)(void *_Nonnull, const AG_Dbt *_Nonnull,
	                      AG_Dbt *_Nonnull);
	int  (*_Nonnull  put)(void *_Nonnull, const AG_Dbt *_Nonnull,
	                      const AG_Dbt *_Nonnull);
	int  (*_Nonnull  del)(void *_Nonnull, const AG_Dbt *_Nonnull);
	int  (*_Nullable iterate)(void *_Nonnull, _Nonnull AG_DbIterateFn,
	                          void *_Nullable);
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

AG_Db *_Nullable AG_DbNew(const char *_Nonnull);
int              AG_DbOpen(AG_Db *_Nonnull, const char *_Nonnull, Uint);
void             AG_DbClose(AG_Db *_Nonnull);
int              AG_DbSync(AG_Db *_Nonnull);

int AG_DbExists(AG_Db *_Nonnull, AG_Dbt *_Nonnull);
int AG_DbGet(AG_Db *_Nonnull, const AG_Dbt *_Nonnull, AG_Dbt *_Nonnull);
int AG_DbPut(AG_Db *_Nonnull, const AG_Dbt *_Nonnull, const AG_Dbt *_Nonnull);
int AG_DbDel(AG_Db *_Nonnull, const AG_Dbt *_Nonnull);
int AG_DbIterate(AG_Db *_Nonnull, _Nonnull AG_DbIterateFn, void *_Nullable);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DB_H_ */
