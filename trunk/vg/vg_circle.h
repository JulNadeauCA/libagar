/*	Public domain	*/

typedef struct vg_circle {
	struct vg_node _inherit;
	VG_Point *p;
	float r;
} VG_Circle;

#define VGCIRCLE(p) ((VG_Circle *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgCircleOps;

static __inline__ VG_Circle *
VG_CircleNew(void *pNode, VG_Point *pCenter, float r)
{
	VG_Circle *vc;

	vc = (VG_Circle *)AG_Malloc(sizeof(VG_Circle));
	VG_NodeInit(vc, &vgCircleOps);
	vc->p = pCenter;
	vc->r = r;
	VG_AddRef(vc, pCenter);
	VG_NodeAttach(pNode, vc);
	return (vc);
}

static __inline__ void
VG_CircleCenter(VG_Circle *vc, VG_Point *pCenter)
{
	VG_Lock(VGNODE(vc)->vg);
	VG_DelRef(vc, vc->p);
	VG_AddRef(vc, pCenter);
	vc->p = pCenter;
	VG_Unlock(VGNODE(vc)->vg);
}
__END_DECLS
