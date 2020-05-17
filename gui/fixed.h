/*	Public domain	*/

#ifndef _AGAR_WIDGET_FIXED_H_
#define _AGAR_WIDGET_FIXED_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

enum ag_fixed_style {
	AG_FIXED_STYLE_NONE,	/* No graphic */
	AG_FIXED_STYLE_BOX,	/* 3D raised box */
	AG_FIXED_STYLE_WELL,	/* 3D well */
	AG_FIXED_STYLE_PLAIN,	/* Filled rectangle */
	AG_FIXED_STYLE_LAST
};

typedef struct ag_fixed {
	struct ag_widget wid;		/* AG_Widget -> AG_Fixed */
	Uint flags;
#define AG_FIXED_HFILL     0x01         /* Expand to fill available width */
#define AG_FIXED_VFILL     0x02         /* Expand to fill available height */
#define AG_FIXED_NO_UPDATE 0x04         /* Don't call WINDOW_UPDATE() */
#define AG_FIXED_EXPAND   (AG_FIXED_HFILL | AG_FIXED_VFILL)
	enum ag_fixed_style style;	/* Graphical style */
	int wPre, hPre;			/* Size hint */
} AG_Fixed;

#define AGFIXED(obj)            ((AG_Fixed *)(obj))
#define AGCFIXED(obj)           ((const AG_Fixed *)(obj))
#define AG_FIXED_SELF()          AGFIXED( AG_OBJECT(0,"AG_Widget:AG_Fixed:*") )
#define AG_FIXED_PTR(n)          AGFIXED( AG_OBJECT((n),"AG_Widget:AG_Fixed:*") )
#define AG_FIXED_NAMED(n)        AGFIXED( AG_OBJECT_NAMED((n),"AG_Widget:AG_Fixed:*") )
#define AG_CONST_FIXED_SELF()   AGCFIXED( AG_CONST_OBJECT(0,"AG_Widget:AG_Fixed:*") )
#define AG_CONST_FIXED_PTR(n)   AGCFIXED( AG_CONST_OBJECT((n),"AG_Widget:AG_Fixed:*") )
#define AG_CONST_FIXED_NAMED(n) AGCFIXED( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Fixed:*") )

__BEGIN_DECLS
extern AG_WidgetClass agFixedClass;

AG_Fixed *_Nonnull AG_FixedNew(void *_Nullable, Uint);

void AG_FixedSetStyle(AG_Fixed *_Nonnull, enum ag_fixed_style);
void AG_FixedSizeHint(AG_Fixed *_Nonnull, int,int);
void AG_FixedPut(AG_Fixed *_Nonnull, void *_Nonnull, int,int);
void AG_FixedDel(AG_Fixed *_Nonnull, void *_Nonnull);
void AG_FixedSize(AG_Fixed *_Nonnull, void *_Nonnull, int,int);
void AG_FixedMove(AG_Fixed *_Nonnull, void *_Nonnull, int,int);

#ifdef AG_LEGACY
#define AG_FIXED_FILLBG 0x08
#define AG_FIXED_BOX    0x10
#define AG_FIXED_INVBOX	0x20
#define AG_FIXED_FRAME  0x40
#define AG_FixedPrescale(f,w,h) AG_FixedSizeHint((f),(w),(h))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FIXED_H_ */
