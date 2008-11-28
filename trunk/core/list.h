/*	Public domain	*/
/*
 * Implementation of generic array-based lists (as opposed to linked lists
 * as implemented by the AG_Queue(3) macros). Private data can be associated
 * with list elements.
 */

#ifndef _AGAR_CORE_LIST_H_
#define _AGAR_CORE_LIST_H_

typedef struct ag_list {
	int n;			/* Element count */
	void  **v;		/* Element pointers */
	void  **vPvt;		/* Element private data */
	size_t *vPvtSize;	/* Element private data sizes */
} AG_List;

__BEGIN_DECLS

/* Create a new list */
static __inline__ AG_List *
AG_ListNew(void)
{
	AG_List *L;

	L = AG_Malloc(sizeof(AG_List));
	L->n = 0;
	L->v = NULL;
	L->vPvt = NULL;
	L->vPvtSize = NULL;
	return (L);
}

/* Duplicate an existing list */
static __inline__ AG_List *
AG_ListDup(const AG_List *L)
{
	size_t vLen = L->n*sizeof(void *);
	size_t vsLen = L->n*sizeof(size_t);
	AG_List *Ldup;
	int i;

	Ldup = AG_Malloc(sizeof(AG_List));
	Ldup->n = L->n;
	Ldup->v = AG_Malloc(vLen);
	Ldup->vPvt = AG_Malloc(vLen);
	Ldup->vPvtSize = AG_Malloc(vsLen);

	for (i = 0; i < Ldup->n; i++) {
		if (L->vPvt[i] != NULL) {
			memcpy(Ldup->vPvt[i], L->vPvt[i], L->vPvtSize[i]);
		} else {
			Ldup->vPvt[i] = NULL;
		}
	}
	memcpy(Ldup->v, L->v, vLen);
	memcpy(Ldup->vPvtSize, L->vPvtSize, vsLen);
	return (Ldup);
}

/* Insert a new item at the tail of a list. */
static __inline__ int
AG_ListAppend(AG_List *L, void *p)
{
	L->v = AG_Realloc(L->v, (L->n+1)*sizeof(void *));
	L->v[L->n] = p;
	L->vPvt = AG_Realloc(L->vPvt, (L->n+1)*sizeof(void *));
	L->vPvt[L->n] = NULL;
	L->vPvtSize = AG_Realloc(L->vPvtSize, (L->n+1)*sizeof(size_t));
	L->vPvtSize[L->n] = 0;
	return (L->n++);
}

/* Insert a new item with private data at the tail of a list. */
static __inline__ int
AG_ListAppendP(AG_List *L, const void *pPvt, size_t pvtSize)
{
	void *p;

	L->v = AG_Realloc(L->v, (L->n+1)*sizeof(void *));
	L->vPvt = AG_Realloc(L->vPvt, (L->n+1)*sizeof(void *));
	L->vPvtSize = AG_Realloc(L->vPvtSize, (L->n+1)*sizeof(size_t));
	p = AG_Malloc(pvtSize);
	L->v[L->n] = p;
	L->vPvt[L->n] = p;
	L->vPvtSize[L->n] = pvtSize;
	memcpy(p, pPvt, pvtSize);
	return (L->n++);
}

/*
 * Insert a new item with private data at specified index in list.
 * Return 1 on success, 0 if position was invalid.
 */
static __inline__ int
AG_ListInsert(AG_List *L, int pos, void *p)
{
	size_t vLen, vsLen;
	
	if (pos < 0 || pos > L->n)
		return (0);

	vLen = (L->n+1)*sizeof(void *);
	vsLen = (L->n+1)*sizeof(size_t);
	L->v = AG_Realloc(L->v, vLen);
	L->vPvt = AG_Realloc(L->vPvt, vLen);
	L->vPvtSize = AG_Realloc(L->vPvtSize, vsLen);
	if (pos < L->n) {
		vLen -= sizeof(void *);
		vsLen -= sizeof(size_t);
		memmove(&L->v[pos+1], &L->v[pos], vLen);
		memmove(&L->vPvt[pos+1], &L->vPvt[pos], vLen);
		memmove(&L->vPvtSize[pos+1], &L->vPvtSize[pos], vsLen);
	}
	L->v[pos] = p;
	L->vPvt[pos] = NULL;
	L->vPvtSize[pos] = 0;
	return (1);
}

