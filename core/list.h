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
AG_List *_Nonnull /*_Malloc_Like_Attribute*/ AG_ListNew(void);
AG_List *_Nonnull AG_ListDup(const AG_List *_Nonnull);

int  AG_ListAppend(AG_List *_Nonnull, const AG_Variable *_Nonnull);
Uint AG_ListInsert(AG_List *_Nonnull, Uint, const AG_Variable *_Nonnull);
int  AG_ListPrepend(AG_List *_Nonnull, const AG_Variable *_Nonnull);
void AG_ListRemove(AG_List *_Nonnull, Uint);
void AG_ListClear(AG_List *_Nonnull);
void AG_ListDestroy(AG_List *_Nonnull);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LIST_H_ */
