/*	Public domain	*/

#ifndef _AGAR_GUI_GRAPH_H_
#define _AGAR_GUI_GRAPH_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>

#include <agar/gui/begin.h>

#ifndef AG_GRAPH_LABEL_MAX
#define AG_GRAPH_LABEL_MAX AG_MODEL
#endif

struct ag_graph_edge;
struct ag_popup_menu;

enum ag_graph_vertex_style {	/* Vertex style */
	AG_GRAPH_RECTANGLE,	/* Rectangular box */
	AG_GRAPH_CIRCLE		/* Circle */
};

enum ag_graph_edge_type {
	AG_GRAPH_EDGE_UNDIRECTED,
	AG_GRAPH_EDGE_DIRECTED
};

typedef struct ag_graph_vertex {
	char labelTxt[AG_GRAPH_LABEL_MAX]; /* Label text */
	int  labelSu;                      /* Text surface handle */
	AG_Color labelColor;               /* Text color */
	AG_Color bgColor;                  /* Background color */
	enum ag_graph_vertex_style style;  /* Vertex style */

	Uint flags;
#define AG_GRAPH_MOUSEOVER	0x01	/* Mouse is over vertex */
#define AG_GRAPH_SELECTED	0x02	/* Vertex is selected */
#define AG_GRAPH_HIDDEN		0x04	/* Vertex in hidden */
#define AG_GRAPH_AUTOPLACED	0x08	/* Vertex has been auto-placed */

	int x, y;					 /* Coordinates */
	Uint w, h;					 /* Bounding box geometry */
	Uint                                     nEdges;
	struct ag_graph_edge *_Nullable *_Nonnull edges; /* Back pointers to edges */
	void *_Nullable userPtr;			 /* User pointer */
	struct ag_graph *_Nonnull graph;		 /* Parent graph */
	AG_TAILQ_ENTRY(ag_graph_vertex) vertices;
	AG_TAILQ_ENTRY(ag_graph_vertex) sorted;		 /* For autoplacer */
	struct ag_popup_menu *_Nullable popupMenu;	 /* Vertex popup menu */
} AG_GraphVertex;

typedef struct ag_graph_edge {
	enum ag_graph_edge_type type;		/* Edge type */
	char labelTxt[AG_GRAPH_LABEL_MAX];	/* Label text */
	int  labelSu;				/* Text surface handle */
	AG_Color edgeColor;			/* Edge color */
	AG_Color labelColor;			/* Label color */
	Uint flags;
/*#define AG_GRAPH_MOUSEOVER	0x01 */
/*#define AG_GRAPH_SELECTED	0x02 */
/*#define AG_GRAPH_HIDDEN	0x04 */
/*#define AG_GRAPH_AUTOPLACED	0x08 */
	Uint32 _pad;
	AG_GraphVertex *_Nonnull v1, *_Nonnull v2;   /* Connected vertices */
	void *_Nullable userPtr;                     /* User pointer */
	struct ag_graph *_Nonnull graph;             /* Back pointer to graph */
	AG_TAILQ_ENTRY(ag_graph_edge) edges;
	struct ag_popup_menu *_Nullable popupMenu;   /* Edge popup menu */
} AG_GraphEdge;

typedef struct ag_graph {
	struct ag_widget wid;		/* AG_Widget -> AG_Graph */
	Uint flags;
#define AG_GRAPH_HFILL     0x01
#define AG_GRAPH_VFILL     0x02
#define AG_GRAPH_EXPAND    (AG_GRAPH_HFILL | AG_GRAPH_VFILL)
#define AG_GRAPH_SCROLL	   0x04
#define AG_GRAPH_DRAGGING  0x08    /* Vertex is being moved (readonly) */
#define AG_GRAPH_PANNING   0x10    /* View is being panned (readonly) */
#define AG_GRAPH_NO_MOVE   0x20    /* User cannot move vertices */
#define AG_GRAPH_NO_SELECT 0x40    /* User cannot select vertices */
#define AG_GRAPH_NO_MENUS  0x80    /* Disable popup menus */
#define AG_GRAPH_READONLY  (AG_GRAPH_NO_MOVE | AG_GRAPH_NO_SELECT | \
                            AG_GRAPH_NO_MENUS)
	int wPre, hPre;                           /* Requested geometry */
	int xOffs, yOffs;                         /* Display offset */
	int xMin, xMax, yMin, yMax;               /* Display boundaries */
	Uint32 _pad;
	AG_TAILQ_HEAD_(ag_graph_vertex) vertices; /* Graph vertices */
	AG_TAILQ_HEAD_(ag_graph_edge) edges;      /* Graph edges */
	Uint nVertices, nEdges;	

	int pxMin, pxMax, pyMin, pyMax;	 /* Last cluster bounds (for autoplacer) */
	AG_Rect r;			 /* Display area */
} AG_Graph;

