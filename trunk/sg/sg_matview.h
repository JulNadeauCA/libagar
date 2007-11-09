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

enum sg_matview_mode {
	SG_MATVIEW_GREYSCALE,
	SG_MATVIEW_NUMERICAL
};

typedef struct sg_matview {
	struct ag_widget wid;
	SG_Matrix *mat;			/* Matrix to view */
	Uint flags;
	enum sg_matview_mode mode;	/* Display mode */
	int ent_w, ent_h;		/* Size of entry */
	const char *numfmt;		/* Numerical entry format */
	int pre_m, pre_n;		/* SizeHint dimensions */
	int hspace, vspace;		/* Spacing between entries */
	int xoffs, yoffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	AG_Scrollbar *hbar, *vbar;	/* Display scrollbars */
} SG_Matview;

__BEGIN_DECLS
extern const AG_WidgetOps sgMatviewOps;

SG_Matview *SG_MatviewNew(void *, SG_Matrix *, Uint);
void	    SG_MatviewSizeHint(SG_Matview *, const char *, Uint, Uint);
void	    SG_MatviewSetNumericalFmt(SG_Matview *, const char *);
void	    SG_MatviewSetDisplayMode(SG_Matview *, enum sg_matview_mode);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MATVIEW_H_ */
