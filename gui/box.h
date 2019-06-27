/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

enum ag_box_type {
	AG_BOX_HORIZ,
	AG_BOX_VERT
};

enum ag_box_align {
	AG_BOX_LEFT	= 0,
	AG_BOX_TOP	= 0,
	AG_BOX_CENTER	= 1,
	AG_BOX_MIDDLE	= 1,
	AG_BOX_RIGHT	= 2,
	AG_BOX_BOTTOM	= 2
};

struct ag_label;

typedef struct ag_box {
	struct ag_widget wid;		/* AG_Widget -> AG_Box */
	enum ag_box_type type;
	Uint flags;
#define AG_BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define AG_BOX_HFILL		0x02	/* Expand to fill available width */
#define AG_BOX_VFILL		0x04	/* Expand to fill available height */
#define AG_BOX_FRAME		0x08	/* Display a frame by default */
#define AG_BOX_EXPAND		(AG_BOX_HFILL|AG_BOX_VFILL)
	int padding;			/* Padding around widgets */
	int spacing;			/* Spacing between widgets */
	int depth;			/* Depth of frame (for AG_BOX_FRAME) */
	struct ag_label *_Nonnull lbl;	/* Optional text label */
	enum ag_box_align hAlign;	/* Horizontal alignment */
	enum ag_box_align vAlign;	/* Vertical alignment */
} AG_Box, AG_HBox, AG_VBox;

#define AG_HBOX_HOMOGENOUS AG_BOX_HOMOGENOUS
#define AG_HBOX_HFILL      AG_BOX_HFILL
#define AG_HBOX_VFILL      AG_BOX_VFILL
#define AG_HBOX_EXPAND    (AG_BOX_HFILL|AG_BOX_VFILL)

#define AG_VBOX_HOMOGENOUS AG_BOX_HOMOGENOUS
#define AG_VBOX_HFILL      AG_BOX_HFILL
#define AG_VBOX_VFILL      AG_BOX_VFILL
#define AG_VBOX_EXPAND    (AG_BOX_HFILL | AG_BOX_VFILL)

#define AGBOX(obj)            ((AG_Box *)(obj))
#define AGCBOX(obj)           ((const AG_Box *)(obj))
#define AG_BOX_SELF()          AGBOX( AG_OBJECT(0,"AG_Widget:AG_Box:*") )
#define AG_BOX_PTR(n)          AGBOX( AG_OBJECT((n),"AG_Widget:AG_Box:*") )
#define AG_BOX_NAMED(n)        AGBOX( AG_OBJECT_NAMED((n),"AG_Widget:AG_Box:*") )
#define AG_CONST_BOX_SELF()   AGCBOX( AG_CONST_OBJECT(0,"AG_Widget:AG_Box:*") )
#define AG_CONST_BOX_PTR(n)   AGCBOX( AG_CONST_OBJECT((n),"AG_Widget:AG_Box:*") )
#define AG_CONST_BOX_NAMED(n) AGCBOX( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Box:*") )

__BEGIN_DECLS
extern const char *agBoxHorizAlignNames[];
extern const char *agBoxVertAlignNames[];

extern AG_WidgetClass agBoxClass;

AG_Box *_Nonnull  AG_BoxNew(void *_Nullable, enum ag_box_type, Uint);
AG_Box *_Nonnull  AG_BoxNewHoriz(void *_Nullable, Uint);
AG_Box *_Nonnull  AG_BoxNewHorizNS(void *_Nullable, Uint);
AG_Box *_Nonnull  AG_BoxNewVert(void *_Nullable, Uint);
AG_Box *_Nonnull  AG_BoxNewVertNS(void *_Nullable, Uint);
AG_HBox *_Nonnull AG_HBoxNew(void *_Nullable, Uint);
AG_HBox *_Nonnull AG_HBoxNewNS(void *_Nullable, Uint);
AG_VBox *_Nonnull AG_VBoxNew(void *_Nullable, Uint);
AG_VBox *_Nonnull AG_VBoxNewNS(void *_Nullable, Uint);

void AG_BoxSetLabel(AG_Box *_Nonnull, const char *_Nullable, ...);
void AG_BoxSetLabelS(AG_Box *_Nonnull, const char *_Nullable);
void AG_BoxSetHomogenous(AG_Box *_Nonnull, int);
void AG_BoxSetPadding(AG_Box *_Nonnull, int);
void AG_BoxSetSpacing(AG_Box *_Nonnull, int);
void AG_BoxSetDepth(AG_Box *_Nonnull, int);
void AG_BoxSetType(AG_Box *_Nonnull, enum ag_box_type);
void AG_BoxSetHorizAlign(AG_Box *_Nonnull, enum ag_box_align);
void AG_BoxSetVertAlign(AG_Box *_Nonnull, enum ag_box_align);

#ifdef AG_LEGACY
#define AG_HBoxInit(b,fl)          AG_BoxInit((b),AG_BOX_HORIZ,(fl))
#define AG_HBoxSetHomogenous(b,fl) AG_BoxSetHomogenous((b),(fl))
#define AG_HBoxSetPadding(b,pad)   AG_BoxSetPadding((b),(pad))
#define AG_HBoxSetSpacing(b,sp)    AG_BoxSetSpacing((b),(sp))
#define AG_VBoxInit(b,fl)          AG_BoxInit((b),AG_BOX_VERT,(fl))
#define AG_VBoxSetHomogenous(b,fl) AG_BoxSetHomogenous((b),(fl))
#define AG_VBoxSetPadding(b,pad)   AG_BoxSetPadding((b),(pad))
#define AG_VBoxSetSpacing(b,sp)    AG_BoxSetSpacing((b),(sp))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_BOX_H_ */
