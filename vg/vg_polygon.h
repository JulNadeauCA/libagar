/*	Public domain	*/

typedef struct vg_polygon {
	struct vg_node _inherit;
	int outline;				/* Render outline? */
	Uint                         nPts;
	VG_Point *_Nullable *_Nonnull pts;	/* Vertices */

	int  *_Nullable ints;	/* Sorted intersections (for FB rendering) */
	Uint           nInts;
	Uint32 _pad;
} VG_Polygon;

#define VGPOLYGON(p) ((VG_Polygon *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgPolygonOps;

VG_Polygon *_Nonnull VG_PolygonNew(void *_Nullable);

void VG_PolygonSetOutline(VG_Polygon *_Nonnull, int);
Uint VG_PolygonVertex(VG_Polygon *_Nonnull, VG_Point *_Nonnull);
void VG_PolygonDelVertex(VG_Polygon *_Nonnull, Uint);
__END_DECLS
