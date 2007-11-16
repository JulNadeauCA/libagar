/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

enum ag_box_type {
	AG_BOX_HORIZ,
	AG_BOX_VERT
};

typedef struct ag_box {
	struct ag_widget wid;
	enum ag_box_type type;
	AG_Mutex lock;
	Uint flags;
#define AG_BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define AG_BOX_HFILL		0x02	/* Expand to fill available width */
#define AG_BOX_VFILL		0x04	/* Expand to fill available height */
#define AG_BOX_FRAME		0x08	/* Display a frame */
#define AG_BOX_EXPAND		(AG_BOX_HFILL|AG_BOX_VFILL)
	int padding;			/* Padding around widgets */
	int spacing;			/* Spacing between widgets */
	int depth;			/* Depth of frame (for AG_BOX_FRAME) */
} AG_Box;

#define AGBOX(p) ((AG_Box *)(p))

__BEGIN_DECLS
extern const AG_WidgetClass agBoxClass;

AG_Box	*AG_BoxNew(void *, enum ag_box_type, Uint);
#define  AG_BoxNewHoriz(p,f) AG_BoxNew((p),AG_BOX_HORIZ,(f))
#define  AG_BoxNewVert(p,f) AG_BoxNew((p),AG_BOX_VERT,(f))

void	 AG_BoxSizeRequest(void *, AG_SizeReq *);
int	 AG_BoxSizeAllocate(void *, const AG_SizeAlloc *);

void	 AG_BoxSetHomogenous(AG_Box *, int);
void	 AG_BoxSetPadding(AG_Box *, int);
void	 AG_BoxSetSpacing(AG_Box *, int);
void	 AG_BoxSetDepth(AG_Box *, int);
void	 AG_BoxSetType(AG_Box *, enum ag_box_type);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
