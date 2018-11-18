/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/label.h>
#include <agar/gui/begin.h>

#ifndef AG_TLIST_LABEL_MAX
#define AG_TLIST_LABEL_MAX AG_LABEL_MAX
#endif

typedef struct ag_tlist_popup {
	const char  *_Nonnull  iclass;	/* Apply to items of this class */
	AG_Menu     *_Nonnull  menu;	/* The popup menu proper */
	AG_MenuItem *_Nonnull  item;	/* Root of the popup menu */
	AG_Window   *_Nullable panel;	/* Created by AG_MenuExpand() */
	AG_TAILQ_ENTRY(ag_tlist_popup) popups;
} AG_TlistPopup;

typedef struct ag_tlist_item {
	int selected;			/* Effective selection flag */
	AG_Surface *_Nullable iconsrc;	/* Source icon */
	int                   icon;	/* Cached icon surface */
	void       *_Nullable p1;	/* User-supplied pointer */
	const char *_Nullable cat;	/* User-supplied category */

	char text[AG_TLIST_LABEL_MAX];	/* Label text */
	int label;			/* Cached label surface */
	Uint depth;			/* Indent in tree display */
	Uint flags;
#define AG_TLIST_EXPANDED     0x01	/* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN 0x02	/* Child items exist (tree) */
#define AG_TLIST_DYNICON      0x04	/* Use a copy of iconsrc */
#define AG_TLIST_NO_SELECT    0x08	/* Item is not selectable */
#define AG_TLIST_NO_POPUP     0x10	/* Disable popups for item */
#define AG_TLIST_VISIBLE_CHILDREN AG_TLIST_EXPANDED

	AG_TAILQ_ENTRY(ag_tlist_item) items;	/* Items in list */
	AG_TAILQ_ENTRY(ag_tlist_item) selitems;	/* Saved selection state */
} AG_TlistItem;

typedef AG_TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item) AG_TlistItemQ;

typedef struct ag_tlist {
	struct ag_widget wid;
	Uint flags;
#define AG_TLIST_MULTI		0x001	/* Multiple selections (ctrl/shift) */
#define AG_TLIST_MULTITOGGLE	0x002	/* Multiple toggle-style selections */
#define AG_TLIST_POLL		0x004	/* Generate tlist-poll events */
#define AG_TLIST_TREE		0x010	/* Hack to display trees */
#define AG_TLIST_HFILL		0x020
#define AG_TLIST_VFILL		0x040
#define AG_TLIST_NOSELSTATE	0x100	/* Don't preserve sel state in poll */
#define AG_TLIST_SCROLLTOSEL	0x200	/* Scroll to initial selection */
#define AG_TLIST_REFRESH	0x400	/* Repopulate display (for polling) */
#define AG_TLIST_EXPAND		(AG_TLIST_HFILL|AG_TLIST_VFILL)

	void *_Nullable selected;	/* Default `selected' binding */
	int wHint, hHint;		/* Size hint */
	int wSpace;			/* Icon/text spacing */
	int item_h;			/* Item height */
	int icon_w;			/* Item icon width */
	void *_Nullable dblClicked;	/* For double click test */
	AG_TlistItemQ items;		/* Current Items */
	AG_TlistItemQ selitems;		/* Saved item state */
	int nitems;			/* Current item count */
	int nvisitems;			/* Visible item count */
	AG_Scrollbar *_Nonnull sbar;	/* Vertical scrollbar */
	AG_TAILQ_HEAD_(ag_tlist_popup) popups; /* Popup menus */

	int (*_Nonnull compare_fn)(const AG_TlistItem *_Nonnull,
	                           const AG_TlistItem *_Nonnull);

	AG_Event *_Nullable popupEv;	/* Popup menu hook */
	AG_Event *_Nullable changedEv;	/* Selection change hook */
	AG_Event *_Nullable dblClickEv;	/* Double click hook */
	AG_Timer moveTo;		/* Timer for keyboard motion */
	Uint32 wheelTicks;		/* For wheel acceleration */
	int wRow;			/* Row width */
	AG_Rect r;			/* View area */
	AG_Timer refreshTo;		/* Timer for polled mode updates */
	int rOffs;			/* Row display offset */
	AG_Timer dblClickTo;		/* Timer for detecting double clicks */
	int lastKeyDown;		/* For key repeat */
} AG_Tlist;

#define AG_TLIST_FOREACH(it, tl) \
	AG_TAILQ_FOREACH(it, &(tl)->items, items)

#define AG_TLIST_FOREACH_ITEM(p, tl, it, t)				\
	for((it) = AG_TAILQ_FIRST(&(tl)->items),			\
	     (p) = (it)!=NULL ? (struct t *)(it)->p1 : NULL;		\
	    (it) != AG_TAILQ_END(&(tl)->children) && (it)->p1 != NULL;	\
	    (it) = AG_TAILQ_NEXT((it), cobjs),				\
	     (p) = (it)!=NULL ? (struct t *)(it)->p1 : NULL)

#define AG_TLIST_ITEM(n) AG_TlistSelectedItemPtr(AG_PTR(n))

__BEGIN_DECLS
extern AG_WidgetClass agTlistClass;

AG_Tlist *_Nonnull AG_TlistNew(void *_Nullable, Uint);
AG_Tlist *_Nonnull AG_TlistNewPolled(void *_Nullable, Uint,
                                     _Nonnull AG_EventFn,
				     const char *_Nullable, ...);

