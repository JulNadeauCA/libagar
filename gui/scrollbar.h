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
	struct ag_widget wid;
	Uint flags;
#define AG_SCROLLBAR_HFILL	0x01
#define AG_SCROLLBAR_VFILL	0x02
#define AG_SCROLLBAR_TEXT	0x08	/* Print numbers (for debugging) */
#define AG_SCROLLBAR_AUTOSIZE	0x10	/* Automatically size control */
#define AG_SCROLLBAR_AUTOHIDE	0x20	/* Show/hide widget based on range */
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL|AG_SCROLLBAR_VFILL)

	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	int visible;			/* Subtracts from range */
	enum ag_scrollbar_type type;	/* Style of scrollbar */
	enum ag_scrollbar_button curBtn; /* Active button */
	int width;			/* Scrollbar width */
	int length;			/* Length of scrolling control area */
	int wBar;			/* Scrolling control length */
	int hArrow;			/* Arrow height */
	AG_Event *buttonIncFn;		/* Alt. handler for increment btns */
	AG_Event *buttonDecFn;		/* Alt. handler for decrement btns */
	AG_Timeout scrollTo;		/* Timer for scrolling */
	AG_Timeout incTo, decTo;	/* Timer for keyboard motion */
	AG_Timeout autohideTo;		/* Timer for autohide check */
	int xOffs;			/* Cursor offset for scrolling */
	double rInc;			/* Base increment for real bindings */
	int    iInc;			/* Base increment for int bindings */
	Uint lenPre;			/* Preferred length size hint */
} AG_Scrollbar;

#define AGSCROLLBAR(p) ((AG_Scrollbar *)p)

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;
extern int agPrefScrollbarSize;

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
#if 0
AG_Scrollbar *AG_ScrollbarNewUint64(void *, enum ag_scrollbar_type, Uint,
                                    Uint64 *, Uint64 *, Uint64 *, Uint64 *);
AG_Scrollbar *AG_ScrollbarNewSint64(void *, enum ag_scrollbar_type, Uint,
                                    Sint64 *, Sint64 *, Sint64 *, Sint64 *);
#endif
AG_Scrollbar *AG_ScrollbarNewFloat(void *, enum ag_scrollbar_type, Uint,
                                   float *, float *, float *, float *);
AG_Scrollbar *AG_ScrollbarNewDouble(void *, enum ag_scrollbar_type, Uint,
                                    double *, double *, double *, double *);
#if 0
AG_Scrollbar *AG_ScrollbarNewLongDouble(void *, enum ag_scrollbar_type, Uint,
                                        long double *, long double *,
					long double *, long double *);
#endif

void AG_ScrollbarSizeHint(AG_Scrollbar *, int);
void AG_ScrollbarSetIncFn(AG_Scrollbar *, AG_EventFn, const char *, ...);
void AG_ScrollbarSetDecFn(AG_Scrollbar *, AG_EventFn, const char *, ...);
void AG_ScrollbarSetIntIncrement(AG_Scrollbar *, int);
void AG_ScrollbarSetRealIncrement(AG_Scrollbar *, double);

int  AG_ScrollbarVisible(AG_Scrollbar *);

/* Set and retrieve global scrollbar width preference. */
static __inline__ void
AG_ScrollbarSetPrefWidth(int n)
{
	agPrefScrollbarSize = n;
}
static __inline__ int
AG_ScrollbarPrefWidth(void)
{
	return (agPrefScrollbarSize);
}

/* Set and retrieve scrolling control length */
static __inline__ void
AG_ScrollbarSetControlLength(AG_Scrollbar *sb, int bsize)
{
	AG_ObjectLock(sb);
	sb->wBar = (bsize > 10 || bsize == -1) ? bsize : 10;
	sb->length = (sb->type == AG_SCROLLBAR_VERT) ? AGWIDGET(sb)->h :
	                                               AGWIDGET(sb)->w;
	sb->length -= sb->width*2;
	sb->length -= sb->wBar;
	AG_ObjectUnlock(sb);
}
static __inline__ int
AG_ScrollbarControlLength(AG_Scrollbar *sb)
{
	int rv;

	if (sb->wBar == -1) {
		rv = (sb->type == AG_SCROLLBAR_VERT) ? AGWIDGET(sb)->h :
		                                       AGWIDGET(sb)->w;
		rv -= sb->width*2;
		if (rv < 0) { rv = 0; }
	} else {
		rv = sb->wBar;
	}
	return (rv);
}

static __inline__ void
AG_ScrollbarSetWidth(AG_Scrollbar *sb, int width)
{
	AG_ObjectLock(sb);
	sb->width = width;
	AG_ObjectUnlock(sb);
}
static __inline__ int
AG_ScrollbarWidth(AG_Scrollbar *sb)
{
	return (sb->width);
}

#ifdef AG_LEGACY
# define AG_ScrollbarSetBarSize(sb,len)  AG_ScrollbarSetControlLength((sb),(len))
# define AG_ScrollbarGetBarSize(sb)	 AG_ScrollbarControlLength(sb)
# define AG_ScrollbarSetButtonSize(sb,w) AG_ScrollbarSetWidth((sb),(w))
# define AG_ScrollbarGetButtonSize(sb)	 AG_ScrollbarWidth(sb)
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SCROLLBAR_H_ */
