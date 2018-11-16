/*	Public domain	*/
/*
 * Implementation of a generic hash table of AG_Variable(3) items.
 */

#ifndef _AGAR_CORE_TBL_H_
#define _AGAR_CORE_TBL_H_
#include <agar/core/begin.h>

typedef struct ag_tbl_bucket {
	char *_Nullable *_Nonnull keys;
	AG_Variable *_Nullable    ents;
	Uint                     nEnts;
} AG_TblBucket;

typedef struct ag_tbl {
	Uint flags;
#define AG_TBL_DUPLICATES	0x01	/* Allow duplicate entries */

	AG_TblBucket *_Nonnull buckets;		/* Hash buckets */
	Uint         nBuckets;			/* Bucket count */
} AG_Tbl;

__BEGIN_DECLS
AG_Tbl *_Nonnull AG_TblNew(Uint, Uint);
void             AG_TblInit(AG_Tbl *_Nonnull, Uint, Uint);
void             AG_TblDestroy(AG_Tbl *_Nonnull);

AG_Variable *_Nullable AG_TblLookupHash(AG_Tbl *_Nonnull, Uint,
                                        const char *_Nonnull)
                                       _Pure_Attribute;

int AG_TblExistsHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull)
                    _Pure_Attribute;

int AG_TblInsertHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull,
                     const AG_Variable *_Nonnull);

int AG_TblDeleteHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull);

/* Iterate over each entry. */
#define AG_TBL_FOREACH(var, i,j, tbl)					\
	for ((i) = 0; ((i) < (tbl)->nBuckets); (i)++)			\
		for ((j) = 0;						\
		    ((j) < (tbl)->buckets[i].nEnts) &&			\
		     ((var) = &(tbl)->buckets[i].ents[j]);		\
		     (j)++)

/* General hash function */
static __inline__ Uint _Pure_Attribute
AG_TblHash(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
{
	Uint h;
	Uchar *p;

	for (h = 0, p = (Uchar *)key; *p != '\0'; p++) {
		h = 31*h + *p;
	}
	return (h % tbl->nBuckets);
}

/*
 * Shorthand access routines.
 */
static __inline__ AG_Variable *_Nullable _Pure_Attribute
AG_TblLookup(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
{
	return AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key);
}
static __inline__ int
AG_TblLookupPointer(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    void *_Nonnull *_Nullable p)
{
	AG_Variable *V;
	
	if ((V = AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key)) != NULL) {
		*p = V->data.p;
		return (0);
	}
	return (-1);
}
static __inline__ int _Pure_Attribute
AG_TblExists(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
{
	return AG_TblExistsHash(tbl, AG_TblHash(tbl,key), key);
}
static __inline__ int
AG_TblInsert(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    const AG_Variable *_Nonnull V)
{
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, V);
}
static __inline__ int
AG_TblInsertPointer(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    void *_Nullable p)
{
	AG_Variable V;
	AG_InitPointer(&V, p);
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, &V);
}
static __inline__ int
AG_TblDelete(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
{
	return AG_TblDeleteHash(tbl, AG_TblHash(tbl,key), key);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_TBL_H_ */
