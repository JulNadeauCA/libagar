/*	Public domain	*/

#include <agar/core/core.h>

/*
 * Generically-accessible, ordered tree of opaque data elements.
 */

AG_Tree *
AG_TreeNew(void)
{
	AG_Tree *t;

	t = Malloc(sizeof(AG_Tree));
	t->root = NULL;
	TAILQ_INIT(&t->list);
	return (t);
}

void
AG_TreeDestroy(AG_Tree *t)
{
	AG_TreeClear(t);
	Free(t);
}
