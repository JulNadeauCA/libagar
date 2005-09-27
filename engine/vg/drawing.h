/*	$Csoft: drawing.h,v 1.2 2005/09/19 01:25:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_DRAWING_H_
#define _AGAR_VG_DRAWING_H_

#include <engine/vg/vg.h>

#include "begin_code.h"

typedef struct ag_drawing {
	struct ag_object obj;
	VG *vg;
} AG_Drawing;

__BEGIN_DECLS
void	 AG_DrawingInit(void *, const char *);
void	 AG_DrawingReinit(void *);
void	 AG_DrawingDestroy(void *);
void	*AG_DrawingEdit(void *);
int	 AG_DrawingLoad(void *, AG_Netbuf *);
int	 AG_DrawingSave(void *, AG_Netbuf *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VG_DRAWING_H_ */
