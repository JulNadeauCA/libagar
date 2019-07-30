/*	Public domain	*/
/*
 * Hash table of AG_Variable(3)'s
 */

#ifndef _AGAR_CORE_TBL_H_
#define _AGAR_CORE_TBL_H_
#include <agar/core/begin.h>

typedef struct ag_tbl_bucket {
	char *_Nullable *_Nonnull keys;
	AG_Variable *_Nullable    ents;
	Uint                     nEnts;
	Uint32 _pad;
} AG_TblBucket;

typedef struct ag_tbl {
	Uint flags;
#define AG_TBL_DUPLICATES	0x01	/* Allow duplicate entries */

	Uint                  nBuckets;		/* Bucket count */
	AG_TblBucket *_Nonnull buckets;		/* Hash buckets */
} AG_Tbl;

__BEGIN_DECLS
AG_Tbl *_Nonnull AG_TblNew(Uint, Uint);
void             AG_TblInit(AG_Tbl *_Nonnull, Uint, Uint);
void             AG_TblDestroy(AG_Tbl *_Nonnull);

AG_Variable *_Nullable AG_TblLookupHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull)
                                       _Pure_Attribute;
int                    AG_TblExistsHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull)
                                       _Pure_Attribute;
int                    AG_TblInsertHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull,
                                        const AG_Variable *_Nonnull);
int                    AG_TblDeleteHash(AG_Tbl *_Nonnull, Uint, const char *_Nonnull);

/* Iterate over each entry. */
#define AG_TBL_FOREACH(var, i,j, tbl)					\
	for ((i) = 0; ((i) < (tbl)->nBuckets); (i)++)			\
		for ((j) = 0;						\
		    ((j) < (tbl)->buckets[i].nEnts) &&			\
		     ((var) = &(tbl)->buckets[i].ents[j]);		\
		     (j)++)
/*
 * Inlinables
 */
Uint                   ag_tbl_hash(AG_Tbl *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute;
AG_Variable *_Nullable ag_tbl_lookup(AG_Tbl *_Nonnull, const char *_Nonnull)
                                    _Pure_Attribute;
int                    ag_tbl_lookup_pointer(AG_Tbl *_Nonnull, const char *_Nonnull,
                                             void *_Nonnull *_Nullable);
int ag_tbl_exists(AG_Tbl *_Nonnull, const char *_Nonnull);
int ag_tbl_insert(AG_Tbl *_Nonnull, const char *_Nonnull, const AG_Variable *_Nonnull);
int ag_tbl_insert_pointer(AG_Tbl *_Nonnull, const char *_Nonnull, void *_Nullable);
int ag_tbl_delete(AG_Tbl *_Nonnull, const char *_Nonnull);
#ifdef AG_INLINE_TBL
# define AG_INLINE_HEADER
# include <agar/core/inline_tbl.h>
#else
# define AG_TblHash(t,k)            ag_tbl_hash((t),(k))
# define AG_TblLookup(t,k)          ag_tbl_lookup((t),(k))
# define AG_TblLookupPointer(t,k,p) ag_tbl_lookup_pointer((t),(k),(p))
# define AG_TblExists(t,k)          ag_tbl_exists((t),(k))
# define AG_TblInsert(t,k,V)        ag_tbl_insert((t),(k),(V))
# define AG_TblInsertPointer(t,k,p) ag_tbl_insert_pointer((t),(k),(p))
# define AG_TblDelete(t,k)          ag_tbl_delete((t),(k))
#endif /* !AG_INLINE_TBL */
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_TBL_H_ */
