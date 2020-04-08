/*	Public domain	*/

#ifndef _AGAR_GUI_MENU_H_
#define _AGAR_GUI_MENU_H_

#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/toolbar.h>

#include <agar/gui/begin.h>

#define AG_MENU_ITEM_TAG "AgMnItm"
#define AG_MENU_ITEM_TAG_LEN 8

struct ag_menu;
struct ag_menu_view;
struct ag_button;

typedef struct ag_menu_item {
#ifdef AG_TYPE_SAFETY
	char tag[AG_MENU_ITEM_TAG_LEN];	/* Tagged Non-object */
#endif
	char *_Nonnull text;		/* Label text */
	int lblMenu[2];			/* Cached surfaces (for AG_Menu) */
	int lblView[2];			/* Cached surfaces (for AG_MenuView) */
	AG_Surface *_Nullable iconSrc;	/* Icon surface source */
	int                   icon;	/* Icon surface mapping */
	int value;			/* Default bool value binding */
	AG_Function *_Nullable stateFn;	/* State function (overrides flag) */
	int                    state;	/* State flag */
	AG_KeySym key_equiv;		/* Key shortcut */
	AG_KeyMod key_mod;
	int x, y;			/* Position in parent view */
	Uint flags;
#define AG_MENU_ITEM_ICONS     0x01 /* At least one item has an icon */
#define AG_MENU_ITEM_NOSELECT  0x02 /* Non-selectable regardless of state */
#define AG_MENU_ITEM_SEPARATOR 0x04 /* Item is a cosmetic separator */
#define AG_MENU_ITEM_INVERTED  0x08 /* Invert the binding state */

	AG_Event *_Nullable clickFn;	/* Raised on click */
	AG_Event *_Nullable poll;	/* Raised before the item is drawn */

	enum ag_menu_binding {		/* Boolean binding */
		AG_MENU_NO_BINDING,
		AG_MENU_INT_BOOL,
		AG_MENU_INT8_BOOL,
		AG_MENU_INT_FLAGS,
		AG_MENU_INT8_FLAGS,
		AG_MENU_INT16_FLAGS,
		AG_MENU_INT32_FLAGS
	} bind_type;
	Uint32          bind_flags;     /* Bitmask (for FLAGS) */
	void *_Nullable bind_p;	        /* Pointer to data */
#ifdef AG_THREADS
	_Nullable_Mutex AG_Mutex *_Nullable bind_lock; /* Lock on data */
#endif
	struct ag_menu_view *_Nullable view;        /* Parent view (subitems) */
	struct ag_menu *_Nullable      pmenu;       /* Parent menu */
	struct ag_menu_item *_Nullable sel_subitem; /* Selected subitem */
	struct ag_button *_Nullable    tbButton;    /* Related toolbar button */
	struct ag_menu_item *_Nullable parent;      /* Parent MenuItem if any */

	AG_TAILQ_HEAD_(ag_menu_item) subItems;      /* Child items */
	Uint                        nSubItems;
	Uint32 _pad;
	AG_TAILQ_ENTRY(ag_menu_item) items;         /* In parent */
} AG_MenuItem;

#define AGMENUITEM(p)             ((AG_MenuItem *)(p))
#define AGCMENUITEM(p)            ((const AG_MenuItem *)(p))
#define AG_MENU_ITEM_SELF()       AGMENUITEM( AG_MENU_ITEM_PTR(0) )
#define AG_CONST_MENU_ITEM_SELF() AGCMENUITEM( AG_CONST_MENU_ITEM(0) )

#ifdef AG_TYPE_SAFETY
# define AG_MENU_ITEM_VALID(p)     (strncmp(AGMENUITEM(p)->tag, AG_MENU_ITEM_TAG, AG_MENU_ITEM_TAG_LEN) == 0)
# define AG_MENU_ITEM_IS_VALID(p)  if (!AG_MENU_ITEM_VALID(p)) { AG_FatalError("Illegal AG_MenuItem access"); }
# define AG_MENU_ITEM_PTR(v)       AG_MenuGetItemPtr(event,(v),0)
# define AG_CONST_MENU_ITEM_PTR(v) AG_MenuGetItemPtr(event,(v),1)
#else
# define AG_MENU_ITEM_VALID(p)     1
# define AG_MENU_ITEM_IS_VALID(p)
# define AG_MENU_ITEM_PTR(v)       event->argv[v].data.p
# define AG_CONST_MENU_ITEM_PTR(v) event->argv[v].data.p
#endif

enum ag_menu_style {
	AG_MENU_DROPDOWN,       /* Drop-down menu */
	AG_MENU_POPUP,          /* Contextual popup */
	AG_MENU_GLOBAL          /* Global application menu */
};