/*
 * Insert a new item with private data at specified index in list.
 * Return 1 on success, 0 if position was invalid.
 */
static __inline__ int
AG_ListInsertP(AG_List *L, int pos, void *pPvt, size_t pvtSize)
{
	size_t vLen, vsLen;
	void *p;

	if (pos < 0 || pos > L->n)
		return (0);

	vLen = (L->n+1)*sizeof(void *);
	vsLen = (L->n+1)*sizeof(size_t);
	L->v = AG_Realloc(L->v, vLen);
	L->vPvt = AG_Realloc(L->vPvt, vLen);
	L->vPvtSize = AG_Realloc(L->vPvtSize, vsLen);
	if (pos < L->n) {
		vLen -= sizeof(void *);
		vsLen -= sizeof(size_t);
		memmove(&L->v[pos+1], &L->v[pos], vLen);
		memmove(&L->vPvt[pos+1], &L->vPvt[pos], vLen);
		memmove(&L->vPvtSize[pos+1], &L->vPvtSize[pos], vsLen);
	}
	L->v[pos] = pPvt;
	L->vPvt[pos] = pPvt;
	L->vPvtSize[pos] = pvtSize;

	p = AG_Malloc(pvtSize);
	L->v[pos] = p;
	L->vPvt[pos] = p;
	L->vPvtSize[pos] = pvtSize;
	memcpy(p, pPvt, pvtSize);
	return (1);
}

/* Insert at head of list. */
static __inline__ int
AG_ListPrepend(AG_List *L, void *p)
{
	return AG_ListInsert(L,0,p);
}

static __inline__ int
AG_ListPrependP(AG_List *L, void *pPvt, size_t pvtSize)
{
	return AG_ListInsertP(L,0,pPvt,pvtSize);
}

/* Remove an item from the list by index. Return 1 if successful. */
static __inline__ int
AG_ListRemove(AG_List *L, int idx)
{
	if (idx < 0 || idx >= L->n) {
		return (0);
	}
	AG_Free(L->vPvt[idx]);
	if (idx < L->n-1) {
		size_t vLen = (L->n - 1)*sizeof(void *);
		size_t vsLen = (L->n - 1)*sizeof(size_t);
		memmove(&L->v[idx], &L->v[idx+1], vLen);
		memmove(&L->vPvt[idx], &L->vPvt[idx+1], vLen);
		memmove(&L->vPvtSize[idx], &L->vPvtSize[idx+1], vsLen);
	}
	L->n--;
	return (1);
}

/* Remove all items from a list. */
static __inline__ void
AG_ListClear(AG_List *L)
{
	int i;

	for (i = 0; i < L->n; i++) {
		AG_Free(L->vPvt[i]);
	}
	AG_Free(L->vPvt);
	AG_Free(L->v);
	AG_Free(L->vPvtSize);
	L->vPvt = NULL;
	L->v = NULL;
	L->vPvtSize = NULL;
}

/* Release resources allocated by a list. */
static __inline__ void
AG_ListDestroy(AG_List *L)
{
	AG_ListClear(L);
	AG_Free(L);
}

/* Create a new list from an array of strings. */
static __inline__ AG_List *
AG_ListNewFromStrings(int n, const char **s)
{
	AG_List *L;
	int i;

	L = AG_ListNew();
	for (i = 0; i < n; i++) {
		AG_ListAppendP(L, s[i], strlen(s[i])+1);
	}
	return (L);
}
__END_DECLS

#if 0
#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_LISTS)
# define LIST			AG_List
# define LIST_New		AG_ListNew
# define LIST_NewFromStrings	AG_ListNewFromStrings
# define LIST_Dup		AG_ListDup
# define LIST_Insert		AG_ListInsert
# define LIST_InsertP		AG_ListInsertP
# define LIST_Append		AG_ListAppend
# define LIST_AppendP		AG_ListAppendP
# define LIST_Prepend		AG_ListPrepend
# define LIST_PrependP		AG_ListPrependP
# define LIST_Remove		AG_ListRemove
# define LIST_Clear		AG_ListClear
# define LIST_Destroy		AG_ListDestroy
#endif
#endif

#endif /* _AGAR_CORE_LIST_H_ */
