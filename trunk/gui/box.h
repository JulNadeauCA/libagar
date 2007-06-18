/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

#define AG_BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define AG_BOX_HFILL		0x02	/* Expand to fill available width */
#define AG_BOX_VFILL		0x04	/* Expand to fill available height */
#define AG_BOX_FRAME		0x08	/* Display a frame */
#define AG_BOX_EXPAND		(AG_BOX_HFILL|AG_BOX_VFILL)

enum ag_box_type {
	AG_BOX_HORIZ,
	AG_BOX_VERT
};

typedef struct ag_box {
	struct ag_widget wid;
	enum ag_box_type type;
	AG_Mutex lock;
	int homogenous;			/* Divide space evenly */
	int padding;			/* Padding around widgets */
	int spacing;			/* Spacing between widgets */
	int depth;			/* Depth of frame (for AG_BOX_FRAME) */
} AG_Box;

__BEGIN_DECLS
AG_Box	*AG_BoxNew(void *, enum ag_box_type, Uint);
AG_Box	*AG_BoxNewHorizPack(void *, Uint);
#define  AG_BoxNewHoriz(p,f) AG_BoxNew((p),AG_BOX_HORIZ,(f))
#define  AG_BoxNewVert(p,f) AG_BoxNew((p),AG_BOX_VERT,(f))

void	 AG_BoxInit(AG_Box *, enum ag_box_type, Uint);
void	 AG_BoxDestroy(void *);
void	 AG_BoxDraw(void *);
void	 AG_BoxScale(void *, int, int);

void	 AG_BoxSetHomogenous(AG_Box *, int);
void	 AG_BoxSetPadding(AG_Box *, int);
void	 AG_BoxSetSpacing(AG_Box *, int);
void	 AG_BoxSetDepth(AG_Box *, int);
void	 AG_BoxSetType(AG_Box *, enum ag_box_type);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
