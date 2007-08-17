/*	Public domain	*/

#ifndef _AGAR_GUI_SCROLLABLE_TEXT_H_
#define _AGAR_GUI_SCROLLABLE_TEXT_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollable.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollable.h>
#endif

#include "begin_code.h"

typedef struct ag_scrollable_text {
	struct ag_scrollable sa;
	Uint flags;
#define AG_SCROLLABLE_TEXT_HFILL  0x01	/* Fill available width */
#define AG_SCROLLABLE_TEXT_VFILL  0x02	/* Fill available height */
#define AG_SCROLLABLE_TEXT_EXPAND (AG_SCROLLABLE_TEXT_HFILL|\
			           AG_SCROLLABLE_TEXT_VFILL)

	char  *text;
	size_t text_len;
	int surface;			/* Text surface */
} AG_ScrollableText;

__BEGIN_DECLS
extern const AG_WidgetOps agScrollableTextOps;

AG_ScrollableText  *AG_ScrollableTextNew(void *, Uint);
void		    AG_ScrollableTextInit(AG_ScrollableText *, Uint);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_GUI_SCROLLABLE_TEXT_H_ */
