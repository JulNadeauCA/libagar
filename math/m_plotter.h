/*	Public domain	*/

#ifndef _AGAR_MATH_M_PLOTTER_H_
#define _AGAR_MATH_M_PLOTTER_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>
#include <agar/gui/scrollbar.h>

#include <agar/math/begin.h>

#define M_PLOTTER_NDEFCOLORS	16
#define M_PLOTTER_LABEL_MAX	AG_MODEL

struct m_plotter;

enum m_plot_label_type {
	M_LABEL_X,			/* Refers to an x value */
	M_LABEL_Y,			/* Refers to an y value */
	M_LABEL_FREE,			/* Free placement in graph */
	M_LABEL_OVERLAY,		/* Overlay on widget */
};

typedef struct m_plot_label {
	char text[M_PLOTTER_LABEL_MAX];		/* Label text */
	int  text_surface;			/* Text surface handle */
	enum m_plot_label_type type;		/* Packing alignment */
	Uint x, y;				/* Display position */
	AG_TAILQ_ENTRY(m_plot_label) labels;
} M_PlotLabel;

enum m_plot_type {
	M_PLOT_POINTS,			/* Individual points */
	M_PLOT_LINEAR,			/* Linear interpolation */
	M_PLOT_CUBIC_SPLINE,		/* Cubic spline interpolation */
	M_PLOT_VECTORS			/* Vector arrows/cones */
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
			void       *_Nonnull vfs;	/* VFS root object */
			const char *_Nonnull key;	/* Property path */
		} varVFS;
		M_Real *_Nonnull real;			/* Real values */
		int    *_Nonnull integer;		/* Integer values */
		struct {
			M_Matrix *_Nonnull A;		/* Matrix/vector */
			Uint i, j;			/* Entry position */
		} com;
		struct m_plot *_Nonnull plot;		/* Other plot */
	} src;
	union {
		M_Real	*_Nullable            r;	/* Real points */
		M_Vector *_Nullable *_Nonnull v;	/* Real vectors */
	} data;
	struct m_plotter *_Nonnull plotter;	/* Back pointer to plotter */
	Uint n;					/* Number of points */
	Uint flags;
#define M_PLOT_SELECTED	  0x01			/* Selected by user */
#define M_PLOT_MOUSEOVER  0x02			/* Mouse hover */
#define M_PLOT_DRAGGING	  0x04			/* Being moved */
#define M_PLOT_HIDDEN	  0x08			/* Hidden */

	char label_txt[32];			/* Label text */
	int label;				/* Label surface handle */
	AG_Color color;				/* Plot color */
#if AG_MODEL == AG_LARGE
	Uint32 _pad;
#endif
	M_Real xScale, yScale;			/* Scaling factors */
	int xOffs, yOffs;			/* Offset in display */
	int xLabel, yLabel;			/* Item position */
	AG_TAILQ_HEAD_(m_plot_label) labels;	/* User labels */
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
	struct ag_widget wid;		/* AG_Widget(3) -> M_Plotter */
	enum m_plotter_type type;	/* Type of plot */
	Uint flags;
#define M_PLOTTER_SCROLL	0x0001
#define M_PLOTTER_HFILL		0x4000
#define M_PLOTTER_VFILL		0x8000
#define M_PLOTTER_EXPAND	(M_PLOTTER_HFILL|M_PLOTTER_VFILL)
	int xMax;			/* Maximum X for single value plots */
	int   curColor;				/* Current default color */
	M_Real yMin, yMax;		/* Extrema for single value plots */
	M_Vector *_Nonnull vMin;	/* Extrema for vector plots */
	M_Vector *_Nonnull vMax;	/* Extrema for vector plots */
	int xOffs, yOffs;		/* Display offset */
	int wPre, hPre;			/* SizeHint dimensions */
	M_Real xScale, yScale;		/* Scaling factors */

	AG_Font *_Nonnull font;			/* Default font */
	AG_Color colors[M_PLOTTER_NDEFCOLORS];	/* Default plot color */
	AG_Scrollbar *_Nonnull hbar;	/* Horizontal scrollbar */
	AG_Scrollbar *_Nonnull vbar;	/* Vertical scrollbar */
	AG_Rect r;			/* View area */
	AG_TAILQ_HEAD_(m_plot) plots;	/* Plots in this view */
} M_Plotter;

