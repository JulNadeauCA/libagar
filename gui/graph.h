/*	Public domain	*/

#ifndef _AGAR_GUI_GRAPH_H_
#define _AGAR_GUI_GRAPH_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

#define AG_GRAPH_NDEFCOLORS	16
#define AG_GRAPH_LABEL_MAX	64

struct ag_graph_edge;
struct ag_popup_menu;

enum ag_graph_vertex_style {	/* Vertex style */
	AG_GRAPH_RECTANGLE,	/* Rectangular box */
	AG_GRAPH_CIRCLE,	/* Circle */
};

typedef struct ag_graph_vertex {
	char labelTxt[AG_GRAPH_LABEL_MAX]; /* Label text */
	int  labelSu;			/* Text surface handle */
	Uint32 labelColor;		/* Text color (surfaceFmt) */
	Uint32 bgColor;			/* Background color (surfaceFmt) */
	enum ag_graph_vertex_style style; /* Vertex style */
	Uint flags;
#define AG_GRAPH_MOUSEOVER	0x01
#define AG_GRAPH_SELECTED	0x02
#define AG_GRAPH_HIDDEN		0x04
#define AG_GRAPH_AUTOPLACED	0x08
	int x, y;				/* Coordinates */
	Uint w, h;				/* Bounding box geometry */
	void *userPtr;				/* User pointer */
	struct ag_graph_edge **edges;		/* Back pointers to edges */
	Uint nedges;
	struct ag_graph *graph;			/* Back pointer to graph */
	AG_TAILQ_ENTRY(ag_graph_vertex) vertices;
	AG_TAILQ_ENTRY(ag_graph_vertex) sorted;	/* For autoplacer */
	struct ag_popup_menu *popupMenu;
} AG_GraphVertex;

typedef struct ag_graph_edge {
	char labelTxt[AG_GRAPH_LABEL_MAX];	/* Label text */
	int  labelSu;				/* Text surface handle */
	Uint32 edgeColor;			/* Edge color (surfaceFmt) */
	Uint32 labelColor;			/* Label color (surfaceFmt) */
	Uint flags;
/*#define AG_GRAPH_MOUSEOVER	0x01 */
/*#define AG_GRAPH_SELECTED	0x02 */
/*#define AG_GRAPH_HIDDEN	0x04 */
	AG_GraphVertex *v1, *v2;		/* Connected vertices */
	void *userPtr;				/* User pointer */
	struct ag_graph *graph;			/* Back pointer to graph */
	AG_TAILQ_ENTRY(ag_graph_edge) edges;
	struct ag_popup_menu *popupMenu;
} AG_GraphEdge;

typedef struct ag_graph {
	struct ag_widget wid;
	Uint flags;
#define AG_GRAPH_HFILL		0x01
#define AG_GRAPH_VFILL		0x02
#define AG_GRAPH_EXPAND		(AG_GRAPH_HFILL|AG_GRAPH_VFILL)
#define AG_GRAPH_SCROLL		0x04
#define AG_GRAPH_DRAGGING	0x08	/* Vertex is being moved (readonly) */
#define AG_GRAPH_PANNING	0x10	/* View is being panned (readonly) */
#define AG_GRAPH_NO_MOVE	0x20	/* User cannot move vertices */
#define AG_GRAPH_NO_SELECT	0x40	/* User cannot select vertices */
#define AG_GRAPH_NO_MENUS	0x80	/* Disable popup menus */
#define AG_GRAPH_READONLY	(AG_GRAPH_NO_MOVE|AG_GRAPH_NO_SELECT| \
                         	 AG_GRAPH_NO_MENUS)

	int wPre, hPre;			/* Requested geometry */
	int xOffs, yOffs;		/* Display offset */
	int xMin, xMax, yMin, yMax;	/* Display boundaries */
	AG_Scrollbar *hbar, *vbar;	/* Scrollbars for panning */

	AG_TAILQ_HEAD(,ag_graph_vertex) vertices;	/* Graph vertices */
	AG_TAILQ_HEAD(,ag_graph_edge) edges;		/* Graph edges */
	Uint nvertices, nedges;	
	int pxMin, pxMax, pyMin, pyMax;		/* Bounds of last cluster
						   (for autoplacer) */
} AG_Graph;

__BEGIN_DECLS
extern AG_WidgetClass agGraphClass;

AG_Graph	*AG_GraphNew(void *, Uint);
void		 AG_GraphFreeVertices(AG_Graph *);
void		 AG_GraphSizeHint(AG_Graph *, Uint, Uint);
 
AG_GraphVertex *AG_GraphVertexNew(AG_Graph *, void *);
void		AG_GraphVertexFree(AG_GraphVertex *);
AG_GraphVertex *AG_GraphVertexFind(AG_Graph *, void *);
void		AG_GraphVertexLabel(AG_GraphVertex *, const char *, ...);
void		AG_GraphVertexColorLabel(AG_GraphVertex *, Uint8, Uint8, Uint8);
void		AG_GraphVertexColorBG(AG_GraphVertex *, Uint8, Uint8, Uint8);
void		AG_GraphVertexPosition(AG_GraphVertex *, int, int);
void		AG_GraphVertexSize(AG_GraphVertex *, Uint, Uint);
void		AG_GraphVertexStyle(AG_GraphVertex *,
		                    enum ag_graph_vertex_style);
void		AG_GraphVertexPopupMenu(AG_GraphVertex *,
		                        struct ag_popup_menu *);

AG_GraphEdge	*AG_GraphEdgeNew(AG_Graph *, AG_GraphVertex *,
		                 AG_GraphVertex *, void *);
void		 AG_GraphEdgeFree(AG_GraphEdge *);
AG_GraphEdge	*AG_GraphEdgeFind(AG_Graph *, void *);
void		 AG_GraphEdgeLabel(AG_GraphEdge *, const char *, ...);
void		 AG_GraphEdgeColorLabel(AG_GraphEdge *, Uint8, Uint8, Uint8);
void		 AG_GraphEdgeColor(AG_GraphEdge *, Uint8, Uint8, Uint8);
void		 AG_GraphEdgePopupMenu(AG_GraphEdge *, struct ag_popup_menu *);

void		 AG_GraphAutoPlace(AG_Graph *, Uint, Uint);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_GUI_GRAPH_H_ */
