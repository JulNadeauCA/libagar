/*	$Csoft: matview.h,v 1.2 2005/09/11 07:05:58 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MATVIEW_H_
#define _AGAR_WIDGET_MATVIEW_H_

#include <mat/mat.h>

#include <engine/widget/widget.h>
#include <engine/widget/scrollbar.h>

#include "begin_code.h"

typedef struct ag_matview {
	struct ag_widget wid;
	struct mat *mat;		/* Matrix to view */
	u_int flags;
	int ent_w, ent_h;		/* Size of entry */
	const char *numfmt;		/* Numerical entry format */
	int pre_m, pre_n;		/* Prescale dimensions */
	int hspace, vspace;		/* Spacing between entries */
	int xoffs, yoffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	AG_Scrollbar *hbar, *vbar;	/* Display scrollbars */
} AG_Matview;

__BEGIN_DECLS
AG_Matview *AG_MatviewNew(void *, struct mat *, u_int);
void	    AG_MatviewInit(AG_Matview *, struct mat *, u_int);
void	    AG_MatviewScale(void *, int, int);
void	    AG_MatviewPrescale(AG_Matview *, const char *, u_int, u_int);
void	    AG_MatviewSetNumericalFmt(AG_Matview *, const char *);
void	    AG_MatviewDrawNumerical(void *);
void	    AG_MatviewDrawGreyscale(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MATVIEW_H_ */
