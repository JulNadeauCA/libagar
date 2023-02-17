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
	char tag[AG_TLIST_ITEM_TAG_LEN]; /* Tagged non-object */
#endif
	int label[3];                    /* Rendered item surface */
	                                 /* 0=Disabled, 1=Enabled, 2=Selected */
	int v;                           /* App-specific integer / sort key */
	const char *_Nullable cat;       /* App-specific category id */
	AG_Surface *_Nullable iconsrc;   /* Icon source image */
	void *_Nullable p1;              /* App-specific pointer */
	AG_Color *_Nullable color;       /* Alternate color */
	AG_Font *_Nullable font;         /* Alternate font */
	int selected;                    /* Effective selection flag */
	Uint depth;                      /* Indent in tree display */
	Uint flags;
#define AG_TLIST_ITEM_EXPANDED 0x01      /* Child items visible (tree) */
#define AG_TLIST_HAS_CHILDREN  0x02      /* Child items exist (tree) */
#define AG_TLIST_NO_SELECT     0x08      /* Item is not selectable */
#define AG_TLIST_NO_POPUP      0x10      /* Disable popups for item */
#define AG_TLIST_ITEM_DISABLED 0x20      /* Disable item (draw as disabled) */

	float scale;                     /* Text scaling factor */
	char text[AG_TLIST_LABEL_MAX];   /* Label text */
	Uint u;                          /* App-specific unsigned integer */

	AG_TAILQ_ENTRY(ag_tlist_item) items;    /* Items in list */
	AG_TAILQ_ENTRY(ag_tlist_item) selitems; /* Saved selection state */
} AG_TlistItem;

typedef AG_TAILQ_HEAD(ag_tlist_itemq, ag_tlist_item) AG_TlistItemQ;

typedef int (*AG_TlistCompareFn)(const AG_TlistItem *_Nonnull,
	                         const AG_TlistItem *_Nonnull);

/* Tree/list widget */
typedef struct ag_tlist {
	struct ag_widget wid;           /* AG_Widget -> AG_Tlist */
	Uint flags;
#define AG_TLIST_MULTI         0x0001      /* Multiple selections (ctrl/shift) */
#define AG_TLIST_MULTITOGGLE   0x0002      /* Multiple toggle-style selections */
#define AG_TLIST_POLL          0x0004      /* Generate tlist-poll events */
#define AG_TLIST_NO_SELECTED   0x0008      /* Inhibit "tlist-selected" event */
#define AG_TLIST_NO_SCALE_ICON 0x0010      /* Don't scale oversize icons. */
#define AG_TLIST_HFILL         0x0020
#define AG_TLIST_VFILL         0x0040
#define AG_TLIST_FIXED_HEIGHT  0x0080      /* Don't set icon height on "font-changed" */
#define AG_TLIST_STATELESS     0x0100      /* Don't preserve selection state (polled mode) */
#define AG_TLIST_SCROLLTOSEL   0x0200      /* Scroll to initial selection */
#define AG_TLIST_REFRESH       0x0400      /* Repopulate now (polled mode) */
#define AG_TLIST_EXPAND_NODES  0x0800      /* Expand node items (items with children) by default */
#define AG_TLIST_NO_KEYREPEAT  0x1000      /* Disable keyrepeat behavior */
#define AG_TLIST_EXPAND        (AG_TLIST_HFILL | AG_TLIST_VFILL)

	int item_h;                     /* Item height */
	void *_Nullable selected;       /* Default `selected' binding */
	int wHint, hHint;               /* Size hint */
	AG_Rect r;                      /* Clipping rectangle */
	int  *expLevels;                /* Tree expansion state for draw() */
	Uint nExpLevels;
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
	AG_TlistCompareFn compare_fn;   /* Item-item comparison function */
	AG_Event *_Nullable popupEv;    /* Popup menu hook */
	AG_Event *_Nullable changedEv;  /* Selection change hook */
	AG_Event *_Nullable dblClickEv; /* Double click hook */
	int lineScrollAmount;
	int lastKeyDown;                /* For key repeat */
	AG_Timer moveTo;                /* Timer for keyboard motion */
	AG_Timer refreshTo;             /* Timer for polled mode updates */
	AG_Timer dblClickTo;            /* Timer for detecting double clicks */
	AG_Timer ctrlMoveTo;            /* Timer for controller-driven move */
	AG_Color cBgLine[AG_WIDGET_NSTATES];  /* Background line color */
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
int  AG_TlistVisibleChildren(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);

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

void AG_TlistMoveToHead(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);
void AG_TlistMoveToTail(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull);

AG_TlistItem *_Nonnull AG_TlistItemNew(const AG_Surface *_Nullable);

void AG_TlistSetIcon(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     const AG_Surface *_Nullable);
void AG_TlistSetColor(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                      const AG_Color *_Nullable);
void AG_TlistSetFont(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                     const char *_Nullable, float, Uint);

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

void AG_TlistScrollToSelection(AG_Tlist *_Nonnull);
void AG_TlistScrollToStart(AG_Tlist *_Nonnull);
void AG_TlistScrollToEnd(AG_Tlist *_Nonnull);

void AG_TlistSetDblClickFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                           const char *_Nullable, ...);
void AG_TlistSetPopupFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                        const char *_Nullable, ...);
void AG_TlistSetChangedFn(AG_Tlist *_Nonnull, _Nonnull AG_EventFn,
                          const char *_Nullable, ...);

AG_TlistCompareFn AG_TlistSetCompareFn(AG_Tlist *_Nonnull, AG_TlistCompareFn);

int  AG_TlistCompareInts(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull)
                         _Pure_Attribute;
int  AG_TlistCompareIntsDsc(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull)
                            _Pure_Attribute;
int  AG_TlistCompareUints(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull)
                          _Pure_Attribute;
int  AG_TlistCompareStrings(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull)
                           _Pure_Attribute;
int  AG_TlistComparePtrs(const AG_TlistItem *_Nonnull,
                         const AG_TlistItem *_Nonnull)
                        _Pure_Attribute;
int  AG_TlistComparePtrsAndCats(const AG_TlistItem *_Nonnull,
                                const AG_TlistItem *_Nonnull)
                                _Pure_Attribute;

void AG_TlistSort(AG_Tlist *_Nonnull);
void AG_TlistSortByInt(AG_Tlist *_Nonnull);
void AG_TlistRefresh(AG_Tlist *_Nonnull);

#ifdef AG_TYPE_SAFETY
AG_TlistItem *_Nullable AG_TlistGetItemPtr(const AG_Event *_Nonnull, int, int);
#endif

#ifdef AG_LEGACY
#define AG_TLIST_TREE 0
#define AG_TLIST_EXPANDED AG_TLIST_ITEM_EXPANDED
#define AG_TLIST_VISIBLE_CHILDREN AG_TLIST_ITEM_EXPANDED
#define AG_TLIST_NOSELSTATE AG_TLIST_STATELESS
#define AG_TLIST_NOSELEVENT AG_TLIST_NO_SELECTED
#define AG_TlistPrescale(tl,text,n) AG_TlistSizeHint((tl),(text),(n))
#define AG_TlistComparePtrsAndClasses(a,b) AG_TlistComparePtrsAndCats(a,b)
#endif

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TLIST_H_ */
