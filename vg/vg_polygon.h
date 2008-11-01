/*	Public domain	*/

typedef struct vg_polygon {
	struct vg_node _inherit;
	VG_Point **pts;			/* Vertices */
	Uint nPts;
	int outline;			/* Render outline */
	int *ints;			/* For scan conversion in FB mode */
	Uint nInts;
} VG_Polygon;

#define VGPOLYGON(p) ((VG_Polygon *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgPolygonOps;

static __inline__ VG_Polygon *
VG_PolygonNew(void *pNode)
{
	VG_Polygon *vP;

	vP = AG_Malloc(sizeof(VG_Polygon));
	VG_NodeInit(vP, &vgPolygonOps);
	VG_NodeAttach(pNode, vP);
	return (vP);
}

static __inline__ void
VG_PolygonSetOutline(VG_Polygon *vP, int flag)
{
	VG_Lock(VGNODE(vP)->vg);
	vP->outline = flag;
	VG_Unlock(VGNODE(vP)->vg);
}

static __inline__ void
VG_PolygonVertex(VG_Polygon *vP, VG_Point *pt)
{
	VG_Lock(VGNODE(vP)->vg);
	vP->pts = AG_Realloc(vP->pts, (vP->nPts+1)*sizeof(VG_Point *));
	vP->pts[vP->nPts++] = pt;
	VG_AddRef(vP, pt);
	VG_Unlock(VGNODE(vP)->vg);
}
__END_DECLS