void AG_TlistSizeHint(AG_Tlist *_Nonnull, const char *_Nonnull, int);
void AG_TlistSizeHintPixels(AG_Tlist *_Nonnull, int,int);
void AG_TlistSizeHintLargest(AG_Tlist *_Nonnull, int);

void AG_TlistSetItemHeight(AG_Tlist *_Nonnull, int);
void AG_TlistSetIconWidth(AG_Tlist *_Nonnull, int);
void AG_TlistSetIcon(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     AG_Surface *_Nullable);
void AG_TlistSetRefresh(AG_Tlist *_Nonnull, int);

void AG_TlistDel(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistUniq(AG_Tlist *_Nonnull);
void AG_TlistClear(AG_Tlist *_Nonnull);
void AG_TlistRestore(AG_Tlist *_Nonnull);

AG_TlistItem *_Nonnull AG_TlistAddS(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                    const char *_Nonnull);
AG_TlistItem *_Nonnull AG_TlistAdd(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                   const char *_Nonnull, ...)
				  FORMAT_ATTRIBUTE(printf,3,4);

AG_TlistItem *_Nonnull AG_TlistAddHeadS(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                        const char *_Nonnull);
AG_TlistItem *_Nonnull AG_TlistAddHead(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                       const char *_Nonnull, ...)
				      FORMAT_ATTRIBUTE(printf,3,4);

AG_TlistItem *_Nonnull AG_TlistAddPtr(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                      const char *_Nonnull, void *_Nullable);
AG_TlistItem *_Nonnull AG_TlistAddPtrHead(AG_Tlist *_Nonnull, AG_Surface *_Nullable,
                                          const char *_Nonnull, void *_Nullable);

void AG_TlistSelect(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistDeselect(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistSelectAll(AG_Tlist *_Nonnull);
void AG_TlistDeselectAll(AG_Tlist *_Nonnull);

AG_TlistItem *_Nullable AG_TlistSelectPtr(AG_Tlist *_Nonnull, void *_Nullable);
AG_TlistItem *_Nullable AG_TlistSelectText(AG_Tlist *_Nonnull, const char *_Nonnull);

AG_TlistItem *_Nullable AG_TlistFindByIndex(AG_Tlist *_Nonnull, int) _Pure_Attribute_If_Unthreaded;
AG_TlistItem *_Nullable AG_TlistSelectedItem(AG_Tlist *_Nonnull) _Pure_Attribute_If_Unthreaded;
void *_Nullable         AG_TlistSelectedItemPtr(AG_Tlist *_Nonnull) _Pure_Attribute_If_Unthreaded;
AG_TlistItem *_Nullable AG_TlistFirstItem(AG_Tlist *_Nonnull) _Pure_Attribute_If_Unthreaded;
AG_TlistItem *_Nullable AG_TlistLastItem(AG_Tlist *_Nonnull) _Pure_Attribute_If_Unthreaded;
void *_Nullable         AG_TlistFindPtr(AG_Tlist *_Nonnull) _Pure_Attribute_If_Unthreaded;
AG_TlistItem *_Nullable AG_TlistFindText(AG_Tlist *_Nonnull, const char *_Nonnull)
                                        _Pure_Attribute_If_Unthreaded;

AG_MenuItem *_Nonnull AG_TlistSetPopup(AG_Tlist *_Nonnull, const char *_Nonnull);

void AG_TlistScrollToStart(AG_Tlist *_Nonnull);
void AG_TlistScrollToEnd(AG_Tlist *_Nonnull);

void AG_TlistSetDblClickFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                           const char *_Nullable, ...);
void AG_TlistSetPopupFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                        const char *_Nullable, ...);
void AG_TlistSetChangedFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                          const char *_Nullable, ...);

void AG_TlistSetCompareFn(AG_Tlist *_Nonnull,
                          int (*_Nonnull)(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull));
int AG_TlistCompareStrings(const AG_TlistItem *_Nonnull,
                           const AG_TlistItem *_Nonnull)
                          _Pure_Attribute;
int AG_TlistComparePtrs(const AG_TlistItem *_Nonnull,
                        const AG_TlistItem *_Nonnull)
                       _Pure_Attribute;
int AG_TlistComparePtrsAndClasses(const AG_TlistItem *_Nonnull,
                                  const AG_TlistItem *_Nonnull)
                                 _Pure_Attribute;
int AG_TlistSort(AG_Tlist *_Nonnull);

#define AG_TlistBegin(tl) AG_TlistClear(tl)
#define AG_TlistEnd(tl)   AG_TlistRestore(tl)

#ifdef AG_LEGACY
#define AG_TlistPrescale(tl,text,nitems) AG_TlistSizeHint((tl),(text),(nitems))
#endif

static __inline__ int
AG_TlistVisibleChildren(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull cit)
{
	AG_TlistItem *sit;

	AG_TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tl->compare_fn(sit, cit))
			break;
	}
	if (sit == NULL) { 
		return (0);			/* TODO default setting */
	}
	return (sit->flags & AG_TLIST_VISIBLE_CHILDREN);
}

static __inline__ void
AG_TlistRefresh(AG_Tlist *_Nonnull tl)
{
	AG_ObjectLock(tl);
	tl->flags |= AG_TLIST_REFRESH;
	AG_ObjectUnlock(tl);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TLIST_H_ */
