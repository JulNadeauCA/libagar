/*	$Csoft: vg.h,v 1.14 2004/05/06 08:47:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include <engine/map.h>

#include "begin_code.h"

#define VG_NAME_MAX		128
#define VG_LAYER_NAME_MAX	128

/*
 * T = top	L = left
 * M = middle	C = center
 * B = bottom	R = right
 */
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
struct vg_vertex {
	double x, y, z, w;
};
struct vg_rect {
	double x, y;
	double w, h;
};

struct vg;
struct vg_element;

#include "close_code.h"

#include <engine/vg/vg_snap.h>
#include <engine/vg/vg_ortho.h>
#include <engine/vg/vg_origin.h>
#include <engine/vg/vg_block.h>

#include <engine/vg/vg_point.h>
#include <engine/vg/vg_line.h>
#include <engine/vg/vg_circle.h>
#include <engine/vg/vg_ellipse.h>
#include <engine/vg/vg_text.h>
#include <engine/vg/vg_mask.h>

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
	VG_ARC,			/* Single arc */
	VG_ELLIPSE,		/* Single ellipse */
	VG_BEZIER_CURVE,	/* Bezier curve */
	VG_BEZIGON,		/* Bezigon */
	VG_TEXT,		/* Text string */
	VG_MASK,		/* Polygonal mask */
	VG_LAST
};

struct vg_element_ops {
	const char *name;
	void (*init)(struct vg *, struct vg_element *);
	void (*destroy)(struct vg *, struct vg_element *);
	void (*draw)(struct vg *, struct vg_element *);
	void (*bbox)(struct vg *, struct vg_element *, struct vg_rect *);
};

struct vg_line_style {
	enum {
		VG_CONTINUOUS,
		VG_STIPPLED
	} style;
	enum {
		VG_SQUARE,		/* Square endpoint */
		VG_BEVELED,		/* Beveled endpoint */
		VG_ROUNDED,		/* Rounded endpoint (circular) */
		VG_MITERED		/* Mitered endpoint */
	} endpoint_style;
	Uint16 stipple;			/* OpenGL-style stipple pattern */
	Uint8 thickness;		/* Pixels */
	Uint8 miter_len;		/* Miter length for VG_MITERED */
};

struct vg_fill_style {
	enum {
		VG_NOFILL,		/* Wireframe */
		VG_SOLID,		/* Solid filling */
		VG_TEXTURED		/* Textured */
	} style;
	struct {
		struct object *gfx_obj;	/* Pixmap object */
		Uint32 gfx_index;	/* Pixmap index */
	} tex;
	Uint32 color;			/* Color for VG_SOLID */
};

struct vg_layer {
	char name[VG_LAYER_NAME_MAX];	/* Layer name */
	int visible;			/* Flag of visibility */
	Uint32 color;			/* Per-layer default color */
	Uint8 alpha;			/* Per-layer alpha value */
};

struct vg_element {
	enum vg_element_type type;		/* Class of element */
	const struct vg_element_ops *ops;	/* Generic element operations */
	struct vg_block *block;			/* Back pointer to block */
	int flags;
#define VG_ELEMENT_NOSAVE 0x01		/* Don't save with drawing */
	int layer;			/* Associated layer */
	int redraw;			/* Element redraw */
	int drawn;			/* Avoid overdraw */
	Uint32 color;			/* Element specific color */
	struct vg_line_style line;	/* Line style */
	struct vg_fill_style fill;	/* Polygon filling style */
	struct vg_vertex *vtx;		/* Vertices */
	Uint32		 nvtx;
	union {
		struct vg_circle_args vg_circle;
		struct vg_ellipse_args vg_arc;
		struct vg_text_args vg_text;
		struct vg_mask_args vg_mask;
	} vg_args;
#define vg_circle   vg_args.vg_circle
#define vg_arc	    vg_args.vg_arc
#define vg_text	    vg_args.vg_text
#define vg_mask	    vg_args.vg_mask
	TAILQ_ENTRY(vg_element) vgbmbs;	/* Entry in block element list */
	TAILQ_ENTRY(vg_element) vges;	/* Entry in global element list */
};

