/*	$Csoft: widget.h,v 1.73 2003/11/15 03:53:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include <engine/map.h>

#include "begin_code.h"

#define VG_TEXT_MAX	256

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

struct vg_element {
	enum vg_element_type type;	/* Type of element being drawn */
	float *vertices;
	int nvertices;
	Uint32 color;
	struct {
		enum {
			VG_CONTINUOUS,
			VG_STIPPLED
		};
		Uint16 stipple_pat;
		Uint8 thickness;
	} line;
	union {
		struct {
			float radius;			/* Inches */
		} vg_circle;
		struct {
			char text[VG_TEXT_MAX];
			float angle;			/* Degrees */
			enum vg_alignment align;	/* Text alignment */
		} vg_text;
	} vg_args;
#define vg_text vg_args.vg_text
	TAILQ_ENTRY(vg_element) rasq;
};

struct vg {
	float w, h, scale;		/* Requested geometry */
	int flags;
#define VG_ANTIALIAS	0x01
#define VG_HWSURFACE	0x02

	pthread_mutex_t lock;
	struct object *obj;		/* Object driving rendering */
	SDL_Surface *su;		/* Single rendered surface */
	Uint32 fill_color;		/* Background filling color */
	struct map *map;		/* Map of surface fragments */
	TAILQ_HEAD(,vg_element) rasq;	/* Elements queued for rasterization */
};

__BEGIN_DECLS
struct vg	*vg_new(void *, int, float, float, float, const char *);
void		 vg_destroy(struct vg *);
void		 vg_scale(struct vg *, float);

struct vg_element	*vg_begin(struct vg *, enum vg_element_type);
__inline__ void		 vg_vertex(struct vg *, float, float);
__inline__ void		 vg_vertices(struct vg *, float *, int);
__inline__ void		 vg_end(struct vg *);
__inline__ void		 vg_clear(struct vg *);
__inline__ void		 vg_render(struct vg *);

__inline__ double	 rad2deg(double);
__inline__ double	 deg2rad(double);

void	vg_draw_points(struct vg *, struct vg_element *);
void	vg_align(struct vg *, float, float, enum vg_alignment, float);
void	vg_printf(struct vg *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
