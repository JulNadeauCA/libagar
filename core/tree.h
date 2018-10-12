/*	Public domain	*/

/*
 * Generically-accessible, ordered tree of opaque data elements.
 *
 * Traversal of linked-list based trees (e.g., structures which use AG_Queue(3)
 * to represent nodes) normally requires knowledge about the structure of the
 * node data (i.e., the location of the next pointers). AG_Tree provides a very
 * simple container which allows opaque data to be encapsulated.
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
 *   2) About 44-88 bytes per node of memory footprint.
 *
 * Pros:
 *   1) Improved safety, because data corruption of a node item is more likely
 *      to crash in an expected context (say an accessor function) as opposed
 *      to unpredictable behavior due to bad linkage pointers being followed.
 *   2) Simplified GUI code. AG_TreeView(3) can access an AG_Tree directly, the
 *      only code needed being an accessor function to render node to graphics.
 *   3) A flat, redundant linked list allows non-recursive global iterators
 *      AGTREE_FOREACH_ITEM() and AGTREE_FOREACH_DATA().
 */

#ifndef _AGAR_CORE_TREE_H_
#define _AGAR_CORE_TREE_H_
#include <agar/core/begin.h>

struct ag_tree;
struct ag_tree_item;

AG_TAILQ_HEAD(ag_tree_itemq, ag_tree_item);

typedef struct ag_tree_item {
	void *_Nullable p;			/* User pointer */
	void *_Nullable privData;		/* Private data */
	size_t privDataSize;			/* Private data size */
	struct ag_tree_itemq chldItems;		/* Child items */
	AG_TAILQ_ENTRY(ag_tree_item) tree;	/* Entry in tree */
	AG_TAILQ_ENTRY(ag_tree_item) list;	/* Entre in list */
	struct ag_tree *_Nonnull parentTree;		/* Parent tree */
	struct ag_tree_item *_Nullable parentItem;	/* Parent item */
} AG_TreeItem;

typedef struct ag_tree {
	AG_TreeItem *_Nullable root;
	struct ag_tree_itemq list;
} AG_Tree;

/* Iterate over direct children under a specified item. */
#define AGTREE_FOREACH_CHILD_ITEM(var, item) \
	for((var) = AG_TAILQ_FIRST(&(item)->chldItems); \
	    (var) != AG_TAILQ_END(&(item)->chldItems); \
	    (var) = AG_TAILQ_NEXT((var), tree))

/* Iterate over data of direct children under a specified item. */
#define AGTREE_FOREACH_CHILD_DATA(var, item, t) \
	for((var) = (struct t *)(AG_TAILQ_FIRST(&(item)->chldItems))->p; \
	    (var) != (struct t *)(AG_TAILQ_END(&(item)->chldItems))->p; \
	    (var) = AG_TAILQ_NEXT((var), tree))

/* Iterate over all items. */
#define AGTREE_FOREACH_ITEM(var, tree) \
	for((var) = AG_TAILQ_FIRST(&(tree)->list); \
	    (var) != AG_TAILQ_END(&(tree)->list); \
	    (var) = AG_TAILQ_NEXT((var), list))

/* Iterate over data of all items. */
#define AGTREE_FOREACH_DATA(var, tree, t) \
	for((var) = (struct t *)(AG_TAILQ_FIRST(&(tree)->list))->p; \
	    (var) != (struct t *)(AG_TAILQ_END(&(tree)->list))->p; \
	    (var) = AG_TAILQ_NEXT((var), list))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_CORE)
# define TREE_FOREACH_CHILD_ITEM(var,item)	AGTREE_FOREACH_CHILD_ITEM((var),(item))
# define TREE_FOREACH_CHILD_DATA(var,item,t)	AGTREE_FOREACH_CHILD_DATA((var),(item),t)
# define TREE_FOREACH_ITEM(var,tree)		AGTREE_FOREACH_ITEM((var),(tree))
# define TREE_FOREACH_DATA(var,tree,t)		AGTREE_FOREACH_ITEM_DATA((var),(tree),t)
#endif

__BEGIN_DECLS
AG_Tree *_Nonnull AG_TreeNew(void); /* _Malloc_Like_Attribute */
void              AG_TreeDestroy(AG_Tree *_Nonnull);

static __inline__ AG_TreeItem *_Nonnull
AG_TreeItemNew(void *_Nullable p, size_t privDataSize)
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

static __inline__ void
AG_TreeItemDestroy(AG_TreeItem *_Nonnull ti)
{
#ifdef DEBUG
	if (ti->parentItem != NULL || ti->parentTree != NULL)
		AG_FatalError("Attempt to destroy referenced TreeItem");
#endif
	AG_Free(ti->privData);
	AG_Free(ti);
}

static __inline__ void
AG_TreeAttach(AG_TreeItem *_Nonnull tiParent, AG_TreeItem *_Nonnull ti)
{
	ti->parentTree = tiParent->parentTree;
	ti->parentItem = tiParent;
	AG_TAILQ_INSERT_TAIL(&ti->parentTree->list, ti, list);
	AG_TAILQ_INSERT_TAIL(&tiParent->chldItems, ti, tree);
}

static __inline__ void
AG_TreeDetach(AG_Tree *_Nonnull t, AG_TreeItem *_Nonnull ti)
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

static __inline__ AG_TreeItem *_Nonnull
AG_TreeInsert(AG_Tree *_Nonnull t, AG_TreeItem *_Nullable tiParent,
    void *_Nullable p, size_t privDataSize)
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

static __inline__ void
AG_TreeRemove(AG_Tree *_Nonnull t, AG_TreeItem *_Nonnull ti)
{
	AG_TreeDetach(t, ti);
	AG_TreeItemDestroy(ti);
}

static __inline__ void
AG_TreeClear(AG_Tree *_Nonnull t)
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
__END_DECLS

#include <agar/core/close.h>
#endif	/* _AGAR_CORE_TREE_H_ */
