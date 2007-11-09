/*	$Csoft: vg.h,v 1.41 2005/09/27 00:25:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include "begin_code.h"

#define VG_NAME_MAX		128
#define VG_LAYER_NAME_MAX	128
#define VG_STYLE_NAME_MAX	16
#define VG_FONT_FACE_MAX	32
#define VG_FONT_STYLE_MAX	16
#define VG_FONT_SIZE_MIN	4
#define VG_FONT_SIZE_MAX	48

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

typedef struct vg_vertex {
	float x, y;
} VG_Vtx;

typedef struct vg_rect {
	float x, y;
	float w, h;
} VG_Rect;

struct vg;
struct vg_element;

#include "close_code.h"

#ifdef _AGAR_INTERNAL
#include <vg/vg_math.h>
#include <vg/vg_snap.h>
#include <vg/vg_ortho.h>
#include <vg/vg_origin.h>
#include <vg/vg_block.h>
#include <vg/vg_line.h>
#include <vg/vg_circle.h>
#include <vg/vg_arc.h>
#include <vg/vg_text.h>
#include <vg/vg_mask.h>
#include <vg/vg_polygon.h>
#else
#include <agar/vg/vg_math.h>
#include <agar/vg/vg_snap.h>
#include <agar/vg/vg_ortho.h>
#include <agar/vg/vg_origin.h>
#include <agar/vg/vg_block.h>
#include <agar/vg/vg_line.h>
#include <agar/vg/vg_circle.h>
#include <agar/vg/vg_arc.h>
#include <agar/vg/vg_text.h>
#include <agar/vg/vg_mask.h>
#include <agar/vg/vg_polygon.h>
#endif

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
	VG_POLYGON,		/* Polygon */
	VG_CIRCLE,		/* Single circle */
	VG_ARC,			/* Single arc */
	VG_ELLIPSE,		/* Single ellipse */
	VG_BEZIER_CURVE,	/* Bezier curve */
	VG_BEZIGON,		/* Bezigon */
	VG_TEXT,		/* Text string */
	VG_MASK,		/* Polygonal mask */
	VG_LAST
};

typedef struct vg_element_ops {
	const char *name;
	AG_StaticIcon *icon;
	void (*init)(struct vg *, struct vg_element *);
	void (*destroy)(struct vg *, struct vg_element *);
	void (*draw)(struct vg *, struct vg_element *);
	void (*bbox)(struct vg *, struct vg_element *, VG_Rect *);
	float (*intsect)(struct vg*, struct vg_element *, float *, float *);
} VG_ElementOps;

typedef struct vg_layer {
	char name[VG_LAYER_NAME_MAX];	/* Layer name */
	int visible;			/* Flag of visibility */
	Uint32 color;			/* Per-layer default color */
	Uint8 alpha;			/* Per-layer alpha value */
} VG_Layer;

