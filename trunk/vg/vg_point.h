/*	Public domain	*/

typedef struct vg_point {
	struct vg_node _inherit;
	float x, y;
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
	VG *vg = VGNODE(pt)->vg;
	VG_Vector v;

	v.x = pt->x;
	v.y = pt->y;
	VG_MultMatrixByVector(&v, &v, &vg->T[vg->nT-1]);
	return (v);
}

static __inline__ VG_Point *
VG_PointNew(void *pNode, VG_Vector pos)
{
	VG_Point *vp;

	vp = AG_Malloc(sizeof(VG_Point));
	VG_NodeInit(vp, &vgPointOps);
	vp->x = pos.x;
	vp->y = pos.y;
	VG_NodeAttach(pNode, vp);
	return (vp);
}
__END_DECLS
