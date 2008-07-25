/*	Public domain	*/

typedef struct vg_point {
	struct vg_node _inherit;
	float size;			/* Size in pixels (0.0 = invisible) */
} VG_Point;

#define VGPOINT(p) ((VG_Point *)(p))

__BEGIN_DECLS
extern const VG_NodeOps vgPointOps;

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

static __inline__ void
VG_PointSize(VG_Point *vp, float r)
{
	VG_Lock(VGNODE(vp)->vg);
	vp->size = r;
	VG_Unlock(VGNODE(vp)->vg);
}
__END_DECLS