typedef struct ag_menu {
	struct ag_widget wid;             /* AG_Widget -> AG_Menu */
	Uint flags;
#define AG_MENU_HFILL  0x01
#define AG_MENU_VFILL  0x02
#define AG_MENU_EXPAND (AG_MENU_HFILL | AG_MENU_VFILL)
	enum ag_menu_style style;         /* Menu style */
	AG_MenuItem *_Nonnull root;       /* Root menu item */
	int selecting;                    /* Selection in progress */
	int lPadLbl, rPadLbl;             /* Item label padding in pixels */
	int tPadLbl, bPadLbl;
	int itemh;                        /* Item height (optimization) */
	int curState;                     /* For MenuState() */
	Uint32 _pad;
	AG_MenuItem *_Nullable itemSel;   /* Selected top-level item */
	AG_Toolbar *_Nullable curToolbar; /* For MenuToolbar() */
	AG_Rect r;                        /* View area */
} AG_Menu;

#define AGMENU(obj)            ((AG_Menu *)(obj))
#define AGCMENU(obj)           ((const AG_Menu *)(obj))
#define AG_MENU_SELF()          AGMENU( AG_OBJECT(0,"AG_Widget:AG_Menu:*") )
#define AG_MENU_PTR(n)          AGMENU( AG_OBJECT((n),"AG_Widget:AG_Menu:*") )
#define AG_MENU_NAMED(n)        AGMENU( AG_OBJECT_NAMED((n),"AG_Widget:AG_Menu:*") )
#define AG_CONST_MENU_SELF()   AGCMENU( AG_CONST_OBJECT(0,"AG_Widget:AG_Menu:*") )
#define AG_CONST_MENU_PTR(n)   AGCMENU( AG_CONST_OBJECT((n),"AG_Widget:AG_Menu:*") )
#define AG_CONST_MENU_NAMED(n) AGCMENU( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Menu:*") )

typedef struct ag_popup_menu {
	AG_Widget *_Nonnull widget;     /* Parent widget */
	AG_Menu *_Nonnull menu;         /* Menu (allocated) */
	AG_MenuItem *_Nonnull root;     /* Alias for menu->root */
	AG_Window *_Nullable win;       /* Expanded window */
} AG_PopupMenu;

typedef struct ag_menu_view {
	struct ag_widget wid;           /* AG_Widget -> AG_MenuView */

	AG_Menu     *_Nullable pmenu;   /* Associated menu */
	AG_MenuItem *_Nullable pitem;   /* Associated menu item */

	int spLblArrow;                 /* Label and submenu arrow spacing */
	int arrowRight;                 /* Right arrow surface handle */
} AG_MenuView;

#define AGMENUVIEW(obj)            ((AG_MenuView *)(obj))
#define AGCMENUVIEW(obj)           ((const AG_MenuView *)(obj))
#define AG_MENUVIEW_SELF()         AGMENUVIEW( AG_OBJECT(0,"AG_Widget:AG_MenuView:*") )
#define AG_MENUVIEW_PTR(n)         AGMENUVIEW( AG_OBJECT((n),"AG_Widget:AG_MenuView:*") )
#define AG_MENUVIEW_NAMED(n)       AGMENUVIEW( AG_OBJECT_NAMED((n),"AG_Widget:AG_MenuView:*") )
#define AG_CONST_MENUVIEW_SELF()   AGCMENUVIEW( AG_CONST_OBJECT(0,"AG_Widget:AG_MenuView:*") )
#define AG_CONST_MENUVIEW_PTR(n)   AGCMENUVIEW( AG_CONST_OBJECT((n),"AG_Widget:AG_MenuView:*") )
#define AG_CONST_MENUVIEW_NAMED(n) AGCMENUVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_MenuView:*") )

__BEGIN_DECLS
extern AG_WidgetClass agMenuClass;
extern AG_WidgetClass agMenuViewClass;

extern AG_Menu   *_Nullable agAppMenu;
extern AG_Window *_Nullable agAppMenuWin;

AG_Menu	*_Nonnull  AG_MenuNew(void *_Nullable, Uint);
AG_Menu	*_Nullable AG_MenuNewGlobal(Uint);
void	           AG_MenuDraw(void *_Nonnull);

AG_PopupMenu *_Nonnull AG_PopupNew(void *_Nonnull);
void		       AG_PopupShow(AG_PopupMenu *_Nonnull);
void		       AG_PopupShowAt(AG_PopupMenu *_Nonnull, int,int);
void		       AG_PopupHide(AG_PopupMenu *_Nonnull);
void		       AG_PopupDestroy(AG_PopupMenu *_Nonnull);

void	             AG_MenuDel(AG_MenuItem *_Nonnull);
void	             AG_MenuItemFree(AG_MenuItem *_Nonnull);
AG_Window *_Nullable AG_MenuExpand(void *_Nullable, AG_MenuItem *_Nonnull,
                                   int,int);
