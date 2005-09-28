/*	$Csoft: box.h,v 1.6 2005/09/27 00:25:22 vedge Exp $	*/
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
} AG_Fixed;

__BEGIN_DECLS
AG_Fixed *AG_FixedNew(void *, u_int);
void	  AG_FixedInit(AG_Fixed *, u_int);
void	  AG_FixedDestroy(void *);
void	  AG_FixedDraw(void *);
void	  AG_FixedScale(void *, int, int);
void	  AG_FixedPut(AG_Fixed *, void *, int, int);
void	  AG_FixedDel(AG_Fixed *, void *);
void	  AG_FixedSize(AG_Fixed *, void *, int, int);
void	  AG_FixedMove(AG_Fixed *, void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_FIXED_H_ */
