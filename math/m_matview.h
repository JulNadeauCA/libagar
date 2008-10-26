/*	Public domain	*/

#ifndef _AGAR_MATH_M_MATVIEW_H_
#define _AGAR_MATH_M_MATVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>

#include <agar/math/begin.h>

enum m_matview_mode {
	M_MATVIEW_GREYSCALE,
	M_MATVIEW_NUMERICAL
};

struct ag_text_cache;

typedef struct m_matview {
	struct ag_widget wid;
	M_Matrix *matrix;	   	/* Matrix to view */
	Uint flags;
	enum m_matview_mode mode;	/* Mode of display */
	int wEnt, hEnt;			/* Size of entry in pixels */
	const char *numFmt;		/* Numerical entry format */
	int mPre, nPre;			/* Size hint */
	int hSpacing, vSpacing;		/* Spacing between entries */
	int xOffs, yOffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	AG_Scrollbar *hBar, *vBar;	/* Display scrollbars */
	struct ag_text_cache *tCache;	/* For numerical display */
	AG_Rect r;			/* View area */
} M_Matview;

__BEGIN_DECLS
extern AG_WidgetClass mMatviewClass;

M_Matview *M_MatviewNew(void *, M_Matrix *, Uint);
void       M_MatviewSizeHint(M_Matview *, const char *, Uint, Uint);
void       M_MatviewSetMatrix(M_Matview *, M_Matrix *);
void       M_MatviewSetDisplayMode(M_Matview *, enum m_matview_mode);
void       M_MatviewSetNumericalFmt(M_Matview *, const char *);
__END_DECLS

#include <agar/math/close.h>
#endif /* _AGAR_MATH_M_MATVIEW_H_ */
