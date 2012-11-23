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
	struct ag_widget wid;
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
	struct ag_label *lbl;		/* Optional text label */
	enum ag_box_align hAlign, vAlign; /* Widget alignment */
} AG_Box;

#define AGBOX(p) ((AG_Box *)(p))

__BEGIN_DECLS
extern AG_WidgetClass agBoxClass;

AG_Box	*AG_BoxNew(void *, enum ag_box_type, Uint);

void     AG_BoxSetLabel(AG_Box *, const char *, ...);
void     AG_BoxSetLabelS(AG_Box *, const char *);
void	 AG_BoxSetHomogenous(AG_Box *, int);
void	 AG_BoxSetPadding(AG_Box *, int);
void	 AG_BoxSetSpacing(AG_Box *, int);
void	 AG_BoxSetDepth(AG_Box *, int);
void	 AG_BoxSetType(AG_Box *, enum ag_box_type);
void     AG_BoxSetHorizAlign(AG_Box *, enum ag_box_align);
void     AG_BoxSetVertAlign(AG_Box *, enum ag_box_align);

#define  AG_BoxNewHoriz(p,f) AG_BoxNew((p),AG_BOX_HORIZ,(f))
#define  AG_BoxNewVert(p,f) AG_BoxNew((p),AG_BOX_VERT,(f))

static __inline__ AG_Box *
AG_BoxNewHorizNS(void *p, Uint flags)
{
	AG_Box *hBox = AG_BoxNewHoriz(p, flags);
	AG_BoxSetSpacing(hBox, 0);
	AG_BoxSetPadding(hBox, 0);
	return (hBox);
}
static __inline__ AG_Box *
AG_BoxNewVertNS(void *p, Uint flags)
{
	AG_Box *vBox = AG_BoxNewVert(p, flags);
	AG_BoxSetSpacing(vBox, 0);
	AG_BoxSetPadding(vBox, 0);
	return (vBox);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_BOX_H_ */
