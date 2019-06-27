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

#define AG_TLIST_ITEM_TAG "AgTlItm"
#define AG_TLIST_ITEM_TAG_LEN 8

/* Popup menu (TODO switch to AG_PopupMenu) */
typedef struct ag_tlist_popup {
	const char  *_Nonnull  iclass;	/* Apply to items of this class */
	AG_Menu     *_Nonnull  menu;	/* The popup menu proper */
	AG_MenuItem *_Nonnull  item;	/* Root of the popup menu */
	AG_Window   *_Nullable panel;	/* Created by AG_MenuExpand() */
	AG_TAILQ_ENTRY(ag_tlist_popup) popups;
} AG_TlistPopup;

/* A tree/list item */
typedef struct ag_tlist_item {
#ifdef AG_TYPE_SAFETY
	char tag[AG_TLIST_ITEM_TAG_LEN];       /* Tagged non-object */
#endif
	int selected;			/* Effective selection flag */

	AG_Surface *_Nullable iconsrc;	/* Source icon */
	int                   icon;	/* Cached icon surface */
	void       *_Nullable p1;	/* User pointer */
	const char *_Nullable cat;	/* Category for filter */

	int label;			/* Cached label surface */
	Uint depth;			/* Indent in tree display */
	Uint flags;
#define AG_TLIST_ITEM_EXPANDED  0x001	/* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN   0x002	/* Child items exist (tree) */
#define AG_TLIST_NO_SELECT      0x008	/* Item is not selectable */
#define AG_TLIST_NO_POPUP       0x010	/* Disable popups for item */
#define AG_TLIST_ITEM_BOLD      0x020	/* Render font in bold weight */
#define AG_TLIST_ITEM_ITALIC    0x040	/* Render font in italic */
#define AG_TLIST_ITEM_UNDERLINE 0x080	/* Render font in underline */
#define AG_TLIST_ITEM_UPPERCASE 0x100	/* Render font in uppercase */
#define AG_TLIST_ITEM_STYLE     (AG_TLIST_ITEM_BOLD | \
				 AG_TLIST_ITEM_ITALIC | \
				 AG_TLIST_ITEM_UNDERLINE | \
				 AG_TLIST_ITEM_UPPERCASE)

	AG_TAILQ_ENTRY(ag_tlist_item) items;	/* Items in list */
	AG_TAILQ_ENTRY(ag_tlist_item) selitems;	/* Saved selection state */
	
	char text[AG_TLIST_LABEL_MAX];	/* Label text */
	AG_Color *_Nullable color;	/* Alternate text-color */
	AG_Font *_Nullable font;	/* Alternate font */
} AG_TlistItem;

typedef AG_TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item) AG_TlistItemQ;

/* Tree/list widget */
typedef struct ag_tlist {
	struct ag_widget wid;		/* AG_Widget -> AG_Tlist */
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
	AG_Rect r;			/* Clipping rectangle */
	int wSpace;			/* Icon/text spacing */
	int item_h;			/* Item height */
	int icon_w;			/* Item icon width */
	int wRow;			/* Row width */
	int rOffs;			/* Row display offset */
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
	AG_Timer refreshTo;		/* Timer for polled mode updates */
	AG_Timer dblClickTo;		/* Timer for detecting double clicks */
	int lastKeyDown;		/* For key repeat */
} AG_Tlist;

#define AGTLIST(obj)            ((AG_Tlist *)(obj))
#define AGCTLIST(obj)           ((const AG_Tlist *)(obj))
#define AG_TLIST_SELF()          AGTLIST( AG_OBJECT(0,"AG_Widget:AG_Tlist:*") )
#define AG_TLIST_PTR(n)          AGTLIST( AG_OBJECT((n),"AG_Widget:AG_Tlist:*") )
#define AG_TLIST_NAMED(n)        AGTLIST( AG_OBJECT_NAMED((n),"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_SELF()   AGCTLIST( AG_CONST_OBJECT(0,"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_PTR(n)   AGCTLIST( AG_CONST_OBJECT((n),"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_NAMED(n) AGCTLIST( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Tlist:*") )

