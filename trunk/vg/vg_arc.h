/*	Public domain	*/

typedef struct vg_arc {
	struct vg_node _inherit;
	VG_Point *p;			/* Centerpoint */
	float r;			/* Arc radius */
	float a1;			/* Start angle (degs) */
	float a2;			/* End angle (degs) */
} VG_Arc;

#define VGARC(p) ((VG_Arc *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgArcOps;

static __inline__ VG_Arc *
VG_ArcNew(void *pNode, VG_Point *pCenter, float r, float a1, float a2)
{
	VG_Arc *va;

	va = (VG_Arc *)AG_Malloc(sizeof(VG_Arc));
	VG_NodeInit(va, &vgArcOps);
	va->p = pCenter;
	va->r = r;
	va->a1 = a1;
	va->a2 = a2;
	VG_AddRef(va, pCenter);
	VG_NodeAttach(pNode, va);
	return (va);
}

static __inline__ void
VG_ArcCenter(VG_Arc *va, VG_Point *pCenter)
{
	VG_Lock(VGNODE(va)->vg);
	VG_DelRef(va, va->p);
	VG_AddRef(va, pCenter);
	va->p = pCenter;
	VG_Unlock(VGNODE(va)->vg);
}

static __inline__ void
VG_ArcRadius(VG_Arc *va, float r)
{
	VG_Lock(VGNODE(va)->vg);
	va->r = r;
	VG_Unlock(VGNODE(va)->vg);
}
__END_DECLS
