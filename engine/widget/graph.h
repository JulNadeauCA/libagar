/*	$Csoft: graph.h,v 1.18 2003/11/21 02:20:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GRAPH_H_
#define _AGAR_WIDGET_GRAPH_H_

#include <engine/widget/widget.h>
#include <engine/widget/label.h>

#include "begin_code.h"

typedef Sint16 AG_GraphValue;

struct ag_graph;

typedef struct ag_graph_item {
	char name[AG_LABEL_MAX];		/* Description */
	Uint32 color;				/* Line color */
	AG_GraphValue *vals;			/* Value array */
	Uint32 nvals;
	Uint32 maxvals;
	Uint32 limit;
	struct ag_graph *graph;			/* Back pointer */
	TAILQ_ENTRY(ag_graph_item) items;
} AG_GraphItem;

TAILQ_HEAD(ag_graph_itemq, ag_graph_item);

enum ag_graph_type {
	AG_GRAPH_POINTS,
	AG_GRAPH_LINES
};

typedef struct ag_graph {
	struct ag_widget wid;

	enum ag_graph_type type;
	char caption[AG_LABEL_MAX];
	int flags;
#define AG_GRAPH_SCROLL	0x01		/* Scroll if the end is not visible */
#define AG_GRAPH_ORIGIN	0x02		/* Visible origin */
	AG_GraphValue yrange;		/* Max. value */
	AG_GraphValue xoffs;		/* Display offset */
	int origin_y;			/* Origin position (%) */
	struct ag_graph_itemq items;	/* Items to plot */
} AG_Graph;

__BEGIN_DECLS
AG_Graph *AG_GraphNew(void *, const char *, enum ag_graph_type, int,
	              AG_GraphValue);

AG_GraphItem *AG_GraphAddItem(AG_Graph *, const char *, Uint8, Uint8, Uint8,
		              Uint32);
void AG_GraphFreeItems(AG_Graph *);

void AG_GraphInit(AG_Graph *, const char *, enum ag_graph_type, int,
	          AG_GraphValue);
void AG_GraphDestroy(void *);
void AG_GraphDraw(void *);
void AG_GraphScale(void *, int, int);

void AG_GraphPlot(AG_GraphItem *, AG_GraphValue);
__inline__ void AG_GraphScroll(AG_Graph *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GRAPH_H_ */
