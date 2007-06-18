/*	Public domain	*/

#ifndef _AGAR_SG_MATVIEW_H_
#define _AGAR_SG_MATVIEW_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

typedef struct ag_matview {
	struct ag_widget wid;
	SG_Matrix *mat;			/* Matrix to view */
	Uint flags;
	int ent_w, ent_h;		/* Size of entry */
	const char *numfmt;		/* Numerical entry format */
	int pre_m, pre_n;		/* Prescale dimensions */
	int hspace, vspace;		/* Spacing between entries */
	int xoffs, yoffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	AG_Scrollbar *hbar, *vbar; /* Display scrollbars */
} AG_Matview;

__BEGIN_DECLS
AG_Matview *AG_MatviewNew(void *, SG_Matrix *, Uint);
void	    AG_MatviewInit(AG_Matview *, SG_Matrix *, Uint);
void	    AG_MatviewScale(void *, int, int);
void	    AG_MatviewPrescale(AG_Matview *, const char *, Uint, Uint);
void	    AG_MatviewSetNumericalFmt(AG_Matview *, const char *);
void	    AG_MatviewDrawNumerical(void *);
void	    AG_MatviewDrawGreyscale(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MATVIEW_H_ */