void   	             AG_MenuCollapse(AG_MenuItem *_Nullable);
void   	             AG_MenuCollapseAll(AG_Menu *_Nonnull);

void	 AG_MenuSetLabelPadding(AG_Menu *_Nonnull, int,int,int,int);
#define	 AG_MenuSetLabelPaddingLeft(m,v) \
	 AG_MenuSetLabelPadding((m),(v),-1,-1,-1)
#define	 AG_MenuSetLabelPaddingRight(m,v) \
	 AG_MenuSetLabelPadding((m),-1,(v),-1,-1)
#define	 AG_MenuSetLabelPaddingTop(m,v) \
	 AG_MenuSetLabelPadding((m),-1,-1,(v),-1)
#define	 AG_MenuSetLabelPaddingBottom(m,v) \
	 AG_MenuSetLabelPadding((m),-1,-1,-1,(v))

void AG_MenuSetIcon(AG_MenuItem *_Nonnull, const AG_Surface *_Nullable);
void AG_MenuSetLabel(AG_MenuItem *_Nonnull, const char *_Nonnull, ...)
                     FORMAT_ATTRIBUTE(printf,2,3);
void AG_MenuSetLabelS(AG_MenuItem *_Nonnull, const char *_Nonnull);
void AG_MenuSetPollFn(AG_MenuItem *_Nonnull, _Nonnull AG_EventFn,
                      const char *_Nullable, ...);
void AG_MenuUpdateItem(AG_MenuItem *_Nonnull);
void AG_MenuInvalidateLabels(AG_MenuItem *_Nonnull);
void AG_MenuFreeSubitems(AG_MenuItem *_Nonnull);

void	AG_MenuState(AG_MenuItem *_Nonnull, int);
#define AG_MenuDisable(m) AG_MenuState((m),0)
#define AG_MenuEnable(m) AG_MenuState((m),1)
void    AG_MenuToolbar(AG_MenuItem *_Nonnull, AG_Toolbar *_Nullable);

AG_MenuItem *_Nonnull AG_MenuNode(AG_MenuItem *_Nonnull, const char *_Nullable,
                                  const AG_Surface *_Nullable);
AG_MenuItem *_Nonnull AG_MenuSeparator(AG_MenuItem *_Nonnull);
AG_MenuItem *_Nonnull AG_MenuSectionS(AG_MenuItem *_Nonnull, const char *_Nullable);
AG_MenuItem *_Nonnull AG_MenuSection(AG_MenuItem *_Nonnull,
                                     const char *_Nonnull, ...)
                                    FORMAT_ATTRIBUTE(printf,2,3);

AG_MenuItem *_Nonnull AG_MenuAction(AG_MenuItem *_Nonnull, const char *_Nullable,
                                    const AG_Surface *_Nullable,
                                    _Nonnull AG_EventFn,
				    const char *_Nullable, ...);

AG_MenuItem *_Nonnull AG_MenuActionKb(AG_MenuItem *_Nonnull,
                                      const char *_Nullable,
				      const AG_Surface *_Nullable,
                                      AG_KeySym, AG_KeyMod,
				      _Nonnull AG_EventFn,
				      const char *_Nullable, ...);

AG_MenuItem *_Nonnull AG_MenuTool(AG_MenuItem *_Nonnull, AG_Toolbar *_Nonnull,
                                  const char *_Nullable,
                                  const AG_Surface *_Nullable,
                                  AG_KeySym, AG_KeyMod,
                                  _Nonnull AG_EventFn,
                                  const char *_Nullable, ...);

AG_MenuItem *_Nonnull AG_MenuDynamicItem(AG_MenuItem *_Nonnull,
                                         const char *_Nullable,
                                         const AG_Surface *_Nullable,
                                         _Nonnull AG_EventFn,
					 const char *_Nullable, ...);

AG_MenuItem *_Nonnull AG_MenuDynamicItemKb(AG_MenuItem *_Nonnull,
					   const char *_Nullable,
					   const AG_Surface *_Nullable,
					   AG_KeySym, AG_KeyMod,
					   _Nonnull AG_EventFn,
					   const char *_Nullable, ...);

AG_MenuItem *_Nonnull AG_MenuIntBoolMp(AG_MenuItem *_Nonnull,
                                       const char *_Nullable,
                                       const AG_Surface *_Nullable,
                                       int *_Nonnull, int,
				       _Nonnull_Mutex AG_Mutex *_Nullable);
AG_MenuItem *_Nonnull AG_MenuInt8BoolMp(AG_MenuItem *_Nonnull,
                                        const char *_Nullable,
                                        const AG_Surface *_Nullable,
                                        Uint8 *_Nonnull, int,
					_Nonnull_Mutex AG_Mutex *_Nullable);
