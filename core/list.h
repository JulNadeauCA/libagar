/*	Public domain	*/
/*
 * Implementation of a generic array-based list of AG_Variable(3) items.
 */

#ifndef _AGAR_CORE_LIST_H_
#define _AGAR_CORE_LIST_H_
#include <agar/core/begin.h>

typedef struct ag_list {
	int n;				/* Element count */
	AG_Variable *_Nullable v;	/* Items */
} AG_List;

__BEGIN_DECLS

/* Create a new list */
static __inline__ AG_List *_Nonnull /* _Malloc_Like_Attribute */
AG_ListNew(void)
{
	AG_List *L;

	L = AG_Malloc(sizeof(AG_List));
	L->n = 0;
	L->v = NULL;
	return (L);
}

/* Duplicate an existing list */
static __inline__ AG_List *_Nonnull
AG_ListDup(const AG_List *_Nonnull L)
{
	AG_Size vLen = L->n*sizeof(AG_Variable);
	AG_List *Ldup;
	int i;

	Ldup = (AG_List *)AG_Malloc(sizeof(AG_List));
	Ldup->n = L->n;
	Ldup->v = (AG_Variable *)AG_Malloc(vLen);
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
AG_ListAppend(AG_List *_Nonnull L, const AG_Variable *_Nonnull V)
{
	L->v = AG_Realloc(L->v, (L->n+1)*sizeof(AG_Variable));
	memcpy(&L->v[L->n], V, sizeof(AG_Variable));

	if (V->type == AG_VARIABLE_STRING &&
	    V->info.size == 0) {
		L->v[L->n].data.s = AG_Strdup(V->data.s);
	}
	return (L->n++);
}

/*
 * Insert a new variable at the specified index in list.
 * Return position on success. Raise exception on failure or bad index.
 */
static __inline__ Uint
AG_ListInsert(AG_List *_Nonnull L, Uint pos, const AG_Variable *_Nonnull V)
{
	AG_Size vLen;
	
	if (pos > L->n) {
		AG_FatalError("Bad index");
	}
	vLen = (L->n+1)*sizeof(AG_Variable);
	L->v = AG_Realloc(L->v, vLen);

	if (pos < L->n) {
		vLen -= sizeof(AG_Variable);
		memmove(&L->v[pos+1], &L->v[pos], vLen);
	}
	memcpy(&L->v[pos], V, sizeof(AG_Variable));

	if (V->type == AG_VARIABLE_STRING && V->info.size == 0) {
		L->v[pos].data.s = AG_Strdup(V->data.s);
	}
	return (pos);
}

/* Insert a new variable at head of list. */
static __inline__ int
AG_ListPrepend(AG_List *_Nonnull L, const AG_Variable *_Nonnull d)
{
	return AG_ListInsert(L,0,d);
}

/*
 * Remove a variable from a list by index.
 * Return 1 on success, -1 on failure.
 */
static __inline__ void
AG_ListRemove(AG_List *_Nonnull L, Uint idx)
{
	AG_Variable *V;

	if (idx >= L->n) {
		AG_FatalError("Bad index");
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
}

/* Remove all items from a list. */
static __inline__ void
AG_ListClear(AG_List *_Nonnull L)
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
AG_ListDestroy(AG_List *_Nonnull L)
{
	AG_ListClear(L);
	AG_Free(L);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LIST_H_ */