struct vg {
	char name[VG_NAME_MAX];		/* Name of drawing */
	int flags;
#define VG_ANTIALIAS	0x01		/* Anti-alias where possible */
#define VG_HWSURFACE	0x02		/* Prefer video memory for fragments */
#define VG_VISORIGIN	0x04		/* Display the origin points */
#define VG_VISGRID	0x08		/* Display the grid */
#define VG_VISBBOXES	0x10		/* Display bounding boxes (debug) */

	pthread_mutex_t lock;
	int redraw;			/* Global redraw */
	int **mask;			/* Fragment mask */
	double w, h;			/* Bounding box */
	double scale;			/* Scaling factor */
	Uint32 fill_color;		/* Background color */
	Uint32 grid_color;		/* Grid color */
	double grid_gap;		/* Grid interval */

	struct vg_vertex *origin;		/* Origin point vertices */
	float		 *origin_radius;	/* Origin point radii */
	Uint32		 *origin_color;		/* Origin point colors */
	Uint32		 norigin;
	
	struct vg_layer *layers;		/* Layers */
	Uint32		nlayers;

	int		 cur_layer;		/* Layer selected for edition */
	struct vg_block	*cur_block;		/* Block being edited */
	enum vg_snap_mode snap_mode;		/* Positional restriction */
	enum vg_ortho_mode ortho_mode;		/* Orthogonal restriction */

	struct object *pobj;
	SDL_Surface *su;		/* Raster surface */
	struct map *submap;		/* Fragment map */
	struct map *map;		/* Raster map */

	TAILQ_HEAD(,vg_element) vges;		/* Elements in drawing */
	TAILQ_HEAD(,vg_block) blocks;		/* Blocks in drawing */
	TAILQ_HEAD(,vg_text_style) txtstyles;	/* Text styles */
};

extern int vg_cos_tbl[];
extern int vg_sin_tbl[];

__BEGIN_DECLS
struct vg	*vg_new(void *, int);
void		 vg_init(struct vg *, int);
void		 vg_destroy(struct vg *);
void		 vg_save(struct vg *, struct netbuf *);
int		 vg_load(struct vg *, struct netbuf *);

void		 vg_scale(struct vg *, double, double, double);
void		 vg_clear(struct vg *);
void		 vg_rasterize(struct vg *);
__inline__ void	 vg_redraw_elements(struct vg *);
__inline__ void	 vg_update_fragments(struct vg *);
__inline__ void	 vg_destroy_fragments(struct vg *);

__inline__ void	 vg_vcoords2(struct vg *, int, int, int, int, double *,
                             double *);
__inline__ void	 vg_avcoords2(struct vg *, int, int, int, int, double *,
		              double *);
__inline__ void  vg_rcoords2(struct vg *, double, double, int *, int *);
__inline__ void	 vg_arcoords2(struct vg *, double, double, int *, int *);
__inline__ void  vg_rlength(struct vg *, double, int *);
void		 vg_pop_vertex(struct vg *);

struct vg_layer *vg_push_layer(struct vg *, const char *);
__inline__ void	 vg_pop_layer(struct vg *);

struct vg_element *vg_begin_element(struct vg *, enum vg_element_type);
void		   vg_destroy_element(struct vg *, struct vg_element *);
__inline__ int	   vg_rcollision(struct vg *, struct vg_rect *,
		                 struct vg_rect *, struct vg_rect *);
 
__inline__ void	   vg_layer(struct vg *, int);
__inline__ void	   vg_color(struct vg *, Uint32);
__inline__ void	   vg_color3(struct vg *, int, int, int);
__inline__ void	   vg_color4(struct vg *, int, int, int, int);
struct vg_vertex  *vg_vertex2(struct vg *, double, double);
struct vg_vertex  *vg_vertex3(struct vg *, double, double, double);
struct vg_vertex  *vg_vertex4(struct vg *, double, double, double, double);
void		   vg_vertex_array(struct vg *, const struct vg_vertex *,
		                   unsigned int);

#ifdef EDITION
void		   vg_geo_changed(int, union evarg *);
void		   vg_changed(int, union evarg *);
struct combo	  *vg_layer_selector(void *, struct vg *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
