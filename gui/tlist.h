/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/label.h>
#include <agar/gui/begin.h>

#ifndef AG_TLIST_LABEL_MAX
# if AG_MODEL == AG_MEDIUM
#  define AG_TLIST_LABEL_MAX 252
# elif AG_MODEL == AG_LARGE
#  define AG_TLIST_LABEL_MAX 508
# endif
#endif

#define AG_TLIST_ITEM_TAG "AgTlItm"
#define AG_TLIST_ITEM_TAG_LEN 8

/* Popup menu (TODO switch to AG_PopupMenu) */
typedef struct ag_tlist_popup {
	const char *_Nonnull iclass;          /* Apply to items of this class */
	AG_Menu *_Nonnull menu;               /* The popup menu proper */
	AG_MenuItem *_Nonnull item;           /* Root of the popup menu */
	AG_Window *_Nullable panel;           /* Created by AG_MenuExpand() */
	AG_TAILQ_ENTRY(ag_tlist_popup) popups;
} AG_TlistPopup;

/* A tree/list item */
typedef struct ag_tlist_item {
#ifdef AG_TYPE_SAFETY
	char tag[AG_TLIST_ITEM_TAG_LEN];      /* Tagged non-object */
#endif
	int icon;                             /* Cached icon surface */
	int label;                            /* Cached label surface */
	const char *_Nullable cat;            /* Category for filter */
	AG_Surface *_Nullable iconsrc;        /* Icon source image */
	void *_Nullable p1;                   /* User pointer */
	AG_Color *_Nullable color;            /* Alternate color */
	AG_Font *_Nullable font;              /* Alternate font */
	int selected;                         /* Effective selection flag */
	Uint depth;                           /* Indent in tree display */
	Uint flags;
#define AG_TLIST_ITEM_EXPANDED 0x01           /* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN  0x02           /* Child items exist (tree) */
#define AG_TLIST_NO_SELECT     0x08           /* Item is not selectable */
#define AG_TLIST_NO_POPUP      0x10           /* Disable popups for item */

	Uint fontFlags;                       /* Font style; see AG_FetchFont(3) */
	char text[AG_TLIST_LABEL_MAX];        /* Label text */
	Uint32 _pad2;
	AG_TAILQ_ENTRY(ag_tlist_item) items;    /* Items in list */
	AG_TAILQ_ENTRY(ag_tlist_item) selitems; /* Saved selection state */
} AG_TlistItem;

typedef AG_TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item) AG_TlistItemQ;

/* Tree/list widget */
typedef struct ag_tlist {
	struct ag_widget wid;           /* AG_Widget -> AG_Tlist */
	Uint flags;
#define AG_TLIST_MULTI       0x001      /* Multiple selections (ctrl/shift) */
#define AG_TLIST_MULTITOGGLE 0x002      /* Multiple toggle-style selections */
#define AG_TLIST_POLL        0x004      /* Generate tlist-poll events */
#define AG_TLIST_NOSELEVENT  0x008      /* Inhibit "tlist-selected" event */
#define AG_TLIST_HFILL       0x020
#define AG_TLIST_VFILL       0x040
#define AG_TLIST_NOSELSTATE  0x100      /* Lose selection state in polled mode */
#define AG_TLIST_SCROLLTOSEL 0x200      /* Scroll to initial selection */
#define AG_TLIST_REFRESH     0x400      /* Repopulate now (polled mode) */
#define AG_TLIST_EXPAND     (AG_TLIST_HFILL | AG_TLIST_VFILL)

	int item_h;                     /* Item height */
	void *_Nullable selected;       /* Default `selected' binding */
	int wHint, hHint;               /* Size hint */
	AG_Rect r;                      /* Clipping rectangle */
	Uint32 _pad1;
	int icon_w;                     /* Item icon width */
	Uint pollDelay;                 /* Refresh rate for POLL mode */
	int rOffs;                      /* Row display offset */
	void *_Nullable dblClicked;     /* For double click test */
	AG_TlistItemQ items;            /* Current Items */
	AG_TlistItemQ selitems;         /* Saved item state */
	int nItems;                     /* Number of items total */
	int nVisible;                   /* Number of items on screen */
	AG_Scrollbar *_Nonnull sbar;    /* Vertical scrollbar */
	AG_TAILQ_HEAD_(ag_tlist_popup) popups; /* Popup menus */

	int (*_Nonnull compare_fn)(const AG_TlistItem *_Nonnull,
	                           const AG_TlistItem *_Nonnull);

	AG_Event *_Nullable popupEv;    /* Popup menu hook */
	AG_Event *_Nullable changedEv;  /* Selection change hook */
	AG_Event *_Nullable dblClickEv; /* Double click hook */
	Uint32 _pad2;
	int lastKeyDown;                /* For key repeat */
	AG_Timer moveTo;                /* Timer for keyboard motion */
	AG_Timer refreshTo;             /* Timer for polled mode updates */
	AG_Timer dblClickTo;            /* Timer for detecting double clicks */
} AG_Tlist;

