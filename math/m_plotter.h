/*	Public domain	*/

#ifndef _AGAR_MATH_M_PLOTTER_H_
#define _AGAR_MATH_M_PLOTTER_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>

#include <agar/math/begin.h>

#define M_PLOTTER_NDEFCOLORS	16
#define M_PLOTTER_LABEL_MAX	64

struct m_plotter;

enum m_plot_label_type {
	M_LABEL_X,				/* Refers to an x value */
	M_LABEL_Y,				/* Refers to an y value */
	M_LABEL_FREE,				/* Free placement in graph */
	M_LABEL_OVERLAY,			/* Overlay on widget */
};

typedef struct m_plot_label {
	char text[M_PLOTTER_LABEL_MAX];	/* Label text */
	int  text_surface;			/* Text surface handle */
	enum m_plot_label_type type;		/* Packing alignment */
	Uint x;					/* X position (or -1) */
	Uint y;					/* Y position (or -1) */
	AG_TAILQ_ENTRY(m_plot_label) labels;
} M_PlotLabel;

enum m_plot_type {
	M_PLOT_POINTS,		/* Individual points */
	M_PLOT_LINEAR,		/* Linear interpolation */
	M_PLOT_CUBIC_SPLINE,	/* Cubic spline interpolation */
	M_PLOT_VECTORS		/* Vector arrows/cones */
};

enum m_plot_source {
	M_PLOT_MANUALLY,		/* Don't update automatically */
	M_PLOT_FROM_VARIABLE_VFS,	/* Object variable (VFS path) */
	M_PLOT_FROM_REAL,		/* M_Real variable */
	M_PLOT_FROM_INT,		/* Integer variable */
	M_PLOT_FROM_COMPONENT,		/* Single vector/matrix component */
	M_PLOT_DERIVATIVE		/* Derivative of other plot */
};

typedef struct m_plot {
	enum m_plot_type type;
	enum m_plot_source src_type;
	union {
		struct {
			void *vfs;	 /* VFS root object */
			const char *key; /* Property path */
		} varVFS;
		M_Real *real;		/* Pointer to real value */
		int *integer;		/* Pointer to integer value */
		struct {
			M_Matrix *A;	/* Matrix/vector */
			Uint i, j;	/* Entry position */
		} com;
		struct m_plot *plot;	/* Other plot */
	} src;
	union {
		M_Real	*r;		/* Real points */
		M_Vector **v;		/* Real vectors */
	} data;
	struct m_plotter *plotter;	/* Back pointer to plotter */
	Uint n;				/* Number of points */
	Uint flags;
#define M_PLOT_SELECTED	0x01
#define M_PLOT_MOUSEOVER	0x02
#define M_PLOT_DRAGGING	0x04
#define M_PLOT_HIDDEN		0x08

	char label_txt[32];		/* Label text */
	int label;			/* Label surface handle */
	AG_Color color;			/* Plot color */
	M_Real xScale, yScale;		/* Scaling factors */
	int xOffs, yOffs;		/* Offset in display */
	int xLabel, yLabel;		/* Item position */
	AG_TAILQ_HEAD_(m_plot_label) labels; /* User labels */
	AG_TAILQ_ENTRY(m_plot) plots;
} M_Plot;

enum m_plotter_type {
	M_PLOT_2D,		/* 2D Cartesian */
	M_PLOT_POLAR,		/* 2D Polar */
	M_PLOT_SMITH,		/* Smith Chart */
	M_PLOT_3D,		/* 3D Cartesian */
	M_PLOT_SPHERICAL	/* 3D Spherical */
};

typedef struct m_plotter {
	struct ag_widget wid;
	enum m_plotter_type type;
	Uint flags;
#define M_PLOTTER_HFILL	0x01
#define M_PLOTTER_VFILL	0x02
#define M_PLOTTER_EXPAND	(M_PLOTTER_HFILL|M_PLOTTER_VFILL)
#define M_PLOTTER_SCROLL	0x04
	int xMax;			/* Maximum X for single value plots */
	M_Real yMin, yMax;		/* Extrema for single value plots */
	M_Vector *vMin, *vMax;		/* Extrema for vector plots */
	int xOffs, yOffs;		/* Display offset */
	int wPre, hPre;			/* SizeHint dimensions */
	M_Real xScale, yScale;		/* Scaling factors */
	AG_Font *font;			/* Default font face (or NULL) */
	AG_Color colors[M_PLOTTER_NDEFCOLORS];	/* Default plot color */
	int curColor;				/* Current default color */
	AG_Scrollbar *hbar, *vbar;	/* Display scrollbars */
	AG_Rect r;			/* View area */
	AG_TAILQ_HEAD_(m_plot) plots;	/* Plots in this view */
} M_Plotter;

__BEGIN_DECLS
extern AG_WidgetClass mPlotterClass;

M_Plotter *M_PlotterNew(void *, Uint);
void       M_PlotterSizeHint(M_Plotter *, Uint, Uint);
void       M_PlotterUpdate(M_Plotter *);
void       M_PlotterSetDefaultFont(M_Plotter *, const char *, int);
void       M_PlotterSetDefaultColor(M_Plotter *, int, Uint8, Uint8, Uint8);
void       M_PlotterSetDefaultScale(M_Plotter *, M_Real, M_Real);

M_Plot      *M_PlotNew(M_Plotter *, enum m_plot_type);
M_PlotLabel *M_PlotLabelNew(M_Plot *, enum m_plot_label_type, Uint, Uint,
                            const char *, ...);
void         M_PlotLabelSetText(M_Plot *, M_PlotLabel *, const char *, ...);
M_PlotLabel *M_PlotLabelReplace(M_Plot *, enum m_plot_label_type, Uint, Uint,
                                const char *, ...);

void	 M_PlotClear(M_Plot *);
M_Plot	*M_PlotFromReal(M_Plotter *, enum m_plot_type, const char *, M_Real *);
M_Plot	*M_PlotFromInt(M_Plotter *, enum m_plot_type, const char *, int *);
M_Plot	*M_PlotFromDerivative(M_Plotter *, enum m_plot_type, M_Plot *);
M_Plot	*M_PlotFromVariableVFS(M_Plotter *, enum m_plot_type, const char *,
                               void *, const char *);

struct ag_window *M_PlotSettings(M_Plot *);

void	M_PlotSetColor(M_Plot *, Uint8, Uint8, Uint8);
void 	M_PlotSetLabel(M_Plot *, const char *, ...);
void 	M_PlotUpdateLabel(M_Plot *);
void	M_PlotSetScale(M_Plot *, M_Real, M_Real);
void	M_PlotSetXoffs(M_Plot *, int);
void	M_PlotSetYoffs(M_Plot *, int);

void	M_PlotReal(M_Plot *, M_Real);
void	M_PlotRealv(M_Plot *, Uint, const M_Real *);
void	M_PlotVector(M_Plot *, const M_Vector *);
void	M_PlotVectorv(M_Plot *, Uint, const M_Vector **);
__END_DECLS

#include <agar/math/close.h>
#endif /* _AGAR_MATH_M_PLOTTER_H_ */
