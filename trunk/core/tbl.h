/*	Public domain	*/
/*
 * Implementation of a generic hash table of AG_Variable(3) items.
 */

#ifndef _AGAR_CORE_TBL_H_
#define _AGAR_CORE_TBL_H_
#include <agar/core/begin.h>

typedef struct ag_tbl_bucket {
	char        **keys;
	AG_Variable  *ents;
	Uint         nEnts;
} AG_TblBucket;

typedef struct ag_tbl {
	Uint flags;
#define AG_TBL_DUPLICATES	0x01	/* Allow duplicate entries */

	AG_TblBucket *buckets;		/* Hash buckets */
	Uint         nBuckets;		/* Bucket count */
} AG_Tbl;

__BEGIN_DECLS
AG_Tbl      *AG_TblNew(Uint, Uint);
int          AG_TblInit(AG_Tbl *, Uint, Uint);
void         AG_TblDestroy(AG_Tbl *);
AG_Variable *AG_TblLookupHash(AG_Tbl *, Uint, const char *);
int          AG_TblExistsHash(AG_Tbl *, Uint, const char *);
int          AG_TblInsertHash(AG_Tbl *, Uint, const char *, const AG_Variable *);
int          AG_TblDeleteHash(AG_Tbl *, Uint, const char *);

/* Iterate over each entry. */
#define AG_TBL_FOREACH(var, i,j, tbl)					\
	for ((i) = 0; ((i) < (tbl)->nBuckets); (i)++)			\
		for ((j) = 0;						\
		    ((j) < (tbl)->buckets[i].nEnts) &&			\
		     ((var) = &(tbl)->buckets[i].ents[j]);		\
		     (j)++)

/* General hash function */
static __inline__ Uint
AG_TblHash(AG_Tbl *tbl, const char *key)
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
static __inline__ AG_Variable *
AG_TblLookup(AG_Tbl *tbl, const char *key)
{
	return AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key);
}
static __inline__ int
AG_TblLookupPointer(AG_Tbl *tbl, const char *key, void **p)
{
	AG_Variable *V;
	
	if ((V = AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key)) != NULL) {
		*p = V->data.p;
		return (0);
	}
	return (-1);
}
static __inline__ int
AG_TblExists(AG_Tbl *tbl, const char *key)
{
	return AG_TblExistsHash(tbl, AG_TblHash(tbl,key), key);
}
static __inline__ int
AG_TblInsert(AG_Tbl *tbl, const char *key, const AG_Variable *V)
{
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, V);
}
static __inline__ int
AG_TblInsertPointer(AG_Tbl *tbl, const char *key, void *p)
{
	AG_Variable V;
	AG_InitPointer(&V, p);
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, &V);
}
static __inline__ int
AG_TblDelete(AG_Tbl *tbl, const char *key)
{
	return AG_TblDeleteHash(tbl, AG_TblHash(tbl,key), key);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_TBL_H_ */