#define AGTLIST(obj)            ((AG_Tlist *)(obj))
#define AGCTLIST(obj)           ((const AG_Tlist *)(obj))
#define AG_TLIST_SELF()          AGTLIST( AG_OBJECT(0,"AG_Widget:AG_Tlist:*") )
#define AG_TLIST_PTR(n)          AGTLIST( AG_OBJECT((n),"AG_Widget:AG_Tlist:*") )
#define AG_TLIST_NAMED(n)        AGTLIST( AG_OBJECT_NAMED((n),"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_SELF()   AGCTLIST( AG_CONST_OBJECT(0,"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_PTR(n)   AGCTLIST( AG_CONST_OBJECT((n),"AG_Widget:AG_Tlist:*") )
#define AG_CONST_TLIST_NAMED(n) AGCTLIST( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Tlist:*") )

#ifdef AG_TYPE_SAFETY
# define AG_TLIST_ITEM_PTR(v)       AG_TlistGetItemPtr(event,(v),0)
# define AG_CONST_TLIST_ITEM_PTR(v) ((const AG_MenuItem *)AG_TlistGetItemPtr(event,(v),1))
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
AG_Tlist *_Nonnull AG_TlistNewPolled(void *_Nullable, Uint,
                                     _Nonnull AG_EventFn,
                                     const char *_Nullable, ...);
AG_Tlist *_Nonnull AG_TlistNewPolledMs(void *_Nullable, Uint, int,
                                       _Nonnull AG_EventFn,
                                       const char *_Nullable, ...);

void AG_TlistSizeHint(AG_Tlist *_Nonnull, const char *_Nonnull, int);
void AG_TlistSizeHintPixels(AG_Tlist *_Nonnull, int,int);
void AG_TlistSizeHintLargest(AG_Tlist *_Nonnull, int);
void AG_TlistSetItemHeight(AG_Tlist *_Nonnull, int);
void AG_TlistSetIconWidth(AG_Tlist *_Nonnull, int);
void AG_TlistSetRefresh(AG_Tlist *_Nonnull, int);
void AG_TlistUniq(AG_Tlist *_Nonnull);
void AG_TlistClear(AG_Tlist *_Nonnull);
void AG_TlistRestore(AG_Tlist *_Nonnull);
void AG_TlistBegin(AG_Tlist *_Nonnull);
void AG_TlistEnd(AG_Tlist *_Nonnull);

#define AG_TlistClear(tl)   AG_TlistBegin(tl)
#define AG_TlistRestore(tl) AG_TlistEnd(tl)

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

AG_TlistItem *_Nonnull AG_TlistItemNew(const AG_Surface *_Nullable);

void AG_TlistSetIcon(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     const AG_Surface *_Nullable);
void AG_TlistSetColor(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                      const AG_Color *_Nullable);
void AG_TlistSetFont(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     AG_Font *_Nullable);

void AG_TlistDel(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);

void AG_TlistSelect(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistDeselect(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistSelectIdx(AG_Tlist *_Nonnull, Uint);
void AG_TlistDeselectIdx(AG_Tlist *_Nonnull, Uint);
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

void AG_TlistSort(AG_Tlist *_Nonnull);
int  AG_TlistVisibleChildren(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistRefresh(AG_Tlist *_Nonnull);

#ifdef AG_TYPE_SAFETY
AG_TlistItem *_Nullable AG_TlistGetItemPtr(const AG_Event *_Nonnull, int, int);
#endif

#ifdef AG_LEGACY
#define AG_TLIST_TREE             0
#define AG_TLIST_EXPANDED         AG_TLIST_ITEM_EXPANDED
#define AG_TLIST_VISIBLE_CHILDREN AG_TLIST_ITEM_EXPANDED
#define AG_TlistPrescale(tl,text,n) AG_TlistSizeHint((tl),(text),(n))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TLIST_H_ */
