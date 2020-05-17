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
	struct ag_widget wid;		/* AG_Widget(3) -> M_Matview */
	M_Matrix *_Nullable matrix;	/* Matrix to display */
	Uint flags;
	enum m_matview_mode mode;	/* Mode of display */
	int wEnt, hEnt;			/* Size of entry in pixels */
	const char *_Nonnull numFmt;	/* Numerical entry format */
	int mPre, nPre;			/* Size hint */
	int hSpacing, vSpacing;		/* Spacing between entries */
	int xOffs, yOffs;		/* Display offset */
	int scale;			/* Scale (for graphic rendering) */
	Uint32 _pad;

	AG_Scrollbar *_Nonnull hBar;	/* Horizontal scrollbar */
	AG_Scrollbar *_Nonnull vBar;	/* Vertical scrollbar */

	struct ag_text_cache *_Nonnull tCache;	/* For numerical display */

	AG_Rect r;			/* View area */
} M_Matview;

#define MMATVIEW(obj)           ((M_Matview *)(obj))
#define MCMATVIEW(obj)          ((const M_Matview *)(obj))
#define M_MATVIEW_SELF()          MMATVIEW( AG_OBJECT(0,"AG_Widget:M_Matview:*") )
#define M_MATVIEW_PTR(n)          MMATVIEW( AG_OBJECT((n),"AG_Widget:M_Matview:*") )
#define M_MATVIEW_NAMED(n)        MMATVIEW( AG_OBJECT_NAMED((n),"AG_Widget:M_Matview:*") )
#define M_CONST_MATVIEW_SELF()   MCMATVIEW( AG_CONST_OBJECT(0,"AG_Widget:M_Matview:*") )
#define M_CONST_MATVIEW_PTR(n)   MCMATVIEW( AG_CONST_OBJECT((n),"AG_Widget:M_Matview:*") )
#define M_CONST_MATVIEW_NAMED(n) MCMATVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:M_Matview:*") )

__BEGIN_DECLS
extern AG_WidgetClass mMatviewClass;

M_Matview *_Nonnull M_MatviewNew(void *_Nullable, M_Matrix *_Nullable, Uint);
void M_MatviewSizeHint(M_Matview *_Nonnull, const char *_Nullable, Uint,Uint);
void M_MatviewSetMatrix(M_Matview *_Nonnull, M_Matrix *_Nonnull);
void M_MatviewSetDisplayMode(M_Matview *_Nonnull, enum m_matview_mode);
void M_MatviewSetNumericalFmt(M_Matview *_Nonnull, const char *_Nonnull);
__END_DECLS

#include <agar/math/close.h>
#endif /* _AGAR_MATH_M_MATVIEW_H_ */
