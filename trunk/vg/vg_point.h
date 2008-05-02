/*	Public domain	*/

typedef struct vg_point {
	struct vg_node _inherit;
} VG_Point;

#define VGPOINT(p) ((VG_Point *)(p))

__BEGIN_DECLS
extern const VG_NodeOps vgPointOps;

/*
 * Return the absolute position of a point as transformed by the
 * current VG viewing matrix.
 */
static __inline__ VG_Vector
VG_PointPos(VG_Point *pt)
{
	VG_Matrix T;
	VG_Vector v = { 0.0f, 0.0f };

	VG_NodeTransform(pt, &T);
	VG_MultMatrixByVector(&v, &v, &T);
	return (v);
}

static __inline__ VG_Point *
VG_PointNew(void *pNode, VG_Vector pos)
{
	VG_Point *vp;

	vp = AG_Malloc(sizeof(VG_Point));
	VG_NodeInit(vp, &vgPointOps);
	VG_Translate(vp, pos);
	VG_NodeAttach(pNode, vp);
	return (vp);
}
__END_DECLS
