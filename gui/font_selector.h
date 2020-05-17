/*	Public domain	*/

#ifndef _AGAR_WIDGET_FONT_SELECTOR_H_
#define _AGAR_WIDGET_FONT_SELECTOR_H_

#include <agar/gui/pane.h>
#include <agar/gui/box.h>
#include <agar/gui/tlist.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/numerical.h>

#include <agar/gui/begin.h>

typedef struct ag_font_selector {
	AG_Widget wid;                      /* AG_Widget -> AG_FontSelector */

	Uint flags;
#define AG_FONTSELECTOR_UPDATE         0x001  /* Refresh listing */
#define AG_FONTSELECTOR_ALT_PHRASE     0x002  /* Different preview phrase */
#define AG_FONTSELECTOR_SW_STYLES      0x004  /* Display "Software styles" */
#define AG_FONTSELECTOR_OBLIQUE_STYLES 0x008  /* Distinguish Italic, Oblique */
#define AG_FONTSELECTOR_HFILL          0x100
#define AG_FONTSELECTOR_VFILL          0x200
#define AG_FONTSELECTOR_EXPAND        (AG_FONTSELECTOR_HFILL | AG_FONTSELECTOR_VFILL)

	char curFace[AG_OBJECT_NAME_MAX];	/* Current font face */
	Uint curStyle;				/* Current style flags */
	int  sPreview;				/* Preview surface */
	float curSize;				/* Current size */
	Uint32 _pad1;

	AG_Pane *_Nonnull  hPane;		/* Base container */
	AG_Pane *_Nonnull  hPane2;		/* Right horizontal container */
	AG_Tlist *_Nonnull tlFaces;		/* List of font faces */
	AG_Tlist *_Nonnull tlStyles;		/* List of font styles */
	AG_Tlist *_Nonnull tlSizes;		/* List of available sizes */
	AG_HSVPal *_Nonnull pal;		/* Preview color picker */

	AG_Rect rPreview;			/* Preview area */
	AG_Font *_Nullable font;		/* Default `font' binding */
	AG_Color cPreviewBG, cPreviewFG;	/* Preview colors */
} AG_FontSelector;

#define AGFONTSELECTOR(obj)            ((AG_FontSelector *)(obj))
#define AGCFONTSELECTOR(obj)           ((const AG_FontSelector *)(obj))
#define AG_FONTSELECTOR_SELF()          AGFONTSELECTOR( AG_OBJECT(0,"AG_Widget:AG_FontSelector:*") )
#define AG_FONTSELECTOR_PTR(n)          AGFONTSELECTOR( AG_OBJECT((n),"AG_Widget:AG_FontSelector:*") )
#define AG_FONTSELECTOR_NAMED(n)        AGFONTSELECTOR( AG_OBJECT_NAMED((n),"AG_Widget:AG_FontSelector:*") )
#define AG_CONST_FONTSELECTOR_SELF()   AGCFONTSELECTOR( AG_CONST_OBJECT(0,"AG_Widget:AG_FontSelector:*") )
#define AG_CONST_FONTSELECTOR_PTR(n)   AGCFONTSELECTOR( AG_CONST_OBJECT((n),"AG_Widget:AG_FontSelector:*") )
#define AG_CONST_FONTSELECTOR_NAMED(n) AGCFONTSELECTOR( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_FontSelector:*") )

__BEGIN_DECLS
extern AG_WidgetClass agFontSelectorClass;

AG_FontSelector *_Nonnull AG_FontSelectorNew(void *_Nullable, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FONT_SELECTOR_H_ */
