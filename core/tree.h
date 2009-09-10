/*
 * Copyright (c) 2008-2009 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * General-purpose tree structure (not to confuse with binary trees as in
 * btree.h).
 *
 * Compared to the AG_Queue(3) macro package, using this interface to
 * implement a tree structure has the advantage of the tree being directly
 * traversable (e.g., by a GUI widget such as AG_TreeView(3)), in a generic
 * fashion. However, this comes at the cost of an extra level of indirection
 * on each element.
 */

#ifndef _AGAR_CORE_TREE_H_
#define _AGAR_CORE_TREE_H_
#include <agar/core/begin.h>

struct ag_tree;
struct ag_tree_item;

AG_TAILQ_HEAD(ag_tree_itemq, ag_tree_item);

typedef struct ag_tree_item {
	void *p;				/* User pointer */
	void *privData;				/* Private data (or NULL) */
	size_t privDataSize;			/* Private data size */
	struct ag_tree_itemq chldItems;		/* Child items */
	AG_TAILQ_ENTRY(ag_tree_item) tree;	/* Entry in tree */
	AG_TAILQ_ENTRY(ag_tree_item) list;	/* Entre in list */
	struct ag_tree *parentTree;		/* Parent tree */
	struct ag_tree_item *parentItem;	/* Parent item */
} AG_TreeItem;

typedef struct ag_tree {
	AG_TreeItem *root;
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
AG_Tree   *AG_TreeNew(void);
void       AG_TreeDestroy(AG_Tree *);

static __inline__ AG_TreeItem *
AG_TreeItemNew(void *p, size_t privDataSize)
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
AG_TreeItemDestroy(AG_TreeItem *ti)
{
#ifdef DEBUG
	if (ti->parentItem != NULL || ti->parentTree != NULL)
		AG_FatalError("Attempt to destroy referenced TreeItem");
#endif
	AG_Free(ti->privData);
	AG_Free(ti);
}

static __inline__ void
AG_TreeAttach(AG_TreeItem *tiParent, AG_TreeItem *ti)
{
	ti->parentTree = tiParent->parentTree;
	ti->parentItem = tiParent;
	AG_TAILQ_INSERT_TAIL(&ti->parentTree->list, ti, list);
	AG_TAILQ_INSERT_TAIL(&tiParent->chldItems, ti, tree);
}

static __inline__ void
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

static __inline__ AG_TreeItem *
AG_TreeInsert(AG_Tree *t, AG_TreeItem *tiParent, void *p, size_t privDataSize)
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
AG_TreeRemove(AG_Tree *t, AG_TreeItem *ti)
{
	AG_TreeDetach(t, ti);
	AG_TreeItemDestroy(ti);
}

static __inline__ void
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
__END_DECLS

#include <agar/core/close.h>
#endif	/* _AGAR_CORE_TREE_H_ */
