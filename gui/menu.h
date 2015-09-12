/*	Public domain	*/

#ifndef _AGAR_GUI_MENU_H_
#define _AGAR_GUI_MENU_H_

#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/toolbar.h>

#include <agar/gui/begin.h>

struct ag_menu;
struct ag_menu_view;
struct ag_button;

typedef AG_TAILQ_HEAD(ag_menu_itemq, ag_menu_item) AG_MenuItemQ;

typedef struct ag_menu_item {
	char *text;			/* Label text */
	int lblMenu[2];			/* Cached surfaces (for AG_Menu) */
	int lblView[2];			/* Cached surfaces (for AG_MenuView) */
	int icon;			/* Icon surface mapping */
	AG_Surface *iconSrc;		/* Icon surface source */
	int value;			/* Default bool value binding */

	int          state;		/* State flag */
	AG_Function *stateFn;		/* State function (overrides flag) */

	AG_KeySym key_equiv;		/* Key shortcut */
	AG_KeyMod key_mod;
	int x, y;			/* Position in parent view */
	AG_Event *clickFn;		/* Raised on click */
	AG_Event *poll;			/* Raised before the item is drawn */
	Uint flags;
#define AG_MENU_ITEM_ICONS	  0x01	/* At least one subitems has an icon
					   (optimization) */
#define AG_MENU_ITEM_NOSELECT	  0x02	/* Non-selectable regardless of state */
#define AG_MENU_ITEM_SEPARATOR	  0x04	/* Item is a cosmetic separator */

	enum ag_menu_binding {		/* Boolean binding */
		AG_MENU_NO_BINDING,
		AG_MENU_INT_BOOL,
		AG_MENU_INT8_BOOL,
		AG_MENU_INT_FLAGS,
		AG_MENU_INT8_FLAGS,
		AG_MENU_INT16_FLAGS,
		AG_MENU_INT32_FLAGS
	} bind_type;
	void *bind_p;			/* Pointer to bound variable */
	Uint32 bind_flags;		/* Bitmask to control (for FLAGS) */
	int bind_invert;		/* Invert the value */
	AG_Mutex *bind_lock;		/* Lock when accessing binding */

	struct ag_menu_view *view;	/* Back pointer to view (subitems) */
	struct ag_menu *pmenu;		/* Parent menu */
	struct ag_menu_item *sel_subitem; /* Selected subitem */
	struct ag_button *tbButton;	/* Associated toolbar button */
	struct ag_menu_item *parent;	/* Parent MenuItem or NULL */

	AG_TAILQ_ENTRY(ag_menu_item) items;	/* In parent */
	AG_TAILQ_HEAD_(ag_menu_item) subItems;	/* Child items */
	Uint                        nSubItems;
} AG_MenuItem;

enum ag_menu_style {
	AG_MENU_DROPDOWN,	/* Drop-down menu */
	AG_MENU_POPUP,		/* Contextual popup */
	AG_MENU_GLOBAL		/* Global application menu */
};

typedef struct ag_menu {
	struct ag_widget wid;
	Uint flags;
#define AG_MENU_HFILL	 	0x01
#define AG_MENU_VFILL	 	0x02
#define AG_MENU_EXPAND	 	(AG_MENU_HFILL|AG_MENU_VFILL)
	enum ag_menu_style style;	/* Menu style */
	AG_MenuItem *root;		/* Root menu item */
	int selecting;			/* Selection in progress */
	AG_MenuItem *itemSel;		/* Selected top-level item */
	int spHoriz;			/* Horiz spacing between items */
	int spVert;			/* Vertical spacing between items */
	int lPad, rPad, tPad, bPad;	/* Global padding in pixels */
	int lPadLbl, rPadLbl;		/* Item label padding in pixels */
	int tPadLbl, bPadLbl;
	int itemh;			/* Item height (optimization) */
	int curState;			/* For MenuState() */
	AG_Toolbar *curToolbar;		/* For MenuToolbar() */
	AG_Rect r;			/* View area */
} AG_Menu;

typedef struct ag_popup_menu {
	AG_Widget *widget;		/* Parent widget */
	AG_Menu *menu;			/* Menu (allocated) */
	AG_MenuItem *root;		/* Alias for menu->root */
	AG_Window *win;			/* Expanded window */
#ifdef AG_LEGACY
	AG_MenuItem *item;
#endif
} AG_PopupMenu;

typedef struct ag_menu_view {
	struct ag_widget wid;
	AG_Menu *pmenu;
	AG_MenuItem *pitem;
	int spIconLbl;			/* Icon and label spacing */
	int spLblArrow;			/* Label and submenu arrow spacing */
	int lPad, rPad, tPad, bPad;	/* Padding in pixels */
	AG_Timer submenuTo;		/* For sub-menu popup delay */
	int arrowRight;			/* Right arrow surface handle */
} AG_MenuView;

__BEGIN_DECLS
extern AG_WidgetClass agMenuClass;
extern AG_WidgetClass agMenuViewClass;
extern AG_Menu *agAppMenu;
extern AG_Window *agAppMenuWin;
extern AG_Mutex agAppMenuLock;

void       AG_InitAppMenu(void);
void       AG_DestroyAppMenu(void);
AG_Menu	  *AG_MenuNew(void *, Uint);
AG_Menu	  *AG_MenuNewGlobal(Uint);
void 	   AG_MenuScale(void *, int, int);
void	   AG_MenuDraw(void *);

AG_PopupMenu	*AG_PopupNew(void *);
void		 AG_PopupShow(AG_PopupMenu *);
void		 AG_PopupShowAt(AG_PopupMenu *, int, int);
void		 AG_PopupHide(AG_PopupMenu *);
void		 AG_PopupDestroy(AG_PopupMenu *);

