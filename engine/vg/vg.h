/*	$Csoft: vg.h,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/
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

#include <engine/vg/vg_vertex.h>
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
		struct object *sobj;
		Uint32 soffs;
	} pat;
	Uint32 color;
};

struct vg_element {
	enum vg_element_type type;
	double *vertices;
	int    nvertices;
	Uint32 color;
	struct vg_line_style line;
	struct vg_fill_style fill;
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
#define VG_ANTIALIAS	0x01
#define VG_HWSURFACE	0x02

	pthread_mutex_t lock;
	double w, h, scale;		/* Geometry in tiles */
	double ox, oy;			/* Origin offset */
	Uint32 fill_color;		/* Background filling color */
	struct object *pobj;		/* Parent object */
	SDL_Surface *su;		/* Surface for rasterization */
	struct map *submap;		/* Pointer to the fragment submap */
	struct map *map;		/* Raster map */
	TAILQ_HEAD(,vg_element) vges;	/* Graphic elements */
};

#define VG_X(vg,v) ((int)((v)*(vg)->scale*TILESZ) + \
                     (int)((vg)->ox*(vg)->scale*TILESZ))
#define VG_Y(vg,v) ((int)((v)*(vg)->scale*TILESZ) + \
                     (int)((vg)->oy*(vg)->scale*TILESZ))
#define VG_PX(vg,v) ((int)((v)*(vg)->scale*TILESZ))

__BEGIN_DECLS
struct vg	*vg_new(void *, int);
void		 vg_destroy(struct vg *);
void		 vg_scale(struct vg *, double, double, double);
__inline__ void	 vg_mvtail(struct vg *, struct vg_element *);
__inline__ void	 vg_mvhead(struct vg *, struct vg_element *);
__inline__ void	 vg_regen_fragments(struct vg *);
__inline__ void	 vg_destroy_fragments(struct vg *);
#ifdef DEBUG
__inline__ int	 vg_x(struct vg *, double);
__inline__ int	 vg_y(struct vg *, double);
#endif

__inline__ void	   vg_origin(struct vg *, double, double);
struct vg_element *vg_begin(struct vg *, enum vg_element_type);
#define		   vg_end(vg) /* nothing */

__inline__ void	vg_clear(struct vg *);
__inline__ void	vg_rasterize(struct vg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