#define AGGRAPH(obj)            ((AG_Graph *)(obj))
#define AGCGRAPH(obj)           ((const AG_Graph *)(obj))
#define AG_GRAPH_SELF()          AGGRAPH( AG_OBJECT(0,"AG_Widget:AG_Graph:*") )
#define AG_GRAPH_PTR(n)          AGGRAPH( AG_OBJECT((n),"AG_Widget:AG_Graph:*") )
#define AG_GRAPH_NAMED(n)        AGGRAPH( AG_OBJECT_NAMED((n),"AG_Widget:AG_Graph:*") )
#define AG_CONST_GRAPH_SELF()   AGCGRAPH( AG_CONST_OBJECT(0,"AG_Widget:AG_Graph:*") )
#define AG_CONST_GRAPH_PTR(n)   AGCGRAPH( AG_CONST_OBJECT((n),"AG_Widget:AG_Graph:*") )
#define AG_CONST_GRAPH_NAMED(n) AGCGRAPH( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Graph:*") )

__BEGIN_DECLS
extern AG_WidgetClass agGraphClass;

AG_Graph *_Nonnull AG_GraphNew(void *_Nonnull, Uint);
void AG_GraphFreeVertices(AG_Graph *_Nonnull);
void AG_GraphSizeHint(AG_Graph *_Nonnull, Uint,Uint);
 
AG_GraphVertex *_Nonnull  AG_GraphVertexNew(AG_Graph *_Nonnull, void *_Nullable);
AG_GraphVertex *_Nullable AG_GraphVertexFind(AG_Graph *_Nonnull, void *_Nullable)
                                            _Pure_Attribute;

void AG_GraphVertexLabelS(AG_GraphVertex *_Nonnull, const char *_Nonnull);
void AG_GraphVertexLabel(AG_GraphVertex *_Nonnull, const char *_Nonnull, ...)
                        FORMAT_ATTRIBUTE(printf,2,3);
void AG_GraphVertexColorLabel(AG_GraphVertex *_Nonnull, Uint8,Uint8,Uint8);
void AG_GraphVertexColorBG(AG_GraphVertex *_Nonnull, Uint8,Uint8,Uint8);
void AG_GraphVertexPosition(AG_GraphVertex *_Nonnull, int,int);
void AG_GraphVertexSize(AG_GraphVertex *_Nonnull, Uint,Uint);
void AG_GraphVertexStyle(AG_GraphVertex *_Nonnull, enum ag_graph_vertex_style);
void AG_GraphVertexPopupMenu(AG_GraphVertex *_Nonnull,
                             struct ag_popup_menu *_Nullable);
void AG_GraphVertexFree(AG_GraphVertex *_Nonnull);

AG_GraphEdge *_Nullable AG_GraphEdgeNew(AG_Graph *_Nonnull,
					AG_GraphVertex *_Nonnull,
					AG_GraphVertex *_Nonnull,
					void *_Nullable);

AG_GraphEdge *_Nullable AG_DirectedGraphEdgeNew(AG_Graph *_Nonnull,
					AG_GraphVertex *_Nonnull,
					AG_GraphVertex *_Nonnull,
					void *_Nullable);

AG_GraphEdge *_Nullable AG_GraphEdgeFind(AG_Graph *_Nonnull, void *_Nullable)
					_Pure_Attribute;

void AG_GraphEdgeFree(AG_GraphEdge *_Nonnull);

void AG_GraphEdgeLabelS(AG_GraphEdge *_Nonnull, const char *_Nonnull);
void AG_GraphEdgeLabel(AG_GraphEdge *_Nonnull, const char *_Nonnull, ...)
                      FORMAT_ATTRIBUTE(printf,2,3);

void AG_GraphEdgeColorLabel(AG_GraphEdge *_Nonnull, Uint8,Uint8,Uint8);
void AG_GraphEdgeColor(AG_GraphEdge *_Nonnull, Uint8,Uint8,Uint8);
void AG_GraphEdgePopupMenu(AG_GraphEdge *_Nonnull, struct ag_popup_menu *_Nullable);

void AG_GraphAutoPlace(AG_Graph *_Nonnull, Uint,Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GRAPH_H_ */
