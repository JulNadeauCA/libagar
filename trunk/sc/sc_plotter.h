/*	Public domain	*/

#ifndef _AGAR_SC_PLOTTER_H_
#define _AGAR_SC_PLOTTER_H_

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

#define SC_PLOTTER_NDEFCOLORS	16
#define SC_PLOTTER_LABEL_MAX	64

struct sc_plotter;

enum sc_plot_label_type {
	SC_LABEL_X,				/* Refers to an x value */
	SC_LABEL_Y,				/* Refers to an y value */
	SC_LABEL_FREE,				/* Free placement in graph */
	SC_LABEL_OVERLAY,			/* Overlay on widget */
};

typedef struct sc_plot_label {
	char text[SC_PLOTTER_LABEL_MAX];	/* Label text */
	int  text_surface;			/* Text surface handle */
	enum sc_plot_label_type type;		/* Packing alignment */
	Uint x;					/* X position (or -1) */
	Uint y;					/* Y position (or -1) */
	TAILQ_ENTRY(sc_plot_label) labels;
} SC_PlotLabel;

typedef struct sc_plot {
	enum sc_plot_type {
		SC_PLOT_POINTS,		/* Individual points */
		SC_PLOT_LINEAR,		/* Linear interpolation */
		SC_PLOT_CUBIC_SPLINE,	/* Cubic spline interpolation */
		SC_PLOT_VECTORS		/* Vector arrows/cones */
	} type;
	enum sc_plot_source {
		SC_PLOT_MANUALLY,	/* Don't update automatically */
		SC_PLOT_FROM_PROP,	/* Numerical object property */
		SC_PLOT_FROM_REAL,	/* SC_Real variable */
		SC_PLOT_FROM_INT,	/* Integer variable */
		SC_PLOT_FROM_COMPONENT,	/* Single vector/matrix component */
		SC_PLOT_DERIVATIVE	/* Derivative of other plot */
	} src_type;
	union {
		const char *prop;	/* Property path */
		SC_Real *real;		/* Pointer to real value */
		int *integer;		/* Pointer to integer value */
		struct {
			SC_Matrix *A;	/* Matrix/vector */
			Uint i, j;	/* Entry position */
		} com;
		struct sc_plot *plot;	/* Other plot */
	} src;
	union {
		SC_Real	*r;		/* Real points */
		SC_Vector **v;		/* Real vectors */
	} data;
	struct sc_plotter *plotter;	/* Back pointer to plotter */
	Uint n;				/* Number of points */
	Uint flags;
#define SC_PLOT_SELECTED	0x01
#define SC_PLOT_MOUSEOVER	0x02
#define SC_PLOT_DRAGGING	0x04
#define SC_PLOT_HIDDEN		0x08

	char label_txt[32];		/* Label text */
	int label;			/* Label surface handle */
	Uint32 color;			/* Plot color */
	SC_Real xScale, yScale;		/* Scaling factors */
	int xOffs, yOffs;		/* Offset in display */
	int xLabel, yLabel;		/* Item position */
	TAILQ_HEAD(,sc_plot_label) labels; /* User labels */
	TAILQ_ENTRY(sc_plot) plots;
} SC_Plot;

typedef struct sc_plotter {
	struct ag_widget wid;
	enum sc_plotter_type {
		SC_PLOT_2D,		/* 2D Cartesian */
		SC_PLOT_POLAR,		/* 2D Polar */
		SC_PLOT_SMITH,		/* Smith Chart */
		SC_PLOT_3D,		/* 3D Cartesian */
		SC_PLOT_SPHERICAL	/* 3D Spherical */
	} type;
	Uint flags;
#define SC_PLOTTER_HFILL	0x01
#define SC_PLOTTER_VFILL	0x02
#define SC_PLOTTER_EXPAND	(SC_PLOTTER_HFILL|SC_PLOTTER_VFILL)
#define SC_PLOTTER_SCROLL	0x04
	int xMax;			/* Maximum X for single value plots */
	SC_Real yMin, yMax;		/* Extrema for single value plots */
	SC_Vector *vMin, *vMax;		/* Extrema for vector plots */
	int xOffs, yOffs;		/* Display offset */
	int wPre, hPre;			/* Prescale dimensions */
	SC_Real xScale, yScale;		/* Scaling factors */
	const char *fontFace;		/* Default font face (or NULL) */
	int fontSize;			/* Default font size (or -1) */
	Uint32 colors[SC_PLOTTER_NDEFCOLORS];	/* Default plot color */
	int curColor;				/* Current default color */
	AG_Scrollbar *hbar, *vbar;	/* Display scrollbars */
	TAILQ_HEAD(,sc_plot) plots;	/* Plots in this view */
} SC_Plotter;

__BEGIN_DECLS
SC_Plotter	*SC_PlotterNew(void *, Uint);
void	 	 SC_PlotterInit(SC_Plotter *, Uint);
void	 	 SC_PlotterScale(void *, int, int);
void	 	 SC_PlotterPrescale(SC_Plotter *, Uint, Uint);
void		 SC_PlotterDraw(void *);
void		 SC_PlotterUpdate(SC_Plotter *);
__inline__ void	 SC_PlotterSetDefaultFont(SC_Plotter *, const char *, int);
__inline__ void	 SC_PlotterSetDefaultColor(SC_Plotter *, int, Uint8, Uint8,
		                           Uint8);
__inline__ void	 SC_PlotterSetDefaultScale(SC_Plotter *, SC_Real, SC_Real);

SC_Plot		*SC_PlotNew(SC_Plotter *, enum sc_plot_type);

SC_PlotLabel	*SC_PlotLabelNew(SC_Plot *, enum sc_plot_label_type,
		                 Uint, Uint, const char *, ...);
void		 SC_PlotLabelSetText(SC_Plot *, SC_PlotLabel *, const char *,
		                     ...);
SC_PlotLabel	*SC_PlotLabelReplace(SC_Plot *, enum sc_plot_label_type,
		                     Uint, Uint, const char *, ...);

void		 SC_PlotClear(SC_Plot *);
SC_Plot		*SC_PlotFromProp(SC_Plotter *, enum sc_plot_type, const char *,
		                 const char *);
SC_Plot		*SC_PlotFromReal(SC_Plotter *, enum sc_plot_type, const char *,
		                 SC_Real *);
SC_Plot		*SC_PlotFromInt(SC_Plotter *, enum sc_plot_type, const char *,
		                int *);
SC_Plot		*SC_PlotFromDerivative(SC_Plotter *, enum sc_plot_type,
		                       SC_Plot *);

AG_Window	*SC_PlotSettings(SC_Plot *);
__inline__ void	 SC_PlotSetColor(SC_Plot *, Uint8, Uint8, Uint8);
void	 	 SC_PlotSetLabel(SC_Plot *, const char *, ...);
void	 	 SC_PlotUpdateLabel(SC_Plot *);
__inline__ void	 SC_PlotSetScale(SC_Plot *, SC_Real, SC_Real);
__inline__ void	 SC_PlotSetXoffs(SC_Plot *, int);
__inline__ void	 SC_PlotSetYoffs(SC_Plot *, int);
__inline__ void	 SC_PlotReal(SC_Plot *, SC_Real);
void	 	 SC_PlotRealv(SC_Plot *, Uint, const SC_Real *);
__inline__ void	 SC_PlotVector(SC_Plot *, const SC_Vector *);
void		 SC_PlotVectorv(SC_Plot *, Uint, const SC_Vector **);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SC_PLOTTER_H_ */
