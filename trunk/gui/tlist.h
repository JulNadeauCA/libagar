/*	$Csoft: tlist.h,v 1.51 2005/08/29 03:13:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <engine/widget/scrollbar.h>
#include <engine/widget/menu.h>
#include <engine/widget/label.h>

#include "begin_code.h"

#define AG_TLIST_LABEL_MAX AG_LABEL_MAX
#define AG_TLIST_ARGS_MAX 8

struct ag_tlist_popup {
	const char *iclass;		/* Apply to items of this class */
	AG_Menu *menu;
	AG_MenuItem *item;
	AG_Window *panel;
	TAILQ_ENTRY(ag_tlist_popup) popups;
};

typedef struct ag_tlist_item {
	int selected;				/* Effective selection */
	SDL_Surface *iconsrc;			/* Source icon (or NULL) */
	int icon;				/* Cached icon surface */
	void *p1;				/* User-supplied pointer */
	const char *class;			/* User-supplied class */
	char text[AG_TLIST_LABEL_MAX];		/* Label text */
	int label;				/* Cached label surface */
	union evarg argv[AG_TLIST_ARGS_MAX];	/* Item arguments */
	int argc;
	Uint8 depth;				/* Depth in tree */
	Uint8 flags;
#define AG_TLIST_VISIBLE_CHILDREN 0x01	/* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN	  0x02	/* Child items exist (tree) */
#define AG_TLIST_DYNICON	  0x04	/* Use a copy of iconsrc */

	TAILQ_ENTRY(ag_tlist_item) items;	/* Items in list */
	TAILQ_ENTRY(ag_tlist_item) selitems;	/* Saved selection state */
} AG_TlistItem;

TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item);

typedef struct ag_tlist {
	struct ag_widget wid;
	int flags;
#define AG_TLIST_MULTI		0x01	/* Multiple selections (ctrl/shift) */
#define AG_TLIST_MULTITOGGLE	0x02	/* Multiple toggle-style selections */
#define AG_TLIST_POLL		0x04	/* Generate tlist-poll events */
#define AG_TLIST_TREE		0x10	/* Hack to display trees */
	void *selected;			/* Default `selected' binding */
	int prew, preh;			/* Prescale hint */

	pthread_mutex_t	lock;
	int item_h;			/* Item height */
	int icon_w;			/* Item icon width */
	void *dblclicked;		/* Used by double click */
	int  keymoved;			/* Used by key repeat */
	struct ag_tlist_itemq items;	/* Current Items */
	struct ag_tlist_itemq selitems;	/* Saved item state */
	int nitems;			/* Current item count */
	int nvisitems;			/* Visible item count */
	AG_Scrollbar *sbar;		/* Vertical scrollbar */
	TAILQ_HEAD(,ag_tlist_popup) popups; /* Popup menus */
	int (*compare_fn)(const AG_TlistItem *, const AG_TlistItem *);
} AG_Tlist;

/* Traverse the user pointer of tlist items assumed to be of the same type. */
#define AG_TLIST_FOREACH_ITEM(p, tl, it, type)				\
	for((it) = TAILQ_FIRST(&(tl)->items),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL;		\
	    (it) != TAILQ_END(&(tl)->children) && (it)->p1 != NULL;	\
	    (it) = TAILQ_NEXT((it), cobjs),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL)

__BEGIN_DECLS
AG_Tlist *AG_TlistNew(void *, int);
void	  AG_TlistInit(AG_Tlist *, int);
void	  AG_TlistScale(void *, int, int);
void	  AG_TlistDraw(void *);
void	  AG_TlistDestroy(void *);

void		AG_TlistPrescale(AG_Tlist *, const char *, int);
void		AG_TlistSetItemHeight(AG_Tlist *, int);
__inline__ void	AG_TlistSetIcon(AG_Tlist *, AG_TlistItem *,
	                        SDL_Surface *);

__inline__ int AG_TlistVisibleChildren(AG_Tlist *, AG_TlistItem *);
void	       AG_TlistSetArgs(AG_TlistItem *, const char *, ...);
void	       AG_TlistDel(AG_Tlist *, AG_TlistItem *);
void	       AG_TlistClear(AG_Tlist *);
void	       AG_TlistRestore(AG_Tlist *);
AG_TlistItem  *AG_TlistAdd(AG_Tlist *, SDL_Surface *, const char *, ...);
AG_TlistItem  *AG_TlistAddPtr(AG_Tlist *, SDL_Surface *, const char *, void *);
AG_TlistItem  *AG_TlistAddPtrHead(AG_Tlist *, SDL_Surface *, const char *,
	                         void *);
void		 AG_TlistSelect(AG_Tlist *, AG_TlistItem *);
void		 AG_TlistDeselect(AG_Tlist *, AG_TlistItem *);
void		 AG_TlistSelectAll(AG_Tlist *);
AG_TlistItem	*AG_TlistSelectPtr(AG_Tlist *, void *);
AG_TlistItem	*AG_TlistSelectText(AG_Tlist *, const char *);
void		 AG_TlistDeselectAll(AG_Tlist *);
AG_TlistItem	*AG_TlistFindByIndex(AG_Tlist *, int);
AG_TlistItem	*AG_TlistSelectedItem(AG_Tlist *);
void		*AG_TlistFindPtr(AG_Tlist *);
AG_TlistItem	*AG_TlistFindText(AG_Tlist *, const char *);
AG_TlistItem	*AG_TlistFirstItem(AG_Tlist *);
AG_TlistItem	*AG_TlistLastItem(AG_Tlist *);
AG_MenuItem	*AG_TlistSetPopup(AG_Tlist *, const char *);

void AG_TlistSetCompareFn(AG_Tlist *, int (*)(const AG_TlistItem *,
		          const AG_TlistItem *));
int AG_TlistCompareStrings(const AG_TlistItem *, const AG_TlistItem *);
int AG_TlistComparePtrs(const AG_TlistItem *, const AG_TlistItem *);
int AG_TlistComparePtrsAndClasses(const AG_TlistItem *, const AG_TlistItem *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TLIST_H_ */
