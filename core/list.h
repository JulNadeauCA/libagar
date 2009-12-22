/*	Public domain	*/
/*
 * Implementation of a generic array-based list of AG_Variable(3) items.
 */

#ifndef _AGAR_CORE_LIST_H_
#define _AGAR_CORE_LIST_H_
#include <agar/core/begin.h>

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

	if ((L = (AG_List *)AG_TryMalloc(sizeof(AG_List))) == NULL) {
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
	int i;

	if ((Ldup = (AG_List *)AG_TryMalloc(sizeof(AG_List))) == NULL) {
		return (NULL);
	}
	Ldup->n = L->n;
	if ((Ldup->v = (AG_Variable *)AG_TryMalloc(vLen)) == NULL) {
		AG_Free(Ldup);
		return (NULL);
	}
	memcpy(Ldup->v, L->v, vLen);
	for (i = 0; i < L->n; i++) {
		AG_Variable *V = &L->v[i];
		AG_Variable *Vdup = &Ldup->v[i];
		if (V->type == AG_VARIABLE_STRING &&
		    V->info.size == 0)
			Vdup->data.s = AG_Strdup(V->data.s);
	}
	return (Ldup);
}

/* Insert a new variable at the tail of a list. */
static __inline__ int
AG_ListAppend(AG_List *L, const AG_Variable *V)
{
	AG_Variable *lv;

	if ((lv = (AG_Variable *)AG_TryRealloc(L->v,
	    (L->n+1)*sizeof(AG_Variable))) == NULL) {
		return (-1);
	}
	L->v = lv;
	memcpy(&L->v[L->n], V, sizeof(AG_Variable));
	if (V->type == AG_VARIABLE_STRING &&
	    V->info.size == 0) {
		L->v[L->n].data.s = AG_Strdup(V->data.s);
	}
	return (L->n++);
}

/*
 * Insert a new variable at the specified index in list.
 * Return position on success, -1 on failure.
 */
static __inline__ int
AG_ListInsert(AG_List *L, int pos, const AG_Variable *V)
{
	AG_Variable *lv;
	size_t vLen;
	
	if (pos < 0 || pos > L->n) {
		AG_SetError("Bad index: %d", pos);
		return (-1);
	}
	vLen = (L->n+1)*sizeof(AG_Variable);
	if ((lv = (AG_Variable *)AG_TryRealloc(L->v, vLen)) == NULL) {
		return (-1);
	}
	L->v = lv;
	if (pos < L->n) {
		vLen -= sizeof(AG_Variable);
		memmove(&L->v[pos+1], &L->v[pos], vLen);
	}
	memcpy(&L->v[pos], V, sizeof(AG_Variable));
	if (V->type == AG_VARIABLE_STRING &&
	    V->info.size == 0) {
		L->v[pos].data.s = AG_Strdup(V->data.s);
	}
	return (pos);
}

/* Insert a new variable at head of list. */
static __inline__ int
AG_ListPrepend(AG_List *L, const AG_Variable *d)
{
	return AG_ListInsert(L,0,d);
}

/*
 * Remove a variable from a list by index.
 * Return 1 on success, -1 on failure.
 */
static __inline__ int
AG_ListRemove(AG_List *L, int idx)
{
	AG_Variable *V;

	if (idx < 0 || idx >= L->n) {
		AG_SetError("Bad index: %d", idx);
		return (-1);
	}
	V = &L->v[idx];
	if (V->type == AG_VARIABLE_STRING &&
	    V->info.size == 0) {
		AG_Free(V->data.s);
	}
	if (idx < L->n-1) {
		memmove(V, &L->v[idx+1], (L->n-1)*sizeof(AG_Variable));
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
		AG_Variable *V = &L->v[i];
		if (V->type == AG_VARIABLE_STRING &&
		    V->info.size == 0)
			AG_Free(V->data.s);
	}
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
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LIST_H_ */
