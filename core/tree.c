/*	Public domain	*/

#include <agar/core/core.h>

/*
 * Generically-accessible, ordered tree of opaque data elements.
 *
 * Traversal of linked list (e.g., AG_Queue(3)) based trees requires knowledge
 * about the internal structure of the node data (i.e., the location of the
 * prev/next pointers). AG_Tree provides a container which encapsulates opaque
 * node data (by using its own separate linkage structure).
 *
 * GUI widgets such as AG_Tlist(3) normally require a user-provided population
 * routine which must traverse the entire tree (culling any non-expanded items).
 * AG_Tlist(3) then compares the new items against its cache of previous items
 * in order to restore user selections (and reuse potential hardware assets).
 *
 * AG_Tree provides a safer, generically-accessible tree structure which
 * encapsulates opaque data items and separates linkage from node data.
 *
 * Cons:
 *   1) One extra level of indirection.
 *   2) About 40 - 88 bytes of memory footprint per node.
 *
 * Pros:
 *   1) Improved safety. A corrupted note is less likely to cause unpredictable
 *      behavior as a result of bad linkage pointers being followed (and is more
 *      likely to crash in the expected context such as an accessor routine).
 *   2) Simplified access by GUI system and other external code. AG_TreeView(3)
 *      can access AG_Tree's directly (the only code needed being the accessor
 *      to access and render the node data).
 *   3) A flat, redundant linked list allows non-recursive global iterators
 *      AGTREE_FOREACH_ITEM() and AGTREE_FOREACH_DATA().
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

AG_TreeItem *
AG_TreeItemNew(void *p, AG_Size privDataSize)
{
	AG_TreeItem *ti;

	ti = (AG_TreeItem *)AG_Malloc(sizeof(AG_TreeItem));
	ti->p = p;
	ti->privData = (privDataSize > 0) ? AG_Malloc(privDataSize) : NULL;
	ti->privDataSize = privDataSize;
	ti->parentTree = NULL;
	ti->parentItem = NULL;
	AG_TAILQ_INIT(&ti->chldItems);
	return (ti);
}

void
AG_TreeItemDestroy(AG_TreeItem *ti)
{
#ifdef DEBUG
	if (ti->parentItem != NULL || ti->parentTree != NULL)
		AG_FatalError("Attempt to destroy referenced TreeItem");
#endif
	AG_Free(ti->privData);
	AG_Free(ti);
}

void
AG_TreeAttach(AG_TreeItem *tiParent, AG_TreeItem *ti)
{
	ti->parentTree = tiParent->parentTree;
	ti->parentItem = tiParent;
	AG_TAILQ_INSERT_TAIL(&ti->parentTree->list, ti, list);
	AG_TAILQ_INSERT_TAIL(&tiParent->chldItems, ti, tree);
}

void
AG_TreeDetach(AG_Tree *t, AG_TreeItem *ti)
{
	if (ti->parentItem != NULL) {
		AG_TAILQ_REMOVE(&ti->parentItem->chldItems, ti, tree);
		ti->parentItem = NULL;
	} else if (ti == t->root) {
		t->root = NULL;
	}
	AG_TAILQ_REMOVE(&t->list, ti, list);
	ti->parentTree = NULL;
}

AG_TreeItem *
AG_TreeInsert(AG_Tree *t, AG_TreeItem *tiParent, void *p, AG_Size privDataSize)
{
	AG_TreeItem *ti;

	ti = AG_TreeItemNew(p, privDataSize);
	if (tiParent == NULL) {
		ti->parentTree = t;
		ti->parentItem = NULL;
		AG_TAILQ_INSERT_TAIL(&t->list, ti, list);
		t->root = ti;
	} else {
		AG_TreeAttach(tiParent, ti);
	}
	return (ti);
}

void
AG_TreeRemove(AG_Tree *t, AG_TreeItem *ti)
{
	AG_TreeDetach(t, ti);
	AG_TreeItemDestroy(ti);
}

void
AG_TreeClear(AG_Tree *t)
{
	AG_TreeItem *ti, *tiNext;

	for (ti = AG_TAILQ_FIRST(&t->list);
	     ti != AG_TAILQ_END(&t->list);
	     ti = tiNext) {
		tiNext = AG_TAILQ_NEXT(ti,list);
		AG_TreeItemDestroy(ti);
	}
	AG_TAILQ_INIT(&t->list);
	t->root = NULL;
}