#define MPLOTTER(obj)           ((M_Plotter *)(obj))
#define MCPLOTTER(obj)          ((const M_Plotter *)(obj))
#define M_PLOTTER_SELF()          MPLOTTER( AG_OBJECT(0,"AG_Widget:M_Plotter:*") )
#define M_PLOTTER_PTR(n)          MPLOTTER( AG_OBJECT((n),"AG_Widget:M_Plotter:*") )
#define M_PLOTTER_NAMED(n)        MPLOTTER( AG_OBJECT_NAMED((n),"AG_Widget:M_Plotter:*") )
#define M_CONST_PLOTTER_SELF()   MCPLOTTER( AG_CONST_OBJECT(0,"AG_Widget:M_Plotter:*") )
#define M_CONST_PLOTTER_PTR(n)   MCPLOTTER( AG_CONST_OBJECT((n),"AG_Widget:M_Plotter:*") )
#define M_CONST_PLOTTER_NAMED(n) MCPLOTTER( AG_CONST_OBJECT_NAMED((n),"AG_Widget:M_Plotter:*") )

__BEGIN_DECLS
extern AG_WidgetClass mPlotterClass;

M_Plotter *_Nonnull M_PlotterNew(void *_Nullable, Uint);

void M_PlotterSizeHint(M_Plotter *_Nonnull, Uint,Uint);
void M_PlotterUpdate(M_Plotter *_Nonnull);
void M_PlotterSetDefaultFont(M_Plotter *_Nonnull, const char *_Nullable, float);
void M_PlotterSetDefaultColor(M_Plotter *_Nonnull, int, Uint8,Uint8,Uint8);
void M_PlotterSetDefaultScale(M_Plotter *_Nonnull, M_Real,M_Real);

M_Plot *_Nonnull      M_PlotNew(M_Plotter *_Nonnull, enum m_plot_type);

M_PlotLabel *_Nonnull M_PlotLabelNew(M_Plot *_Nonnull, enum m_plot_label_type,
                                     Uint,Uint, const char *_Nonnull, ...);
void                  M_PlotLabelSetText(M_Plot *_Nonnull,
                                         M_PlotLabel *_Nonnull,
					 const char *_Nonnull, ...)
                                        FORMAT_ATTRIBUTE(printf,3,4);
M_PlotLabel *_Nonnull M_PlotLabelReplace(M_Plot *_Nonnull,
                                         enum m_plot_label_type, Uint,Uint,
					 const char *_Nonnull, ...)
                                        FORMAT_ATTRIBUTE(printf,5,6);

void M_PlotClear(M_Plot *_Nonnull);

M_Plot *_Nonnull M_PlotFromReal(M_Plotter *_Nonnull, enum m_plot_type,
                                const char *_Nonnull, M_Real *_Nonnull);
M_Plot *_Nonnull M_PlotFromInt(M_Plotter *_Nonnull, enum m_plot_type,
                               const char *_Nonnull, int *_Nonnull);
M_Plot *_Nonnull M_PlotFromDerivative(M_Plotter *_Nonnull, enum m_plot_type,
                                      M_Plot *_Nonnull);
M_Plot *_Nonnull M_PlotFromVariableVFS(M_Plotter *_Nonnull, enum m_plot_type,
                                       const char *_Nonnull, void *_Nonnull,
				       const char *_Nonnull);

struct ag_window *_Nullable M_PlotSettings(M_Plot *_Nonnull);

void	M_PlotSetColor(M_Plot *_Nonnull, Uint8,Uint8,Uint8);
void 	M_PlotSetLabel(M_Plot *_Nonnull, const char *_Nonnull, ...)
                      FORMAT_ATTRIBUTE(printf,2,3);
void 	M_PlotUpdateLabel(M_Plot *_Nonnull);
void	M_PlotSetScale(M_Plot *_Nonnull, M_Real,M_Real);
void	M_PlotSetXoffs(M_Plot *_Nonnull, int);
void	M_PlotSetYoffs(M_Plot *_Nonnull, int);

void	M_PlotReal(M_Plot *_Nonnull, M_Real);
void	M_PlotRealv(M_Plot *_Nonnull, Uint, const M_Real *_Nonnull);
void	M_PlotVector(M_Plot *_Nonnull, const M_Vector *_Nonnull);
void	M_PlotVectorv(M_Plot *_Nonnull, Uint, const M_Vector *_Nonnull *_Nonnull);
__END_DECLS

#include <agar/math/close.h>
#endif /* _AGAR_MATH_M_PLOTTER_H_ */
