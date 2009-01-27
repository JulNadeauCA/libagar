/*	Public domain	*/
/*
 * Implementation of a generic array-based list of AG_Variable(3) items.
 */

#ifndef _AGAR_CORE_LIST_H_
#define _AGAR_CORE_LIST_H_

typedef struct ag_list {
	int n;			/* Element count */
	AG_Variable *v;		/* Items */
} AG_List;

__BEGIN_DECLS

/* Create a new list */
static __inline__ AG_List *
AG_ListNew(void)
{
	AG_List *L;

	if ((L = (AG_List *)malloc(sizeof(AG_List))) == NULL) {
		AG_SetError("Out of memory");
		return (NULL);
	}
	L->n = 0;
	L->v = NULL;
	return (L);
}

/* Duplicate an existing list */
static __inline__ AG_List *
AG_ListDup(const AG_List *L)
{
	size_t vLen = L->n*sizeof(AG_Variable);
	AG_List *Ldup;

	Ldup = (AG_List *)AG_Malloc(sizeof(AG_List));
	Ldup->n = L->n;
	Ldup->v = (AG_Variable *)AG_Malloc(vLen);
	memcpy(Ldup->v, L->v, vLen);
	return (Ldup);
}

/* Insert a new variable at the tail of a list. */
static __inline__ int
AG_ListAppend(AG_List *L, const AG_Variable *d)
{
	L->v = (AG_Variable *)AG_Realloc(L->v, (L->n+1)*sizeof(AG_Variable));
	memcpy(&L->v[L->n], d, sizeof(AG_Variable));
	return (L->n++);
}

/*
 * Insert a new variable at the specified index in list.
 * Return 1 on success, 0 if index was invalid.
 */
static __inline__ int
AG_ListInsert(AG_List *L, int pos, const AG_Variable *d)
{
	size_t vLen;
	
	if (pos < 0 || pos > L->n)
		return (0);

	vLen = (L->n+1)*sizeof(AG_Variable);
	L->v = (AG_Variable *)AG_Realloc(L->v, vLen);
	if (pos < L->n) {
		vLen -= sizeof(AG_Variable);
		memmove(&L->v[pos+1], &L->v[pos], vLen);
	}
	memcpy(&L->v[pos], d, sizeof(AG_Variable));
	return (1);
}

/* Insert a new variable at head of list. */
static __inline__ int
AG_ListPrepend(AG_List *L, const AG_Variable *d)
{
	return AG_ListInsert(L,0,d);
}

/*
 * Remove a variable from a list by index.
 * Return 1 on success, 0 if index was invalid.
 */
static __inline__ int
AG_ListRemove(AG_List *L, int idx)
{
	if (idx < 0 || idx >= L->n) {
		return (0);
	}
	if (idx < L->n-1) {
		memmove(&L->v[idx], &L->v[idx+1], (L->n-1)*sizeof(AG_Variable));
	}
	L->n--;
	return (1);
}

/* Remove all items from a list. */
static __inline__ void
AG_ListClear(AG_List *L)
{
	AG_Free(L->v);
	L->v = NULL;
}

/* Release resources allocated by a list. */
static __inline__ void
AG_ListDestroy(AG_List *L)
{
	AG_ListClear(L);
	AG_Free(L);
}

/* Insert items of specific types. */
static __inline__ int
AG_ListAppendPointer(AG_List *L, void *p)
{
	AG_Variable V;
	
	V.type = AG_VARIABLE_POINTER;
	V.name[0] = '\0';
	V.mutex = NULL;
	V.fn.fnVoid = NULL;
	V.data.p = p;
	return AG_ListAppend(L, &V);
}
__END_DECLS

#endif /* _AGAR_CORE_LIST_H_ */
