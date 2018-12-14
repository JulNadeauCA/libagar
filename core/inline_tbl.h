/*	Public domain	*/

/* General hash function */
#ifdef AG_INLINE_HEADER
static __inline__ Uint _Pure_Attribute
AG_TblHash(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
#else
Uint
ag_tbl_hash(AG_Tbl *tbl, const char *key)
#endif
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
#ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nullable _Pure_Attribute
AG_TblLookup(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
#else
AG_Variable *
ag_tbl_lookup(AG_Tbl *tbl, const char *key)
#endif
{
	return AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_TblLookupPointer(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    void *_Nonnull *_Nullable p)
#else
int
ag_tbl_lookup_pointer(AG_Tbl *tbl, const char *key, void **p)
#endif
{
	AG_Variable *V;
	
	if ((V = AG_TblLookupHash(tbl, AG_TblHash(tbl,key), key)) != NULL) {
		*p = V->data.p;
		return (0);
	}
	return (-1);
}

#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_TblExists(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
#else
int
ag_tbl_exists(AG_Tbl *tbl, const char *key)
#endif
{
	return AG_TblExistsHash(tbl, AG_TblHash(tbl,key), key);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_TblInsert(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    const AG_Variable *_Nonnull V)
#else
int
ag_tbl_insert(AG_Tbl *tbl, const char *key, const AG_Variable *V)
#endif
{
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, V);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_TblInsertPointer(AG_Tbl *_Nonnull tbl, const char *_Nonnull key,
    void *_Nullable p)
#else
int
ag_tbl_insert_pointer(AG_Tbl *tbl, const char *key, void *p)
#endif
{
	AG_Variable V;
	AG_InitPointer(&V, p);
	return AG_TblInsertHash(tbl, AG_TblHash(tbl,key), key, &V);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_TblDelete(AG_Tbl *_Nonnull tbl, const char *_Nonnull key)
#else
int
ag_tbl_delete(AG_Tbl *tbl, const char *key)
#endif
{
	return AG_TblDeleteHash(tbl, AG_TblHash(tbl,key), key);
}