/* NOTE: AG_TlistItem is a tagged non-object. */
#ifdef AG_TYPE_SAFETY
# define AG_TLIST_ITEM_PTR(v) \
    (v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
     !(event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
     strncmp(AGTLISTITEM(event->argv[v].data.p)->tag, AG_TLIST_ITEM_TAG, AG_TLIST_ITEM_TAG_LEN) == 0) ? \
     event->argv[v].data.p : AG_ObjectMismatch()
# define AG_CONST_TLIST_ITEM_PTR(v) \
    (v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
     (event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
     strncmp(AGTLISTITEM(event->argv[v].data.p)->tag, AG_TLIST_ITEM_TAG, AG_TLIST_ITEM_TAG_LEN) == 0) ? \
     event->argv[v].data.p : AG_ObjectMismatch()
#else
# define AG_TLIST_ITEM_PTR(v)       event->argv[v].data.p
# define AG_CONST_TLIST_ITEM_PTR(v) event->argv[v].data.p
#endif
#define AGTLISTITEM(p)               ((AG_TlistItem *)(p))
#define AG_TLIST_ITEM_SELF()         AG_TLIST_ITEM_PTR(0)
#define AG_CONST_TLIST_ITEM_SELF()   AG_CONST_TLIST_ITEM_PTR(0)

#define AG_TLIST_FOREACH(it, tl) \
	AG_TAILQ_FOREACH(it, &(tl)->items, items)

#define AG_TLIST_FOREACH_ITEM(p, tl, it, t)				\
	for((it) = AG_TAILQ_FIRST(&(tl)->items),			\
	     (p) = (it)!=NULL ? (struct t *)(it)->p1 : NULL;		\
	    (it) != AG_TAILQ_END(&(tl)->children) && (it)->p1 != NULL;	\
	    (it) = AG_TAILQ_NEXT((it), cobjs),				\
	     (p) = (it)!=NULL ? (struct t *)(it)->p1 : NULL)

__BEGIN_DECLS
extern AG_WidgetClass agTlistClass;

AG_Tlist *_Nonnull AG_TlistNew(void *_Nullable, Uint);
#ifdef AG_TIMERS
AG_Tlist *_Nonnull AG_TlistNewPolled(void *_Nullable, Uint,
                                     _Nonnull AG_EventFn,
				     const char *_Nullable, ...);
#endif

void AG_TlistSizeHint(AG_Tlist *_Nonnull, const char *_Nonnull, int);
void AG_TlistSizeHintPixels(AG_Tlist *_Nonnull, int,int);
void AG_TlistSizeHintLargest(AG_Tlist *_Nonnull, int);

void AG_TlistSetItemHeight(AG_Tlist *_Nonnull, int);
void AG_TlistSetIconWidth(AG_Tlist *_Nonnull, int);

#ifdef AG_TIMERS
void AG_TlistSetRefresh(AG_Tlist *_Nonnull, int);
#endif
void AG_TlistUniq(AG_Tlist *_Nonnull);

void AG_TlistClear(AG_Tlist *_Nonnull);
void AG_TlistRestore(AG_Tlist *_Nonnull);

void AG_TlistBegin(AG_Tlist *_Nonnull);
void AG_TlistEnd(AG_Tlist *_Nonnull);

AG_TlistItem *_Nonnull AG_TlistItemNew(AG_Tlist *_Nonnull,
                                       const AG_Surface *_Nullable);

AG_TlistItem *_Nonnull AG_TlistAddS(AG_Tlist *_Nonnull,
                                    const AG_Surface *_Nullable,
                                    const char *_Nonnull);

AG_TlistItem *_Nonnull AG_TlistAdd(AG_Tlist *_Nonnull,
                                   const AG_Surface *_Nullable,
                                   const char *_Nonnull, ...)
				  FORMAT_ATTRIBUTE(printf,3,4);

AG_TlistItem *_Nonnull AG_TlistAddHeadS(AG_Tlist *_Nonnull,
                                        const AG_Surface *_Nullable,
                                        const char *_Nonnull);

AG_TlistItem *_Nonnull AG_TlistAddHead(AG_Tlist *_Nonnull,
                                       const AG_Surface *_Nullable,
                                       const char *_Nonnull, ...)
				      FORMAT_ATTRIBUTE(printf,3,4);

AG_TlistItem *_Nonnull AG_TlistAddPtr(AG_Tlist *_Nonnull,
                                      const AG_Surface *_Nullable,
                                      const char *_Nonnull, void *_Nullable);

AG_TlistItem *_Nonnull AG_TlistAddPtrHead(AG_Tlist *_Nonnull,
                                          const AG_Surface *_Nullable,
                                          const char *_Nonnull, void *_Nullable);

void AG_TlistSetIcon(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     const AG_Surface *_Nullable);
void AG_TlistSetColor(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                      const AG_Color *_Nullable);
void AG_TlistSetFont(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     AG_Font *_Nullable);

void AG_TlistDel(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);

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
                          int (*_Nonnull)(const AG_TlistItem *_Nonnull,
			                  const AG_TlistItem *_Nonnull));
int  AG_TlistCompareStrings(const AG_TlistItem *_Nonnull,
                            const AG_TlistItem *_Nonnull)
                           _Pure_Attribute;
int  AG_TlistComparePtrs(const AG_TlistItem *_Nonnull,
                         const AG_TlistItem *_Nonnull)
                        _Pure_Attribute;
int  AG_TlistComparePtrsAndClasses(const AG_TlistItem *_Nonnull,
                                   const AG_TlistItem *_Nonnull)
                                  _Pure_Attribute;

int  AG_TlistSort(AG_Tlist *_Nonnull);
int  AG_TlistVisibleChildren(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistRefresh(AG_Tlist *_Nonnull);

#ifdef AG_LEGACY
#define AG_TLIST_EXPANDED         AG_TLIST_ITEM_EXPANDED
#define AG_TLIST_VISIBLE_CHILDREN AG_TLIST_ITEM_EXPANDED
#define AG_TlistPrescale(tl,text,n) AG_TlistSizeHint((tl),(text),(n))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TLIST_H_ */
