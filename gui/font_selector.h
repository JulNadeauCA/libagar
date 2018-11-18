/*	Public domain	*/

#ifndef _AGAR_WIDGET_FONT_SELECTOR_H_
#define _AGAR_WIDGET_FONT_SELECTOR_H_

#include <agar/gui/pane.h>
#include <agar/gui/box.h>
#include <agar/gui/tlist.h>
#include <agar/gui/numerical.h>

#include <agar/gui/begin.h>

typedef struct ag_font_selector {
	AG_Widget wid;

	Uint flags;
#define AG_FONTSELECTOR_UPDATE	0x01	/* Refresh listing */
#define AG_FONTSELECTOR_HFILL	0x100
#define AG_FONTSELECTOR_VFILL	0x200
#define AG_FONTSELECTOR_EXPAND	(AG_FONTSELECTOR_HFILL|AG_FONTSELECTOR_VFILL)

	AG_Pane *_Nonnull  hPane;		/* Base container */
	AG_Pane *_Nonnull  hPane2;		/* Right horizontal container */
	AG_Box  *_Nonnull  sizeBox;		/* Font size selection area */
	AG_Tlist *_Nonnull tlFaces;		/* List of font faces */
	AG_Tlist *_Nonnull tlStyles;		/* List of font styles */
	AG_Tlist *_Nonnull tlSizes;		/* List of available sizes */

	char curFace[AG_OBJECT_NAME_MAX];	/* Current font face */
	Uint curStyle;				/* Current style flags */
	int  curSize;				/* Current size */

	AG_Rect rPreview;			/* Preview area */
	int     sPreview;			/* Preview surface */

	AG_Font *_Nullable font;			/* Default `font' binding */
} AG_FontSelector;

__BEGIN_DECLS
extern AG_WidgetClass agFontSelectorClass;

AG_FontSelector *_Nonnull AG_FontSelectorNew(void *_Nullable, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FONT_SELECTOR_H_ */
