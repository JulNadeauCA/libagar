/*	Public domain	*/

#ifndef _AGAR_CORE_DB_H_
#define _AGAR_CORE_DB_H_
#include <agar/core/begin.h>

/* Type of database */
enum ag_db_type {
	AG_DB_DUMMY,		/* No-op */
	AG_DB_DB4,		/* Berkeley DB v4 */
	AG_DB_LAST
};

/* DB4 database subtype */
enum ag_db4_type {
	AG_DB4_BTREE	= 1,
	AG_DB4_HASH	= 2,
	AG_DB4_RECNO	= 3,
	AG_DB4_QUEUE	= 4,
	AG_DB4_UNKNOWN	= 5
};

/* DB4 database open flags */
#define	AG_DB4_AUTO_COMMIT	0x2000000
#define	AG_DB4_CREATE		0x0000001
#define	AG_DB4_EXCL		0x0004000
#define	AG_DB4_MULTIVERSION	0x0000008
#define	AG_DB4_NOMMAP		0x0000010
#define	AG_DB4_RDONLY		0x0000020
#define	AG_DB4_THREAD		0x0000080
#define	AG_DB4_TRUNCATE		0x0000100
#define	AG_DB4_READ_UNCOMMITTED	0x8000000

typedef struct ag_db {
	struct ag_object obj;
	enum ag_db_type type;
	void *db4;			/* Pointer to DB object */
} AG_Db;

typedef struct ag_db_entry {
	AG_Db *db;			/* Back pointer to Db */
	void *key, *data;
	size_t keySize, dataSize;
} AG_DbEntry;

#define AGDB(p) ((AG_Db *)(p))

__BEGIN_DECLS
extern AG_ObjectClass agDbClass;

AG_Db       *AG_DbNew(enum ag_db_type);
AG_Db       *AG_DbNewDB4(const char *, enum ag_db4_type, Uint32);
AG_List     *AG_DbListKeys(AG_Db *);
int          AG_DbExists(AG_Db *, const char *);
int          AG_DbExistsDK(AG_Db *, void *, size_t);
int          AG_DbLookup(AG_Db *, AG_DbEntry *, const char *);
int          AG_DbLookupDK(AG_Db *, AG_DbEntry *, void *, size_t);
int          AG_DbDelete(AG_Db *, const char *);
int          AG_DbDeleteDK(AG_Db *, void *, size_t);
int          AG_DbPut(AG_Db *, AG_DbEntry *);
int          AG_DbSync(AG_Db *);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DB_H_ */