AG_MenuItem *_Nonnull AG_MenuIntFlagsMp(AG_MenuItem *_Nonnull,
					const char *_Nullable,
					const AG_Surface *_Nullable,
					int *_Nonnull, int, int,
					_Nonnull_Mutex AG_Mutex *_Nullable);
AG_MenuItem *_Nonnull AG_MenuInt8FlagsMp(AG_MenuItem *_Nonnull,
					 const char *_Nullable,
					 const AG_Surface *_Nullable,
					 Uint8 *_Nonnull, Uint8, int,
					 _Nonnull_Mutex AG_Mutex *_Nullable);
AG_MenuItem *_Nonnull AG_MenuInt16FlagsMp(AG_MenuItem *_Nonnull,
					 const char *_Nullable,
					 const AG_Surface *_Nullable,
					 Uint16 *_Nonnull, Uint16, int,
					 _Nonnull_Mutex AG_Mutex *_Nullable);
AG_MenuItem *_Nonnull AG_MenuInt32FlagsMp(AG_MenuItem *_Nonnull,
					  const char *_Nullable,
					  const AG_Surface *_Nullable,
					  Uint32 *_Nonnull, Uint32, int,
					  _Nonnull_Mutex AG_Mutex *_Nullable);

#define	AG_MenuIntBool(mi,t,i,p,inv) \
	AG_MenuIntBoolMp((mi),(t),(i),(p),(inv),NULL)
#define AG_MenuInt8Bool(mi,t,i,p,inv) \
	AG_MenuInt8BoolMp((mi),(t),(i),(p),(inv),NULL)

#define	AG_MenuBool(mi,t,i,p,inv) \
	AG_MenuIntBoolMp((mi),(t),(i),(p),(inv),NULL)
#define	AG_MenuBoolMp(mi,t,i,p,inv,mu) \
	AG_MenuIntBoolMp((mi),(t),(i),(p),(inv),(mu))

#define AG_MenuIntFlags(mi,t,i,fp,fl,inv) \
	AG_MenuIntFlagsMp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define AG_MenuInt8Flags(mi,t,i,fp,fl,inv) \
	AG_MenuInt8FlagsMp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define AG_MenuInt16Flags(mi,t,i,fp,fl,inv) \
	AG_MenuInt16FlagsMp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define AG_MenuInt32Flags(mi,t,i,fp,fl,inv) \
	AG_MenuInt32FlagsMp((mi),(t),(i),(fp),(fl),(inv),NULL)

#define AG_MenuUintFlagsMp(mi,t,i,fp,fl,inv,mtx) \
	AG_MenuIntFlagsMp((mi),(t),(i),(int *)(fp),(int)(fl),(inv),(mtx))
#define AG_MenuUintFlags(mi,t,i,fp,fl,inv) \
	AG_MenuIntFlagsMp((mi),(t),(i),(int *)(fp),(int)(fl),(inv),NULL)

#define AG_MenuFlagsMp(mi,t,i,fp,fl,inv,mtx) \
	AG_MenuIntFlagsMp((mi),(t),(i),(int *)(fp),(int)(fl),(inv),(mtx))
#define AG_MenuFlags(mi,t,i,fp,fl,inv) \
	AG_MenuIntFlagsMp((mi),(t),(i),(int *)(fp),(int)(fl),(inv),NULL)

void    AG_MenuSetIntBoolMp(AG_MenuItem *_Nonnull, int *_Nonnull, int,
                            _Nonnull_Mutex AG_Mutex *_Nonnull);
#define AG_MenuSetIntBool(mi,p,fl,inv,mtx) \
        AG_MenuSetIntBoolMp((mi),(p),(fl),(inv),(mtx))

void    AG_MenuSetIntFlagsMp(AG_MenuItem *_Nonnull, int *_Nonnull, int, int,
                             _Nonnull_Mutex AG_Mutex *_Nonnull);

#ifdef AG_TYPE_SAFETY
AG_MenuItem *_Nullable AG_MenuGetItemPtr(const AG_Event *_Nonnull, int, int);
#endif

#ifdef AG_LEGACY
# define AG_MenuAddItem(m,lbl)        AG_MenuNode((m)->root,(lbl),NULL)
# define AG_MenuSetPaddingLeft(m,v)   AG_MenuSetPadding((m),(v),-1,-1,-1)
# define AG_MenuSetPaddingRight(m,v)  AG_MenuSetPadding((m),-1,(v),-1,-1)
# define AG_MenuSetPaddingTop(m,v)    AG_MenuSetPadding((m),-1,-1,(v),-1)
# define AG_MenuSetPaddingBottom(m,v) AG_MenuSetPadding((m),-1,-1,-1,(v))
void AG_MenuSetPadding(AG_Menu *_Nonnull, int,int,int,int)
                      DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_MENU_H_ */
