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

typedef struct vg_color {
	Uint8 r, g, b, a;
} VG_Color;

struct vg;
struct vg_view;
struct vg_element;

#include "close_code.h"

#ifdef _AGAR_INTERNAL
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
	void (*draw)(struct vg_view *, struct vg_element *);
	void (*bbox)(struct vg *, struct vg_element *, VG_Rect *);
	float (*intsect)(struct vg*, struct vg_element *, float *, float *);
} VG_ElementOps;

typedef struct vg_layer {
	char name[VG_LAYER_NAME_MAX];	/* Layer name */
	int visible;			/* Flag of visibility */
	VG_Color color;			/* Per-layer default color */
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
	Uint flags;
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
	VG_Color color;
	union {
		VG_LineStyle vg_line_style;
		VG_TextStyle vg_text_style;
		VG_FillingStyle vg_fill_style;
	} vg_style_args;
#define vg_line_st	vg_style_args.vg_line_style
#define vg_text_st	vg_style_args.vg_text_style
#define vg_fill_st	vg_style_args.vg_fill_style
	AG_TAILQ_ENTRY(vg_style) styles;
} VG_Style;

typedef struct vg_matrix {
	float m[4][4];
} VG_Matrix;

typedef struct vg_element {
	enum vg_element_type type;
	const VG_ElementOps *ops;

	Uint flags;
#define VG_ELEMENT_NOSAVE	0x01	/* Don't save with drawing */
#define VG_ELEMENT_SELECTED	0x02	/* Selection flag */
#define VG_ELEMENT_MOUSEOVER	0x04	/* Mouse overlap flag */

	VG_Block *block;		/* Back pointer to block */
	VG_Style *style;		/* Default element style */
	VG_Color  color;		/* Effective foreground color */
	int       layer;		/* Associated layer */
	
	VG_LineStyle    line_st;	/* Effective line style */
	VG_FillingStyle fill_st;	/* Effective filling style */
	VG_TextStyle    text_st;	/* Effective text style */

	VG_Vtx    *vtx;			/* Vertices */
	Uint      nvtx;
	VG_Matrix *trans;		/* Transformation matrices */
	Uint      ntrans;

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
	AG_TAILQ_ENTRY(vg_element) vgbmbs; /* Entry in block element list */
	AG_TAILQ_ENTRY(vg_element) vges;   /* Entry in global element list */
} VG_Element;

typedef struct vg {
	char name[VG_NAME_MAX];		/* Name of drawing */

	Uint flags;
#define VG_ANTIALIAS	0x01		/* Anti-alias where possible */
#define VG_VISORIGIN	0x02		/* Display origin points */
#define VG_VISGRID	0x04		/* Display grid */
#define VG_VISBBOXES	0x08		/* Display bounding boxes (DEBUG) */

	AG_Mutex lock;

	float gridIval;			/* Grid interval */
	VG_Color fillColor;		/* Background color */
	VG_Color gridColor;		/* Grid color */
	VG_Color selectionColor;	/* Selected item/block color */
	VG_Color mouseoverColor;	/* Mouse overlap item color */

	VG_Vtx	  *origin;		/* Reference points */
	float	  *originRadius;	/* Reference point radii */
	VG_Color  *originColor;		/* Reference point colors */
	Uint32	  norigin;

	VG_Layer *layers;
	Uint32	 nlayers;

	int	    cur_layer;		/* Layer selected for edition */
	VG_Block   *cur_block;		/* Block selected for edition */
	VG_Element *cur_vge;		/* Element selected for edition */

	int *ints;			/* For scan conversion */
	Uint nints;

	AG_TAILQ_HEAD(,vg_element) vges;	/* Elements in drawing */
	AG_TAILQ_HEAD(,vg_block) blocks;	/* Blocks in drawing */
	AG_TAILQ_HEAD(,vg_style) styles;	/* Global default styles */
} VG;

extern const VG_ElementOps *vgElementTypes[];

__BEGIN_DECLS
void       VG_InitSubsystem(void);
void       VG_DestroySubsystem(void);

VG        *VG_New(Uint);
void       VG_Init(VG *, Uint);
void       VG_Destroy(VG *);
void       VG_Reinit(VG *);
void       VG_Save(VG *, AG_DataSource *);
int        VG_Load(VG *, AG_DataSource *);

void       VG_SetBackgroundColor(VG *, VG_Color);
void       VG_SetGridColor(VG *, VG_Color);
void       VG_SetSelectionColor(VG *, VG_Color);
void       VG_SetMouseOverColor(VG *, VG_Color);

VG_Vtx    *VG_PopVertex(VG *);
VG_Vtx    *VG_AllocVertex(VG_Element *);
VG_Matrix *VG_AllocMatrix(VG_Element *);
VG_Matrix *VG_PopMatrix(VG *);
VG_Vtx     VG_ReadVertex(AG_DataSource *);
void       VG_WriteVertex(AG_DataSource *, const VG_Vtx *);
VG_Color   VG_ReadColor(AG_DataSource *);
void       VG_WriteColor(AG_DataSource *, const VG_Color *);

VG_Layer  *VG_PushLayer(VG *, const char *);
void       VG_PopLayer(VG *);

VG_Element *VG_Begin(VG *, enum vg_element_type);
void        VG_End(VG *);
void        VG_Select(VG *, VG_Element *);
void        VG_DestroyElement(VG *, VG_Element *);
void        VG_FreeElement(VG *, VG_Element *);
VG_Style   *VG_CreateStyle(VG *, enum vg_style_type, const char *);
int         VG_SetStyle(VG *, const char *);
 
void	 VG_SetLayer(VG *, int);
void	 VG_Colorv(VG *, const VG_Color *);
void	 VG_ColorRGB(VG *, Uint8, Uint8, Uint8);
void	 VG_ColorRGBA(VG *, Uint8, Uint8, Uint8, Uint8);
VG_Vtx	*VG_Vertex2(VG *, float, float);
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
void VG_DrawExtents(struct vg_view *);
#endif

static __inline__ VG_Color
VG_GetColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	VG_Color vc;
	vc.r = r;
	vc.g = g;
	vc.b = b;
	vc.a = 255;
	return (vc);
}

static __inline__ Uint32
VG_MapColorRGB(VG_Color vc)
{
	return SDL_MapRGB(agVideoFmt, vc.r, vc.g, vc.b);
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_H_ */
