/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

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
#define AG_SCROLLBAR_FOCUSABLE	0x04
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL|AG_SCROLLBAR_VFILL)
	int value;			/* Default value binding */
	int min, max;			/* Default range binding */
	int visible;			/* Number of visible items */
	enum ag_scrollbar_type type;
	int bw;				/* Effective button size */
	int bwDefault;			/* Default button size */
	enum ag_scrollbar_button curBtn; /* Active button */
	int barSz;			/* Scroll bar size */
	int arrowSz;			/* Arrow height */
	AG_Event *buttonIncFn;		/* Alt. handler for increment btns */
	AG_Event *buttonDecFn;		/* Alt. handler for decrement btns */
} AG_Scrollbar;

#define AGSCROLLBAR(p) ((AG_Scrollbar *)p)

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;

AG_Scrollbar *AG_ScrollbarNew(void *, enum ag_scrollbar_type, Uint);
int           AG_ScrollbarVisible(AG_Scrollbar *);
void          AG_ScrollbarSetIncFn(AG_Scrollbar *, AG_EventFn, const char *,
                                   ...);
void          AG_ScrollbarSetDecFn(AG_Scrollbar *, AG_EventFn, const char *,
                                   ...);

static __inline__ void
AG_ScrollbarSetBarSize(AG_Scrollbar *sb, int bsize)
{
	AG_ObjectLock(sb);
	sb->barSz = (bsize > 10 || bsize == -1) ? bsize : 10;
	AG_ObjectUnlock(sb);
}
static __inline__ int
AG_ScrollbarGetBarSize(AG_Scrollbar *sb)
{
	int rv;
	AG_ObjectLock(sb);
	rv = sb->barSz;
	AG_ObjectUnlock(sb);
	return (rv);
}
static __inline__ void
AG_ScrollbarSetButtonSize(AG_Scrollbar *sb, int bw)
{
	AG_ObjectLock(sb);
	sb->bwDefault = bw;
	sb->bw = bw;
	AG_ObjectUnlock(sb);
}
static __inline__ int
AG_ScrollbarGetButtonSize(AG_Scrollbar *sb)
{
	int rv;
	AG_ObjectLock(sb);
	rv = sb->bw;
	AG_ObjectUnlock(sb);
	return (rv);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