typedef struct vg_line_style {
	enum {
		VG_CONTINUOUS,
		VG_STIPPLE,
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
} VG_LineStyle;

typedef struct vg_filling_style {
	enum {
		VG_NOFILL,		/* Wireframe */
		VG_SOLID,		/* Solid filling */
		VG_TEXTURED		/* Textured */
	} style;
	char texture[32];
	Uint8 texture_alpha;
} VG_FillingStyle;

typedef struct vg_text_style {
	char face[VG_FONT_FACE_MAX];
	int size;
	int flags;
#define VG_FONT_BOLD		0x01		/* Bold style */
#define VG_FONT_ITALIC		0x02		/* Italic style */
#define VG_FONT_UNDERLINE	0x04		/* Underlined */
#define VG_FONT_SCALED		0x08		/* Try to scale the text */
} VG_TextStyle;

enum vg_style_type {
	VG_LINE_STYLE,
	VG_FILL_STYLE,
	VG_TEXT_STYLE
};

typedef struct vg_style {
	char name[VG_STYLE_NAME_MAX];
	enum vg_style_type type;
	Uint32 color;
	union {
		VG_LineStyle vg_line_style;
		VG_TextStyle vg_text_style;
		VG_FillingStyle vg_fill_style;
	} vg_style_args;
#define vg_line_st	vg_style_args.vg_line_style
#define vg_text_st	vg_style_args.vg_text_style
#define vg_fill_st	vg_style_args.vg_fill_style
	TAILQ_ENTRY(vg_style) styles;
} VG_Style;

typedef struct vg_matrix {
	float m[4][4];
} VG_Matrix;

typedef struct vg_element {
	enum vg_element_type type;
	const VG_ElementOps *ops;
	int flags;
#define VG_ELEMENT_NOSAVE 0x01			/* Don't save with drawing */
#define VG_ELEMENT_IGNORE_BLK_COLOR 0x02	/* Ignore block color */

	VG_Block *block;		/* Back pointer to block */
	VG_Style *style;		/* Default element style */
	VG_LineStyle line_st;		/* Effective line style */
	VG_FillingStyle fill_st;	/* Effective filling style */
	VG_TextStyle text_st;		/* Effective text style */
	Uint32 color;			/* Effective foreground color */
	int layer;			/* Associated layer */
	int selected;			/* Multiple selection flag */
	int mouseover;			/* Mouse overlap flag */

	VG_Vtx	  *vtx;			/* Vertices */
	Uint	  nvtx;
	VG_Matrix *trans;		/* Transformation matrices */
	Uint     ntrans;

	union {
		struct vg_circle_args vg_circle;
		struct vg_arc_args vg_arc;
		struct vg_text_args vg_text;
		struct vg_mask_args vg_mask;
		struct vg_polygon_args vg_polygon;
	} vg_args;
#ifndef _AGAR_VG_PUBLIC
#define vg_circle   vg_args.vg_circle
#define vg_arc	    vg_args.vg_arc
#define vg_text	    vg_args.vg_text
#define vg_mask	    vg_args.vg_mask
#define vg_polygon  vg_args.vg_polygon
#endif
	TAILQ_ENTRY(vg_element) vgbmbs;	/* Entry in block element list */
	TAILQ_ENTRY(vg_element) vges;	/* Entry in global element list */
} VG_Element;

typedef struct vg {
	char name[VG_NAME_MAX];		/* Name of drawing */
	int flags;
#define VG_ANTIALIAS	0x001		/* Anti-alias where possible */
#define VG_HWSURFACE	0x002		/* Prefer video memory for fragments */
#define VG_VISORIGIN	0x004		/* Display the origin points */
#define VG_VISGRID	0x008		/* Display the grid */
#define VG_VISBBOXES	0x010		/* Display bounding boxes (debug) */
#define VG_ALPHA	0x020		/* Enable alpha channel */
#define VG_COLORKEY	0x040		/* Enable colorkey */
#define VG_RLEACCEL	0x080		/* Enable RLE acceleration */
#define VG_DIRECT	0x100		/* Render directly to display */

	AG_Mutex lock;
	float scale;			/* Scaling factor */
	float default_scale;		/* Default scaling factor */
	float grid_gap;			/* Grid size */

	Uint32 fill_color;		/* Background color */
	Uint32 grid_color;		/* Grid color */
	Uint32 selection_color;		/* Selected item/block color */
	Uint32 mouseover_color;		/* Mouse overlap item color */

	VG_Vtx	  *origin;		/* Reference points */
	float	  *origin_radius;	/* Reference point radii */
	Uint32	  *origin_color;	/* Reference point colors */
	Uint32	  norigin;

	VG_Layer *layers;
	Uint32	 nlayers;

	int	    cur_layer;	/* Layer selected for edition */
	VG_Block   *cur_block;	/* Block selected for edition */
	VG_Element *cur_vge;	/* Element selected for edition */

	enum vg_snap_mode  snap_mode;	/* Positional restriction */
	enum vg_ortho_mode ortho_mode;	/* Orthogonal restriction */

	AG_Event *preRasterEv;		/* Raised prior to drawing */
	AG_Event *postRasterEv;		/* Raised after drawing */

	SDL_Surface *su;		/* Raster surface */
	SDL_PixelFormat *fmt;		/* Raster pixel format */
	SDL_Rect rDst;			/* Rendering window */

	int *ints;			/* Used for scan conversion */
	Uint nints;

	TAILQ_HEAD(,vg_element) vges;	/* Elements in drawing */
	TAILQ_HEAD(,vg_block) blocks;	/* Blocks in drawing */
	TAILQ_HEAD(,vg_style) styles;	/* Global default styles */
} VG;

#define VG_RASXF(vg,cx) ((cx)*(vg)->scale + (vg)->origin[0].x*(vg)->scale)
#define VG_RASYF(vg,cy) ((cy)*(vg)->scale + (vg)->origin[0].y*(vg)->scale)
#define VG_RASLENF(vg,i) ((i)*(vg)->scale)
#define VG_VECXF(vg,rx) VG_VcoordX((vg),(rx))
#define VG_VECYF(vg,ry) VG_VcoordY((vg),(ry))
#define VG_VECLENF(vg,ry) (((float)(ry))/(vg)->scale)
#define VG_VECXF_NOSNAP(vg,rx) ((rx)/(vg)->scale - (vg)->origin[0].x)
#define VG_VECYF_NOSNAP(vg,ry) ((ry)/(vg)->scale - (vg)->origin[0].y)

#define VG_RASX(vg,cx) ((int)((cx)*(vg)->scale) + \
                       (int)((vg)->origin[0].x*(vg)->scale))
