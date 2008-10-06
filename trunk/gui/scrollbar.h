/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#include <agar/gui/widget.h>

#include "begin_code.h"

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
	struct ag_widget wid;
	Uint flags;
#define AG_SCROLLBAR_HFILL	0x01
#define AG_SCROLLBAR_VFILL	0x02
#define AG_SCROLLBAR_TEXT	0x08	/* Print values */
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL|AG_SCROLLBAR_VFILL)

	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	int visible;			/* Subtracts from range */
	enum ag_scrollbar_type type;	/* Style of scrollbar */
	enum ag_scrollbar_button curBtn; /* Active button */
	int wButton;			/* Button size */
	int wBar;			/* Scroll bar size */
	int hArrow;			/* Arrow height */
	AG_Event *buttonIncFn;		/* Alt. handler for increment btns */
	AG_Event *buttonDecFn;		/* Alt. handler for decrement btns */
	AG_Timeout scrollTo;		/* Timer for scrolling */
	AG_Timeout incTo, decTo;	/* Timer for keyboard motion */
	int xOffs;			/* Cursor offset for scrolling */
	int extent;			/* Available area for scrolling */
	double rInc;			/* Base increment for real bindings */
	int    iInc;			/* Base increment for int bindings */
} AG_Scrollbar;

#define AGSCROLLBAR(p) ((AG_Scrollbar *)p)

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;

AG_Scrollbar *AG_ScrollbarNew(void *, enum ag_scrollbar_type, Uint);
AG_Scrollbar *AG_ScrollbarNewInt(void *, enum ag_scrollbar_type, Uint,
                                 int *, int *, int *, int *);
AG_Scrollbar *AG_ScrollbarNewUint(void *, enum ag_scrollbar_type, Uint,
                                  Uint *, Uint *, Uint *, Uint *);
AG_Scrollbar *AG_ScrollbarNewUint8(void *, enum ag_scrollbar_type, Uint,
                                   Uint8 *, Uint8 *, Uint8 *, Uint8 *);
AG_Scrollbar *AG_ScrollbarNewSint8(void *, enum ag_scrollbar_type, Uint,
                                   Sint8 *, Sint8 *, Sint8 *, Sint8 *);
AG_Scrollbar *AG_ScrollbarNewUint16(void *, enum ag_scrollbar_type, Uint,
                                    Uint16 *, Uint16 *, Uint16 *, Uint16 *);
AG_Scrollbar *AG_ScrollbarNewSint16(void *, enum ag_scrollbar_type, Uint,
                                    Sint16 *, Sint16 *, Sint16 *, Sint16 *);
AG_Scrollbar *AG_ScrollbarNewUint32(void *, enum ag_scrollbar_type, Uint,
                                    Uint32 *, Uint32 *, Uint32 *, Uint32 *);
AG_Scrollbar *AG_ScrollbarNewSint32(void *, enum ag_scrollbar_type, Uint,
                                    Sint32 *, Sint32 *, Sint32 *, Sint32 *);
#ifdef HAVE_64BIT
AG_Scrollbar *AG_ScrollbarNewUint64(void *, enum ag_scrollbar_type, Uint,
                                    Uint64 *, Uint64 *, Uint64 *, Uint64 *);
AG_Scrollbar *AG_ScrollbarNewSint64(void *, enum ag_scrollbar_type, Uint,
                                    Sint64 *, Sint64 *, Sint64 *, Sint64 *);
#endif
AG_Scrollbar *AG_ScrollbarNewFloat(void *, enum ag_scrollbar_type, Uint,
                                   float *, float *, float *, float *);
AG_Scrollbar *AG_ScrollbarNewDouble(void *, enum ag_scrollbar_type, Uint,
                                    double *, double *, double *, double *);
#ifdef HAVE_LONG_DOUBLE
AG_Scrollbar *AG_ScrollbarNewLongDouble(void *, enum ag_scrollbar_type, Uint,
                                        long double *, long double *,
					long double *, long double *);
#endif

int  AG_ScrollbarVisible(AG_Scrollbar *);
void AG_ScrollbarSetIncFn(AG_Scrollbar *, AG_EventFn, const char *, ...);
void AG_ScrollbarSetDecFn(AG_Scrollbar *, AG_EventFn, const char *, ...);
void AG_ScrollbarSetIntIncrement(AG_Scrollbar *, int);
void AG_ScrollbarSetRealIncrement(AG_Scrollbar *, double);

static __inline__ void
AG_ScrollbarSetBarSize(AG_Scrollbar *sb, int bsize)
{
	AG_ObjectLock(sb);
	sb->wBar = (bsize > 10 || bsize == -1) ? bsize : 10;
	sb->extent = (sb->type == AG_SCROLLBAR_VERT) ? AGWIDGET(sb)->h :
	                                               AGWIDGET(sb)->w;
	sb->extent -= sb->wButton*2;
	sb->extent -= sb->wBar;
	AG_ObjectUnlock(sb);
}

static __inline__ int
AG_ScrollbarGetBarSize(AG_Scrollbar *sb)
{
	int rv;

	AG_ObjectLock(sb);
	if (sb->wBar == -1) {
		rv = (sb->type == AG_SCROLLBAR_VERT) ? AGWIDGET(sb)->h :
		                                       AGWIDGET(sb)->w;
		rv -= sb->wButton*2;
		if (rv < 0) { rv = 0; }
	} else {
		rv = sb->wBar;
	}
	AG_ObjectUnlock(sb);
	return (rv);
}

static __inline__ void
AG_ScrollbarSetButtonSize(AG_Scrollbar *sb, int wButton)
{
	AG_ObjectLock(sb);
	sb->wButton = wButton;
	AG_ObjectUnlock(sb);
}
static __inline__ int
AG_ScrollbarGetButtonSize(AG_Scrollbar *sb)
{
	int rv;
	AG_ObjectLock(sb);
	rv = sb->wButton;
	AG_ObjectUnlock(sb);
	return (rv);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
