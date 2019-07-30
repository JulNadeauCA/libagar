/*	Public domain	*/
/*
 * Generically-accessible, ordered tree of opaque data elements.
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
	AG_Size privDataSize;			/* Private data size */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
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
AG_Tree *_Nonnull     AG_TreeNew(void); /* _Malloc_Like_Attribute */
void                  AG_TreeDestroy(AG_Tree *_Nonnull);

AG_TreeItem *_Nonnull AG_TreeItemNew(void *_Nullable, AG_Size);
void                  AG_TreeItemDestroy(AG_TreeItem *_Nonnull);

void                  AG_TreeAttach(AG_TreeItem *_Nonnull, AG_TreeItem *_Nonnull);
void                  AG_TreeDetach(AG_Tree *_Nonnull, AG_TreeItem *_Nonnull);

AG_TreeItem *_Nonnull AG_TreeInsert(AG_Tree *_Nonnull, AG_TreeItem *_Nullable,
                                    void *_Nullable, AG_Size);
void                  AG_TreeRemove(AG_Tree *_Nonnull, AG_TreeItem *_Nonnull);
void                  AG_TreeClear(AG_Tree *_Nonnull);
__END_DECLS

#include <agar/core/close.h>
#endif	/* _AGAR_CORE_TREE_H_ */
