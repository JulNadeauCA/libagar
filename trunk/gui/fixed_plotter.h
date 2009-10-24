/*	Public domain	*/

#ifndef _AGAR_GUI_FIXED_PLOTTER_H_
#define _AGAR_GUI_FIXED_PLOTTER_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

typedef Sint16 AG_FixedPlotterValue;

struct ag_fixed_plotter;

typedef struct ag_fixed_plotter_item {
	char name[AG_LABEL_MAX];		/* Description */
	AG_Color color;				/* Line color */
	AG_FixedPlotterValue *vals;		/* Value array */
	Uint32 nvals;
	Uint32 maxvals;
	Uint32 limit;
	struct ag_fixed_plotter *fpl;			/* Back pointer */
	AG_TAILQ_ENTRY(ag_fixed_plotter_item) items;
} AG_FixedPlotterItem;

AG_TAILQ_HEAD(ag_fixed_plotter_itemq, ag_fixed_plotter_item);

enum ag_fixed_plotter_type {
	AG_FIXED_PLOTTER_POINTS,
	AG_FIXED_PLOTTER_LINES
};

typedef struct ag_fixed_plotter {
	struct ag_widget wid;

	enum ag_fixed_plotter_type type;
	Uint flags;
#define AG_FIXED_PLOTTER_SCROLL	0x01	/* Scroll if the end is not visible */
#define AG_FIXED_PLOTTER_XAXIS	0x02	/* Display X axis */
#define AG_FIXED_PLOTTER_HFILL	0x04
#define AG_FIXED_PLOTTER_VFILL	0x08
#define AG_FIXED_PLOTTER_EXPAND (AG_FIXED_PLOTTER_HFILL|AG_FIXED_PLOTTER_VFILL)
	AG_FixedPlotterValue yrange;		/* Max. value */
	AG_FixedPlotterValue xoffs;		/* Display offset */
	int yOrigin;				/* Origin position (%) */
	struct ag_fixed_plotter_itemq items;	/* Items to plot */
} AG_FixedPlotter;

__BEGIN_DECLS
extern AG_WidgetClass agFixedPlotterClass;

AG_FixedPlotter *AG_FixedPlotterNew(void *, enum ag_fixed_plotter_type, Uint);
AG_FixedPlotterItem *AG_FixedPlotterCurve(AG_FixedPlotter *, const char *,
		                          Uint8, Uint8, Uint8, Uint32);
void AG_FixedPlotterFreeItems(AG_FixedPlotter *);
void AG_FixedPlotterSetRange(AG_FixedPlotter *, AG_FixedPlotterValue);
void AG_FixedPlotterDatum(AG_FixedPlotterItem *, AG_FixedPlotterValue);

static __inline__ void
AG_FixedPlotterScroll(AG_FixedPlotter *fpl, int i)
{
	if (fpl->flags & AG_FIXED_PLOTTER_SCROLL)
		fpl->xoffs += i;
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_FIXED_PLOTTER_H_ */
