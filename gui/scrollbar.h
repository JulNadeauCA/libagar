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
#define AG_SCROLLBAR_SMOOTH	0x04	/* Animate scrolling to the target
                                           (default: jump to it) */
#define AG_SCROLLBAR_TEXT	0x08	/* Display values and offsets in text */
#define AG_SCROLLBAR_EXCL	0x40	/* Has exclusive access to bindings */
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL | AG_SCROLLBAR_VFILL)

	enum ag_scrollbar_type type;	/* Horizontal or vertical */

	enum ag_scrollbar_button curBtn;	/* Active button */
	enum ag_scrollbar_button mouseOverBtn;	/* Mouseover button */

	int length;			/* Length of scrolling control area */
	int wBarLast;			/* Auto-calculated control length */
	int hArrow;			/* Arrow height */
	int value;			/* Default `value' binding */
	AG_Event *_Nullable buttonIncFn; /* Alt. handler for increment btns */
	AG_Event *_Nullable buttonDecFn; /* Alt. handler for decrement btns */
	AG_Timer moveTo;		/* Timer for scrolling control */
	int xOffs, xSeek;		/* Cursor offset for scrolling */
} AG_Scrollbar;

#define AGSCROLLBAR(obj)            ((AG_Scrollbar *)(obj))
#define AGCSCROLLBAR(obj)           ((const AG_Scrollbar *)(obj))
#define AG_SCROLLBAR_SELF()          AGSCROLLBAR( AG_OBJECT(0,"AG_Widget:AG_Scrollbar:*") )
#define AG_SCROLLBAR_PTR(n)          AGSCROLLBAR( AG_OBJECT((n),"AG_Widget:AG_Scrollbar:*") )
#define AG_SCROLLBAR_NAMED(n)        AGSCROLLBAR( AG_OBJECT_NAMED((n),"AG_Widget:AG_Scrollbar:*") )
#define AG_CONST_SCROLLBAR_SELF()   AGCSCROLLBAR( AG_CONST_OBJECT(0,"AG_Widget:AG_Scrollbar:*") )
#define AG_CONST_SCROLLBAR_PTR(n)   AGCSCROLLBAR( AG_CONST_OBJECT((n),"AG_Widget:AG_Scrollbar:*") )
#define AG_CONST_SCROLLBAR_NAMED(n) AGCSCROLLBAR( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Scrollbar:*") )

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;

AG_Scrollbar *_Nonnull AG_ScrollbarNewHoriz(void *_Nullable, Uint);
AG_Scrollbar *_Nonnull AG_ScrollbarNewVert(void *_Nullable, Uint);
AG_Scrollbar *_Nonnull AG_ScrollbarNew(void *_Nullable, enum ag_scrollbar_type,
                                       Uint);

void AG_ScrollbarSetIncFn(AG_Scrollbar *_Nonnull,
                         _Nullable AG_EventFn, const char *_Nullable, ...);
void AG_ScrollbarSetDecFn(AG_Scrollbar *_Nonnull,
                         _Nullable AG_EventFn, const char *_Nullable, ...);

int AG_ScrollbarIsUseful(AG_Scrollbar *_Nonnull);
int AG_ScrollbarWidth(AG_Scrollbar *_Nonnull) _Pure_Attribute_If_Unthreaded;
__END_DECLS

#ifdef AG_LEGACY
# define AG_SCROLLBAR_NOAUTOHIDE 0
void AG_ScrollbarSetControlLength(AG_Scrollbar *_Nonnull, int) DEPRECATED_ATTRIBUTE;
int  AG_ScrollbarVisible(AG_Scrollbar *_Nonnull) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SCROLLBAR_H_ */
