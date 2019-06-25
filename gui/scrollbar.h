/*	Public domain	*/

#ifndef _AGAR_GUI_SCROLLBAR_H_
#define _AGAR_GUI_SCROLLBAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/begin.h>

enum ag_scrollbar_type {
	AG_SCROLLBAR_HORIZ,
	AG_SCROLLBAR_VERT
};

enum ag_scrollbar_button {
	AG_SCROLLBAR_BUTTON_NONE,
	AG_SCROLLBAR_BUTTON_DEC,
	AG_SCROLLBAR_BUTTON_INC,
	AG_SCROLLBAR_BUTTON_SCROLL
};

typedef struct ag_scrollbar {
	struct ag_widget wid;		/* AG_Widget -> AG_Scrollbar */
	Uint flags;
#define AG_SCROLLBAR_HFILL	0x01
#define AG_SCROLLBAR_VFILL	0x02
#define AG_SCROLLBAR_TEXT	0x08	/* Print numbers (for debugging) */
#define AG_SCROLLBAR_AUTOHIDE	0x20	/* Show based on range (default) */
#define AG_SCROLLBAR_EXCL	0x40	/* Has exclusive access to bindings */
#define AG_SCROLLBAR_NOAUTOHIDE 0x80	/* Disable autohide */
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL|AG_SCROLLBAR_VFILL)

	enum ag_scrollbar_type type;	/* Horizontal or vertical */

	enum ag_scrollbar_button curBtn;	/* Active button */
	enum ag_scrollbar_button mouseOverBtn;	/* Mouseover button */

	int width;			/* Scrollbar width */
	int length;			/* Length of scrolling control area */
	int wBar;			/* Preferred control length */
	int wBarMin;			/* Minimum control length */
	int hArrow;			/* Arrow height */
	AG_Event *_Nullable buttonIncFn; /* Alt. handler for increment btns */
	AG_Event *_Nullable buttonDecFn; /* Alt. handler for decrement btns */
	AG_Timer moveTo;		/* Timer for scrolling control */
	AG_Timer autoHideTo;		/* Timer for autohide check */
	int xOffs, xSeek;		/* Cursor offset for scrolling */
	Uint lenPre;			/* Preferred length size hint */
	int minOffs, maxOffs, visOffs;	/* Constants to add to binding values */
	int value;			/* Default `value' binding */
} AG_Scrollbar;

#define AGSCROLLBAR(obj)            ((AG_Scrollbar *)(obj))
#define AG_SCROLLBAR_SELF()         AG_OBJECT(0,"AG_Widget:AG_Scrollbar:*")
#define AG_SCROLLBAR_PTR(n)         AG_OBJECT((n),"AG_Widget:AG_Scrollbar:*")
#define AG_SCROLLBAR_NAMED(n)       AG_OBJECT_NAMED((n),"AG_Widget:AG_Scrollbar:*")
#define AG_CONST_SCROLLBAR_SELF()   AG_CONST_OBJECT(0,"AG_Widget:AG_Scrollbar:*")
#define AG_CONST_SCROLLBAR_PTR(n)   AG_CONST_OBJECT((n),"AG_Widget:AG_Scrollbar:*")
#define AG_CONST_SCROLLBAR_NAMED(n) AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Scrollbar:*")

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;

AG_Scrollbar *_Nonnull AG_ScrollbarNew(void *_Nullable, enum ag_scrollbar_type,
                                       Uint);
AG_Scrollbar *_Nonnull AG_ScrollbarNewHoriz(void *_Nullable, Uint);
AG_Scrollbar *_Nonnull AG_ScrollbarNewVert(void *_Nullable, Uint);

void AG_ScrollbarSizeHint(AG_Scrollbar *_Nonnull, int);
void AG_ScrollbarSetControlLength(AG_Scrollbar *_Nonnull, int);
void AG_ScrollbarSetWidth(AG_Scrollbar *_Nonnull, int);

void AG_ScrollbarSetIncFn(AG_Scrollbar *_Nonnull,
                         _Nullable AG_EventFn, const char *_Nullable, ...);
void AG_ScrollbarSetDecFn(AG_Scrollbar *_Nonnull,
                         _Nullable AG_EventFn, const char *_Nullable, ...);

int AG_ScrollbarVisible(AG_Scrollbar *_Nonnull);
int AG_ScrollbarWidth(AG_Scrollbar *_Nonnull) _Pure_Attribute_If_Unthreaded;
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SCROLLBAR_H_ */
