/*	$Csoft: fixed.h,v 1.1 2005/09/28 15:46:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FIXED_H_
#define _AGAR_WIDGET_FIXED_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_fixed {
	struct ag_widget wid;
	u_int flags;
#define AG_FIXED_WFILL		0x01	/* Expand to fill available width */
#define AG_FIXED_HFILL		0x02	/* Expand to fill available height */
#define AG_FIXED_NO_UPDATE	0x04	/* Don't call WINDOW_UPDATE() */
#define AG_FIXED_FILLBG		0x08	/* Fill background */
#define AG_FIXED_BOX		0x10	/* Draw a box */
#define AG_FIXED_INVBOX		0x20	/* Draw a box */
#define AG_FIXED_FRAME		0x40	/* Draw a frame */
} AG_Fixed;

__BEGIN_DECLS
AG_Fixed *AG_FixedNew(void *, u_int);
void	  AG_FixedInit(AG_Fixed *, u_int);
void	  AG_FixedDestroy(void *);
void	  AG_FixedDrawBg(void *);
void	  AG_FixedDrawBox(void *);
void	  AG_FixedDrawInvBox(void *);
void	  AG_FixedDrawFrame(void *);
void	  AG_FixedScale(void *, int, int);
void	  AG_FixedPut(AG_Fixed *, void *, int, int);
void	  AG_FixedDel(AG_Fixed *, void *);
void	  AG_FixedSize(AG_Fixed *, void *, int, int);
void	  AG_FixedMove(AG_Fixed *, void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FIXED_H_ */
