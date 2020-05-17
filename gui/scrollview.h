/*	Public domain	*/

#ifndef _AGAR_GUI_SCROLLVIEW_H_
#define _AGAR_GUI_SCROLLVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/begin.h>

enum ag_scrollview_type {
	AG_SCROLLVIEW_HORIZ,
	AG_SCROLLVIEW_VERT
};

enum ag_scrollview_style {
	AG_SCROLLVIEW_STYLE_NONE,	/* No graphic */
	AG_SCROLLVIEW_STYLE_BOX,	/* 3D raised box */
	AG_SCROLLVIEW_STYLE_WELL,	/* 3D well */
	AG_SCROLLVIEW_STYLE_PLAIN,	/* Filled rectangle */
	AG_SCROLLVIEW_STYLE_LAST
};

typedef struct ag_scrollview {
	struct ag_widget wid;		/* AG_Widget -> AG_Scrollview */
	Uint flags;
#define AG_SCROLLVIEW_HFILL        0x001
#define AG_SCROLLVIEW_VFILL        0x002
#define AG_SCROLLVIEW_NOPAN_X      0x004 /* X is not pannable */ 
#define AG_SCROLLVIEW_NOPAN_Y      0x008 /* Y is not pannable */
#define AG_SCROLLVIEW_PANNING      0x010 /* Panning in progress */
#define AG_SCROLLVIEW_BY_MOUSE     0x020 /* Panning with mouse allowed */
#define AG_SCROLLVIEW_FRAME        0x080 /* Draw background and frame */
#define AG_SCROLLVIEW_PAN_RIGHT    0x100 /* Right-button pannable */
#define AG_SCROLLVIEW_PAN_LEFT     0x200 /* Left-button pannable */
#define AG_SCROLLVIEW_EXPAND       (AG_SCROLLVIEW_HFILL|AG_SCROLLVIEW_VFILL)
#define AG_SCROLLVIEW_NOPAN_XY     (AG_SCROLLVIEW_NOPAN_X|AG_SCROLLVIEW_NOPAN_Y)

	enum ag_scrollview_type type;	/* Packing mode */
	enum ag_scrollview_style style;	/* Graphical style */
	Uint32 _pad;
	int wPre, hPre;			/* Requested geometry */
	int xOffs, yOffs;		/* Display offset */
	int xMin, xMax, yMin, yMax;	/* Display boundaries */
	int wBar, hBar;			/* Effective scrollbar sizes */
	AG_Rect r;			/* Widget space (scrollbars excluded) */
	AG_Scrollbar *_Nullable hbar;	/* Horizontal scrollbar */
	AG_Scrollbar *_Nullable vbar;	/* Vertical scrollbar */
} AG_Scrollview;

#define AGSCROLLVIEW(obj)            ((AG_Scrollview *)(obj))
#define AGCSCROLLVIEW(obj)           ((const AG_Scrollview *)(obj))
#define AG_SCROLLVIEW_SELF()          AGSCROLLVIEW( AG_OBJECT(0,"AG_Widget:AG_Scrollview:*") )
#define AG_SCROLLVIEW_PTR(n)          AGSCROLLVIEW( AG_OBJECT((n),"AG_Widget:AG_Scrollview:*") )
#define AG_SCROLLVIEW_NAMED(n)        AGSCROLLVIEW( AG_OBJECT_NAMED((n),"AG_Widget:AG_Scrollview:*") )
#define AG_CONST_SCROLLVIEW_SELF()   AGCSCROLLVIEW( AG_CONST_OBJECT(0,"AG_Widget:AG_Scrollview:*") )
#define AG_CONST_SCROLLVIEW_PTR(n)   AGCSCROLLVIEW( AG_CONST_OBJECT((n),"AG_Widget:AG_Scrollview:*") )
#define AG_CONST_SCROLLVIEW_NAMED(n) AGCSCROLLVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Scrollview:*") )

__BEGIN_DECLS
extern AG_WidgetClass agScrollviewClass;

AG_Scrollview *_Nonnull AG_ScrollviewNew(void *_Nullable, Uint);
void AG_ScrollviewSizeHint(AG_Scrollview *_Nonnull, Uint,Uint);
void AG_ScrollviewSetIncrement(AG_Scrollview *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SCROLLVIEW_H_ */
