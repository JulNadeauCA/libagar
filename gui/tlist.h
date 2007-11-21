/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#ifdef _AGAR_INTERNAL
#include <gui/scrollbar.h>
#include <gui/menu.h>
#include <gui/label.h>
#else
#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/label.h>
#endif

#include "begin_code.h"

#define AG_TLIST_LABEL_MAX AG_LABEL_MAX
#define AG_TLIST_ARGS_MAX 8

typedef struct ag_tlist_popup {
	const char *iclass;		/* Apply to items of this class */
	AG_Menu *menu;
	AG_MenuItem *item;
	AG_Window *panel;
	TAILQ_ENTRY(ag_tlist_popup) popups;
} AG_TlistPopup;

typedef struct ag_tlist_item {
	int selected;				/* Effective selection */
	SDL_Surface *iconsrc;			/* Source icon (or NULL) */
	int icon;				/* Cached icon surface */
	void *p1;				/* User-supplied pointer */
	const char *cat;			/* User-supplied category */
	char text[AG_TLIST_LABEL_MAX];		/* Label text */
	int label;				/* Cached label surface */
	union evarg argv[AG_TLIST_ARGS_MAX];	/* Item arguments */
	int argc;
	Uint8 depth;				/* Depth in tree */
	Uint8 flags;
#define AG_TLIST_EXPANDED	  0x01	/* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN	  0x02	/* Child items exist (tree) */
#define AG_TLIST_DYNICON	  0x04	/* Use a copy of iconsrc */
#define AG_TLIST_NO_SELECT	  0x08	/* Item is not selectable */
#define AG_TLIST_NO_POPUP	  0x10	/* Disable popups for item */
#define AG_TLIST_VISIBLE_CHILDREN AG_TLIST_EXPANDED

	TAILQ_ENTRY(ag_tlist_item) items;	/* Items in list */
	TAILQ_ENTRY(ag_tlist_item) selitems;	/* Saved selection state */
} AG_TlistItem;

TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item);

typedef struct ag_tlist {
	struct ag_widget wid;
	Uint flags;
#define AG_TLIST_MULTI		0x001	/* Multiple selections (ctrl/shift) */
#define AG_TLIST_MULTITOGGLE	0x002	/* Multiple toggle-style selections */
#define AG_TLIST_POLL		0x004	/* Generate tlist-poll events */
#define AG_TLIST_TREE		0x010	/* Hack to display trees */
#define AG_TLIST_HFILL		0x020
#define AG_TLIST_VFILL		0x040
#define AG_TLIST_NOSELSTATE	0x100	/* Don't restore previous selection
					   state after poll operation */
#define AG_TLIST_EXPAND		(AG_TLIST_HFILL|AG_TLIST_VFILL)

	void *selected;			/* Default `selected' binding */
	int wHint, hHint;		/* Size hint */
	int wSpace;			/* Icon/text spacing */

	AG_Mutex lock;
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
	AG_Event *popupEv;
	AG_Event *changedEv;
	AG_Event *dblClickEv;
} AG_Tlist;

#define AG_TLIST_FOREACH(it, tl) \
	TAILQ_FOREACH(it, &(tl)->items, items)

#define AG_TLIST_FOREACH_ITEM(p, tl, it, type)				\
	for((it) = TAILQ_FIRST(&(tl)->items),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL;		\
	    (it) != TAILQ_END(&(tl)->children) && (it)->p1 != NULL;	\
	    (it) = TAILQ_NEXT((it), cobjs),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL)

#define AG_TLIST_ITEM(n) AG_TlistSelectedItemPtr(AG_PTR(n))

__BEGIN_DECLS
extern AG_WidgetClass agTlistClass;

AG_Tlist *AG_TlistNew(void *, Uint);
AG_Tlist *AG_TlistNewPolled(void *, Uint, AG_EventFn, const char *, ...);

void		AG_TlistSizeHint(AG_Tlist *, const char *, int);
void		AG_TlistSizeHintPixels(AG_Tlist *, int, int);
#define		AG_TlistPrescale AG_TlistSizeHint

void		AG_TlistSetItemHeight(AG_Tlist *, int);
void		AG_TlistSetIcon(AG_Tlist *, AG_TlistItem *, SDL_Surface *);

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
void		*AG_TlistSelectedItemPtr(AG_Tlist *);
void		*AG_TlistFindPtr(AG_Tlist *);
AG_TlistItem	*AG_TlistFindText(AG_Tlist *, const char *);
AG_TlistItem	*AG_TlistFirstItem(AG_Tlist *);
AG_TlistItem	*AG_TlistLastItem(AG_Tlist *);
AG_MenuItem	*AG_TlistSetPopup(AG_Tlist *, const char *);

void AG_TlistSetDblClickFn(AG_Tlist *, AG_EventFn, const char *, ...);
void AG_TlistSetPopupFn(AG_Tlist *, AG_EventFn, const char *, ...);
void AG_TlistSetChangedFn(AG_Tlist *, AG_EventFn, const char *, ...);
void AG_TlistSetCompareFn(AG_Tlist *, int (*)(const AG_TlistItem *,
		          const AG_TlistItem *));
int AG_TlistCompareStrings(const AG_TlistItem *, const AG_TlistItem *);
int AG_TlistComparePtrs(const AG_TlistItem *, const AG_TlistItem *);
int AG_TlistComparePtrsAndClasses(const AG_TlistItem *, const AG_TlistItem *);

#define AG_TlistBegin AG_TlistClear
#define AG_TlistEnd AG_TlistRestore

static __inline__ int
AG_TlistVisibleChildren(AG_Tlist *tl, AG_TlistItem *cit)
{
	AG_TlistItem *sit;

	TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tl->compare_fn(sit, cit))
			break;
	}
	if (sit == NULL) { 
		return (0);			/* TODO default setting */
	}
	return (sit->flags & AG_TLIST_VISIBLE_CHILDREN);
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TLIST_H_ */
