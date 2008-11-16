/*	Public domain	*/
/*
 * Implementation of array-based lists (as opposed to linked lists as
 * in the queue(3) macros).
 */

#ifndef _AGAR_CORE_LIST_H_
#define _AGAR_CORE_LIST_H_

typedef struct list {
	int n;			/* Element count */
	void  **v;		/* Element pointers */
	void  **vPvt;		/* Element private data */
	size_t *vPvtSize;	/* Element private data sizes */
} LIST;

__BEGIN_DECLS

/* Create a new list */
static __inline__ LIST *
LIST_New(void)
{
	LIST *L;

	L = Malloc(sizeof(LIST));
	L->n = 0;
	L->v = NULL;
	L->vPvt = NULL;
	L->vPvtSize = NULL;
	return (L);
}

/* Duplicate an existing list */
static __inline__ LIST *
LIST_Dup(const LIST *L)
{
	size_t vLen = L->n*sizeof(void *);
	size_t vsLen = L->n*sizeof(size_t);
	LIST *Ldup;
	int i;

	Ldup = Malloc(sizeof(LIST));
	Ldup->n = L->n;
	Ldup->v = Malloc(vLen);
	Ldup->vPvt = Malloc(vLen);
	Ldup->vPvtSize = Malloc(vsLen);

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
LIST_Append(LIST *L, void *p)
{
	L->v = Realloc(L->v, (L->n+1)*sizeof(void *));
	L->v[L->n] = p;
	L->vPvt = Realloc(L->vPvt, (L->n+1)*sizeof(void *));
	L->vPvt[L->n] = NULL;
	L->vPvtSize = Realloc(L->vPvtSize, (L->n+1)*sizeof(size_t));
	L->vPvtSize[L->n] = 0;
	return (L->n++);
}

/* Insert a new item with private data at the tail of a list. */
static __inline__ int
LIST_AppendPvt(LIST *L, const void *pPvt, size_t pvtSize)
{
	void *p;

	L->v = Realloc(L->v, (L->n+1)*sizeof(void *));
	L->vPvt = Realloc(L->vPvt, (L->n+1)*sizeof(void *));
	L->vPvtSize = Realloc(L->vPvtSize, (L->n+1)*sizeof(size_t));
	p = Malloc(pvtSize);
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
LIST_Insert(LIST *L, int pos, void *p)
{
	size_t vLen, vsLen;
	
	if (pos < 0 || pos > L->n)
		return (0);

	vLen = (L->n+1)*sizeof(void *);
	vsLen = (L->n+1)*sizeof(size_t);
	L->v = Realloc(L->v, vLen);
	L->vPvt = Realloc(L->vPvt, vLen);
	L->vPvtSize = Realloc(L->vPvtSize, vsLen);
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
LIST_InsertPvt(LIST *L, int pos, void *pPvt, size_t pvtSize)
{
	size_t vLen, vsLen;
	void *p;

	if (pos < 0 || pos > L->n)
		return (0);

	vLen = (L->n+1)*sizeof(void *);
	vsLen = (L->n+1)*sizeof(size_t);
	L->v = Realloc(L->v, vLen);
	L->vPvt = Realloc(L->vPvt, vLen);
	L->vPvtSize = Realloc(L->vPvtSize, vsLen);
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

	p = Malloc(pvtSize);
	L->v[pos] = p;
	L->vPvt[pos] = p;
	L->vPvtSize[pos] = pvtSize;
	memcpy(p, pPvt, pvtSize);
	return (1);
}

/* Insert at head of list. */
static __inline__ int
LIST_Prepend(LIST *L, void *p)
{
	return LIST_Insert(L,0,p);
}

static __inline__ int
LIST_PrependPvt(LIST *L, void *pPvt, size_t pvtSize)
{
	return LIST_InsertPvt(L,0,pPvt,pvtSize);
}

/* Remove an item from the list by index. Return 1 if successful. */
static __inline__ int
LIST_Remove(LIST *L, int idx)
{
	if (idx < 0 || idx >= L->n) {
		return (0);
	}
	Free(L->vPvt[idx]);
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
LIST_Clear(LIST *L)
{
	int i;

	for (i = 0; i < L->n; i++) {
		Free(L->vPvt[i]);
	}
	Free(L->vPvt);
	Free(L->v);
	Free(L->vPvtSize);
	L->vPvt = NULL;
	L->v = NULL;
	L->vPvtSize = NULL;
}

/* Release resources allocated by a list. */
static __inline__ void
LIST_Destroy(LIST *L)
{
	LIST_Clear(L);
	Free(L);
}

/* Create a new list from an array of strings. */
static __inline__ LIST *
LIST_NewFromStrings(int n, const char **s)
{
	LIST *L;
	int i;

	L = LIST_New();
	for (i = 0; i < n; i++) {
		LIST_AppendPvt(L, s[i], strlen(s[i])+1);
	}
	return (L);
}
__END_DECLS

#endif /* _AGAR_CORE_LIST_H_ */