void	     AG_MenuDel(AG_MenuItem *);
void	     AG_MenuItemFree(AG_MenuItem *);
AG_Window   *AG_MenuExpand(void *, AG_MenuItem *, int, int);
void   	     AG_MenuCollapse(AG_MenuItem *);
void   	     AG_MenuCollapseAll(AG_Menu *);

void	 AG_MenuSetPadding(AG_Menu *, int, int, int, int);
void	 AG_MenuSetLabelPadding(AG_Menu *, int, int, int, int);
#define	 AG_MenuSetPaddingLeft(m,v)   AG_MenuSetPadding((m),(v),-1,-1,-1)
#define	 AG_MenuSetPaddingRight(m,v)  AG_MenuSetPadding((m),-1,(v),-1,-1)
#define	 AG_MenuSetPaddingTop(m,v)    AG_MenuSetPadding((m),-1,-1,(v),-1)
#define	 AG_MenuSetPaddingBottom(m,v) AG_MenuSetPadding((m),-1,-1,-1,(v))
#define	 AG_MenuSetLabelPaddingLeft(m,v) \
	 AG_MenuSetLabelPadding((m),(v),-1,-1,-1)
#define	 AG_MenuSetLabelPaddingRight(m,v) \
	 AG_MenuSetLabelPadding((m),-1,(v),-1,-1)
#define	 AG_MenuSetLabelPaddingTop(m,v) \
	 AG_MenuSetLabelPadding((m),-1,-1,(v),-1)
#define	 AG_MenuSetLabelPaddingBottom(m,v) \
	 AG_MenuSetLabelPadding((m),-1,-1,-1,(v))

void AG_MenuSetIcon(AG_MenuItem *, const AG_Surface *);
void AG_MenuSetLabel(AG_MenuItem *, const char *, ...)
                     FORMAT_ATTRIBUTE(printf,2,3)
		     NONNULL_ATTRIBUTE(2);
void AG_MenuSetLabelS(AG_MenuItem *, const char *);
void AG_MenuSetPollFn(AG_MenuItem *, AG_EventFn, const char *, ...);
void AG_MenuUpdateItem(AG_MenuItem *);

void	     AG_MenuState(AG_MenuItem *, int);
#define      AG_MenuDisable(m) AG_MenuState((m),0)
#define      AG_MenuEnable(m) AG_MenuState((m),1)

void	     AG_MenuToolbar(AG_MenuItem *, AG_Toolbar *);

AG_MenuItem *AG_MenuNode(AG_MenuItem *, const char *, const AG_Surface *);
AG_MenuItem *AG_MenuSeparator(AG_MenuItem *);
AG_MenuItem *AG_MenuSection(AG_MenuItem *, const char *, ...)
                            FORMAT_ATTRIBUTE(printf,2,3)
			    NONNULL_ATTRIBUTE(2);
AG_MenuItem *AG_MenuSectionS(AG_MenuItem *, const char *);
AG_MenuItem *AG_MenuAction(AG_MenuItem *, const char *, const AG_Surface *,
			   AG_EventFn, const char *, ...);
AG_MenuItem *AG_MenuActionKb(AG_MenuItem *, const char *, const AG_Surface *,
                             AG_KeySym, AG_KeyMod, AG_EventFn, const char *, ...);
AG_MenuItem *AG_MenuTool(AG_MenuItem *, AG_Toolbar *, const char *,
                         const AG_Surface *, AG_KeySym, AG_KeyMod, AG_EventFn,
			 const char *, ...);
AG_MenuItem *AG_MenuDynamicItem(AG_MenuItem *, const char *, const AG_Surface *,
                                AG_EventFn, const char *, ...);
AG_MenuItem *AG_MenuDynamicItemKb(AG_MenuItem *, const char *, const AG_Surface *,
                                  AG_KeySym, AG_KeyMod, AG_EventFn, const char *,
				  ...);
AG_MenuItem *AG_MenuIntBoolMp(AG_MenuItem *, const char *, const AG_Surface *,
                              int *, int, AG_Mutex *);
AG_MenuItem *AG_MenuInt8BoolMp(AG_MenuItem *, const char *, const AG_Surface *,
                               Uint8 *, int, AG_Mutex *);
AG_MenuItem *AG_MenuIntFlagsMp(AG_MenuItem *, const char *, const AG_Surface *,
                               int *, int, int, AG_Mutex *);
AG_MenuItem *AG_MenuInt8FlagsMp(AG_MenuItem *, const char *, const AG_Surface *,
                                Uint8 *, Uint8, int, AG_Mutex *);
AG_MenuItem *AG_MenuInt16FlagsMp(AG_MenuItem *, const char *, const AG_Surface *,
                                 Uint16 *, Uint16, int, AG_Mutex *);
AG_MenuItem *AG_MenuInt32FlagsMp(AG_MenuItem *, const char *, const AG_Surface *,
                                 Uint32 *, Uint32, int, AG_Mutex *);

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

void    AG_MenuSetIntBoolMp(AG_MenuItem *, int *, int, AG_Mutex *);
#define AG_MenuSetIntBool(mi,p,fl,inv,mtx) \
        AG_MenuSetIntBoolMp((mi),(p),(fl),(inv),(mtx))

void AG_MenuSetIntFlagsMp(AG_MenuItem *, int *, int, int, AG_Mutex *);
#ifdef AG_LEGACY
# define AG_MenuAddItem(m,lbl) AG_MenuNode((m)->root,(lbl),NULL)
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_MENU_H_ */
