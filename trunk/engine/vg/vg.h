/*	$Csoft: vg.h,v 1.5 2004/04/12 03:38:02 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include <engine/map.h>

#include "begin_code.h"
struct vg;
struct vg_element;
enum vg_alignment {
	VG_ALIGN_TL,
	VG_ALIGN_TC,
	VG_ALIGN_TR,
	VG_ALIGN_ML,
	VG_ALIGN_MC,
	VG_ALIGN_MR,
	VG_ALIGN_BL,
	VG_ALIGN_BC,
	VG_ALIGN_BR
};
#include "close_code.h"

#include <engine/vg/vg_snap.h>
#include <engine/vg/vg_origin.h>
#include <engine/vg/vg_point.h>
#include <engine/vg/vg_line.h>
#include <engine/vg/vg_circle.h>
#include <engine/vg/vg_text.h>

#include "begin_code.h"

enum vg_element_type {
	VG_POINTS,		/* Individual points */
	VG_LINES,		/* Individual line segments */
	VG_LINE_STRIP,		/* Series of connected line segments */
	VG_LINE_LOOP,		/* Strip + line between first/last vertices */
	VG_TRIANGLES,		/* Individual triangles */
	VG_TRIANGLE_STRIP,	/* Series of connected triangles */
	VG_TRIANGLE_FAN,	/* Fan of connected triangles */
	VG_QUADS,		/* Individual four-sided polygons */
	VG_QUAD_STRIP,		/* Series of connected four-sided polygons */
	VG_POLYGON,		/* Simple, convex polygon */
	VG_CIRCLE,		/* Single circle */
	VG_ELLIPSE,		/* Single ellipse */
	VG_BEZIER_CURVE,	/* Bezier curve */
	VG_BEZIGON,		/* Bezigon */
	VG_TEXT			/* Text string */
};

struct vg_line_style {
	enum {
		VG_CONTINUOUS,
		VG_STIPPLED
	} style;
	Uint16 stipple;		/* OpenGL-style */
	Uint8 thickness;	/* Pixels */
};

struct vg_fill_style {
	enum {
		VG_NOFILL,
		VG_SOLID,
		VG_PATTERN
	} style;
	struct {
		struct object *gfx_obj;
		Uint32 gfx_offs;
	} pat;
	Uint32 color;
};

struct vg_vertex {
	double x, y, z, w;
};

struct vg_element {
	enum vg_element_type type;
	struct vg_vertex *vtx;				/* Vertices */
	unsigned int	 nvtx;
	Uint32 color;					/* Foreground color */
	struct vg_line_style line;			/* Line style */
	struct vg_fill_style fill;			/* Filling style */
	union {
		struct {
			double radius;
		} vg_point;
		struct {
			double radius;
		} vg_circle;
		struct {
			char text[VG_TEXT_MAX];
			double angle;
			enum vg_alignment align;
		} vg_text;
	} vg_args;
#define vg_point   vg_args.vg_point
#define vg_circle  vg_args.vg_circle
#define vg_text	   vg_args.vg_text
	TAILQ_ENTRY(vg_element) vges;
};

struct vg {
	int flags;
#define VG_ANTIALIAS	0x01		/* Anti-alias where possible */
#define VG_HWSURFACE	0x02		/* Prefer video memory for fragments */
#define VG_VISORIGIN	0x04		/* Display the origin points */
#define VG_VISGRID	0x08		/* Display the grid */

	pthread_mutex_t lock;
	double w, h;
	double scale;
	Uint32 fill_color;		/* Background color */
	Uint32 grid_color;		/* Grid color */
	double grid_gap;		/* Grid interval */

	struct vg_vertex *origin;		/* Origin point vertices */
	float		 *origin_radius;	/* Origin point radii */
	Uint32		 *origin_color;		/* Origin point colors */
	Uint32		 norigin;
	
	struct object *pobj;
	SDL_Surface *su;		/* Raster surface */
	struct map *submap;		/* Fragment map */
	struct map *map;		/* Raster map */
	enum vg_snap_mode snap_mode;	/* Positional restriction */

	TAILQ_HEAD(,vg_element) vges;
};

#define VG_PX(vg,v) ((int)((v)*(vg)->scale*TILESZ))

__BEGIN_DECLS
struct vg	*vg_new(void *, int);
void		 vg_init(struct vg *, int);
void		 vg_destroy(struct vg *);
void		 vg_scale(struct vg *, double, double, double);
void		 vg_clear(struct vg *);
void		 vg_rasterize(struct vg *);
__inline__ void	 vg_regen_fragments(struct vg *);
__inline__ void	 vg_destroy_fragments(struct vg *);
__inline__ void	 vg_vcoords2(struct vg *, int, int, int, int, double *,
                             double *);
__inline__ void	 vg_avcoords2(struct vg *, int, int, int, int, double *,
		              double *);
__inline__ void  vg_rcoords2(struct vg *, double, double, int *, int *);
__inline__ void	 vg_arcoords2(struct vg *, double, double, int *, int *);
__inline__ void  vg_rlength(struct vg *, double, int *);

struct vg_element *vg_begin(struct vg *, enum vg_element_type);

struct vg_vertex *vg_vertex2(struct vg *, double, double);
struct vg_vertex *vg_vertex3(struct vg *, double, double, double);
struct vg_vertex *vg_vertex4(struct vg *, double, double, double, double);
void		  vg_vertex_array(struct vg *, const struct vg_vertex *,
		                  unsigned int);
int		  vg_near_vertex2(struct vg *, const struct vg_vertex *,
		                  double, double, double);

void		  vg_undo_element(struct vg *, struct vg_element *);
void		  vg_pop_vertex(struct vg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
