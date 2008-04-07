/*	Public domain	*/

enum vg_line_endpoint {
	VG_LINE_SQUARE,		/* Square endpoint */
	VG_LINE_BEVELED,	/* Beveled endpoint */
	VG_LINE_ROUNDED,	/* Rounded endpoint (circular) */
	VG_LINE_MITERED		/* Mitered endpoint */
};

typedef struct vg_line {
	struct vg_node _inherit;
	VG_Point *p1, *p2;
	enum vg_line_endpoint endPt;	/* Endpoint style */
	Uint16 stipple;			/* Stipple pattern (OpenGL-style) */
	Uint8  miterLen;		/* Miter length for VG_MITERED */
	Uint8  thickness;		/* Thickness in pixels */
} VG_Line;

#define VGLINE(p) ((VG_Line *)(p))

__BEGIN_DECLS
extern const VG_NodeOps vgLineOps;

static __inline__ VG_Line *
VG_LineNew(void *pNode, VG_Point *p1, VG_Point *p2)
{
	VG_Line *vl;

	vl = AG_Malloc(sizeof(VG_Line));
	VG_NodeInit(vl, &vgLineOps);
	vl->p1 = p1;
	vl->p2 = p2;
	VG_AddRef(vl, p1);
	VG_AddRef(vl, p2);
	VG_NodeAttach(pNode, vl);
	return (vl);
}
__END_DECLS
