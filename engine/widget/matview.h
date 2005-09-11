/*	$Csoft: matview.h,v 1.1 2005/09/11 02:33:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MATVIEW_H_
#define _AGAR_WIDGET_MATVIEW_H_

#include <mat/mat.h>

#include <engine/widget/widget.h>
#include <engine/widget/scrollbar.h>

#include "begin_code.h"

struct matview {
	struct widget wid;
	struct mat *mat;		/* Matrix to view */
	u_int flags;
	int ent_w, ent_h;		/* Size of entry */
	const char *numfmt;		/* Numerical entry format */
	int pre_m, pre_n;		/* Prescale dimensions */
	int hspace, vspace;		/* Spacing between entries */
	int xoffs, yoffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	struct scrollbar *hbar, *vbar;	/* Display scrollbars */
};

__BEGIN_DECLS
struct matview *matview_new(void *, struct mat *, u_int);
void matview_init(struct matview *, struct mat *, u_int);
void matview_scale(void *, int, int);
void matview_prescale(struct matview *, const char *, u_int, u_int);
void matview_set_numfmt(struct matview *, const char *);
void matview_draw_numerical(void *);
void matview_draw_greyscale(void *);
void matview_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MATVIEW_H_ */