#define VG_RASY(vg,cy) ((int)((cy)*(vg)->scale) + \
                       (int)((vg)->origin[0].y*(vg)->scale))
#define VG_RASLEN(vg,i) (int)((i)*(vg)->scale)

extern const VG_ElementOps *vgElementTypes[];

__BEGIN_DECLS
void	 VG_InitSubsystem(void);
void	 VG_DestroySubsystem(void);

VG	*VG_New(int);
void	 VG_Init(VG *, int);
void	 VG_Destroy(VG *);
void	 VG_Reinit(VG *);
void	 VG_Save(VG *, AG_DataSource *);
int	 VG_Load(VG *, AG_DataSource *);

void	 	 VG_PreRasterFn(VG *, void *, AG_EventFn, const char *, ...);
void	 	 VG_PostRasterFn(VG *, void *, AG_EventFn, const char *, ...);
void		 VG_Scale(VG *, int, int, float);
void	 	 VG_DefaultScale(VG *, float);
void	 	 VG_SetGridGap(VG *, float);
void	 	 VG_Rasterize(VG *);

void	VG_Vcoords2(VG *, int, int, float *, float *);
void	VG_AbsVcoords2(VG *, int, int, float *, float *);
float	VG_VcoordX(VG *, int);
float	VG_VcoordY(VG *, int);
void	VG_Rcoords2(VG *, float, float, int *, int *);
void	VG_AbsRcoords2(VG *, float, float, int *, int *);
void	VG_Rcoords2d(VG *, float, float, float *, float *);
void	VG_AbsRcoords2d(VG *, float, float, float *, float *);
void	VG_VtxCoords2d(VG *, VG_Element *, int, float *, float *);
void	VG_VtxCoords2i(VG *, VG_Element *, int, int *, int *);
void	VG_RLength(VG *, float, int *);
void	VG_VLength(VG *, int, float *);

VG_Vtx		*VG_PopVertex(VG *);
VG_Vtx    	*VG_AllocVertex(VG_Element *);
VG_Matrix 	*VG_AllocMatrix(VG_Element *);
VG_Matrix	*VG_PopMatrix(VG *);
void		 VG_WriteVertex(AG_DataSource *, VG_Vtx *);
void		 VG_ReadVertex(AG_DataSource *, VG_Vtx *);

VG_Layer *VG_PushLayer(VG *, const char *);
void	  VG_PopLayer(VG *);

VG_Element	*VG_Begin(VG *, enum vg_element_type);
void		 VG_End(VG *);
void		 VG_Select(VG *, VG_Element *);
void		 VG_DestroyElement(VG *, VG_Element *);
void		 VG_FreeElement(VG *, VG_Element *);
int		 VG_Rintersect(VG *, VG_Rect *, VG_Rect *, VG_Rect *);
VG_Style	*VG_CreateStyle(VG *, enum vg_style_type, const char *);
int		 VG_SetStyle(VG *, const char *);
 
void	 VG_SetLayer(VG *, int);
void	 VG_Color(VG *, Uint32);
void	 VG_Color3(VG *, int r, int g, int b);
void	 VG_Color4(VG *, int r, int g, int b, int a);
VG_Vtx	*VG_Vertex2(VG *, float x, float y);
void	 VG_VertexV(VG *, const VG_Vtx *, Uint);
VG_Vtx	*VG_VertexVint2(VG *, float x, float x1, float y1, float x2, float y2);
void	 VG_Line(VG *, float x1, float y1, float x2, float y2);
void	 VG_VLine(VG *, float x, float y1, float y2);
void	 VG_HLine(VG *, float x1, float x2, float y);
void	 VG_VintVLine2(VG *, float x, float y, float x1, float y1, float x2,
	               float y2);
void	 VG_Rectangle(VG *, float x1, float y1, float x2, float y2);

VG_Matrix *VG_PushIdentity(VG *);
VG_Matrix *VG_Translate(VG *, float, float);
VG_Matrix *VG_Rotate(VG *, float);

void VG_LoadIdentity(VG_Matrix *);
void VG_LoadTranslate(VG_Matrix *, float, float);
void VG_LoadRotate(VG_Matrix *, float);
void VG_MultMatrixByVector(VG_Vtx *, const VG_Vtx *, const VG_Matrix *);
void VG_MultMatrixByMatrix(VG_Matrix *, const VG_Matrix *, const VG_Matrix *);
void VG_CopyMatrix(VG_Matrix *, const VG_Matrix *);

#ifdef DEBUG
void VG_DrawExtents(VG *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
