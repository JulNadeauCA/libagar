/*	Public domain	*/

#ifndef _AGAR_GUI_FIXED_PLOTTER_H_
#define _AGAR_GUI_FIXED_PLOTTER_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

#ifndef AG_FIXED_PLOTTER_LABEL_MAX
#define AG_FIXED_PLOTTER_LABEL_MAX 124
#endif

typedef int AG_FixedPlotterValue;

struct ag_fixed_plotter;

typedef struct ag_fixed_plotter_item {
	char name[AG_FIXED_PLOTTER_LABEL_MAX];		/* Description */
	AG_Color color;					/* Line color */

	Uint32                        nvals;		/* Values total */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
	AG_FixedPlotterValue *_Nonnull vals;		/* Values array */
	Uint32                      maxvals;		/* Values allocated */
	Uint32                        limit;		/* Hard limit */
	struct ag_fixed_plotter *_Nonnull fpl;		/* Back pointer */
	AG_TAILQ_ENTRY(ag_fixed_plotter_item) items;
} AG_FixedPlotterItem;

AG_TAILQ_HEAD(ag_fixed_plotter_itemq, ag_fixed_plotter_item);

enum ag_fixed_plotter_type {
	AG_FIXED_PLOTTER_POINTS,
	AG_FIXED_PLOTTER_LINES
};

typedef struct ag_fixed_plotter {
	struct ag_widget wid;		/* AG_Widget -> AG_FixedPlotter */

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
	Uint32 _pad;
	struct ag_fixed_plotter_itemq items;	/* Items to plot */
} AG_FixedPlotter;

#define AGFIXEDPLOTTER(obj)            ((AG_FixedPlotter *)(obj))
#define AGCFIXEDPLOTTER(obj)           ((const AG_FixedPlotter *)(obj))
#define AG_FIXEDPLOTTER_SELF()          AGFIXEDPLOTTER( AG_OBJECT(0,"AG_Widget:AG_FixedPlotter:*") )
#define AG_FIXEDPLOTTER_PTR(n)          AGFIXEDPLOTTER( AG_OBJECT((n),"AG_Widget:AG_FixedPlotter:*") )
#define AG_FIXEDPLOTTER_NAMED(n)        AGFIXEDPLOTTER( AG_OBJECT_NAMED((n),"AG_Widget:AG_FixedPlotter:*") )
#define AG_CONST_FIXEDPLOTTER_SELF()   AGCFIXEDPLOTTER( AG_CONST_OBJECT(0,"AG_Widget:AG_FixedPlotter:*") )
#define AG_CONST_FIXEDPLOTTER_PTR(n)   AGCFIXEDPLOTTER( AG_CONST_OBJECT((n),"AG_Widget:AG_FixedPlotter:*") )
#define AG_CONST_FIXEDPLOTTER_NAMED(n) AGCFIXEDPLOTTER( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_FixedPlotter:*") )

__BEGIN_DECLS
extern AG_WidgetClass agFixedPlotterClass;

AG_FixedPlotter *_Nonnull AG_FixedPlotterNew(void *_Nullable,
                                             enum ag_fixed_plotter_type, Uint);

AG_FixedPlotterItem *_Nonnull AG_FixedPlotterCurve(AG_FixedPlotter *_Nonnull,
						   const char *_Nonnull,
						   Uint8,Uint8,Uint8, Uint32);

void AG_FixedPlotterFreeItems(AG_FixedPlotter *_Nonnull);
void AG_FixedPlotterSetRange(AG_FixedPlotter *_Nonnull, AG_FixedPlotterValue);
void AG_FixedPlotterDatum(AG_FixedPlotterItem *_Nonnull, AG_FixedPlotterValue);
void AG_FixedPlotterScroll(AG_FixedPlotter *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_FIXED_PLOTTER_H_ */
