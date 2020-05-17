/*	Public domain	*/

enum vg_line_endpoint {
	VG_LINE_SQUARE,		/* Square endpoint */
	VG_LINE_BEVELED,	/* Beveled endpoint */
	VG_LINE_ROUNDED,	/* Rounded endpoint (circular) */
	VG_LINE_MITERED		/* Mitered endpoint */
};

typedef struct vg_line {
	struct vg_node _inherit;	/* VG_Node(3) -> VG_Line */
	VG_Point *_Nullable p1;		/* First endpoint */
	VG_Point *_Nullable p2;		/* Second endpoint */
	enum vg_line_endpoint endPt;	/* Endpoint rendering style */
	Uint16 stipple;			/* Stipple pattern (OpenGL-style) */
	Uint8  miterLen;		/* Miter length for VG_MITERED */
	Uint8  thickness;		/* Thickness in pixels */
} VG_Line;

#define VGLINE(p) ((VG_Line *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgLineOps;

VG_Line *_Nonnull VG_LineNew(void *_Nullable, VG_Point *_Nonnull, VG_Point *_Nonnull);

void VG_LineThickness(VG_Line *_Nonnull, Uint8);
void VG_LineStipple(VG_Line *_Nonnull, Uint16);
void VG_LineEndpointStyle(VG_Line *_Nonnull, enum vg_line_endpoint, ...);
__END_DECLS
