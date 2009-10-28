/*	Public domain	*/

#ifndef _AGAR_SC_MATVIEW_H_
#define _AGAR_SC_MATVIEW_H_

#ifdef _AGAR_INTERNAL
#include <sc/sc.h>
#include <gui/widget.h>
#include <gui/scrollbar.h>
#else
#include <agar/sc/sc.h>
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

enum sc_matview_mode {
	SC_MATVIEW_GREYSCALE,
	SC_MATVIEW_NUMERICAL
};

typedef struct sc_matview {
	struct ag_widget wid;
	struct sc_matrix *mat;		/* Matrix to view */
	Uint flags;
	enum sc_matview_mode mode;	/* Mode of display */
	int ent_w, ent_h;		/* Size of entry */
	const char *numfmt;		/* Numerical entry format */
	int pre_m, pre_n;		/* Size hint */
	int hspace, vspace;		/* Spacing between entries */
	int xoffs, yoffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	AG_Scrollbar *hbar, *vbar;	/* Display scrollbars */
} SC_Matview;

__BEGIN_DECLS
extern AG_WidgetClass scMatviewClass;

SC_Matview *SC_MatviewNew(void *, struct sc_matrix *, Uint);
void	    SC_MatviewSizeHint(SC_Matview *, const char *, Uint, Uint);
void        SC_MatviewSetMatrix(SC_Matview *, struct sc_matrix *);
void	    SC_MatviewSetDisplayMode(SC_Matview *, enum sc_matview_mode);
void	    SC_MatviewSetNumericalFmt(SC_Matview *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MATVIEW_H_ */