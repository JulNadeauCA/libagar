/*	Public domain	*/

typedef struct vg_point {
	struct vg_node _inherit;	/* VG_Node(3) -> VG_Point */
	double size;			/* Size in pixels (0.0 = invisible) */
} VG_Point;

#define VGPOINT(p) ((VG_Point *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgPointOps;

VG_Point *_Nonnull VG_PointNew(void *_Nullable, VG_Vector);
void               VG_PointSize(VG_Point *_Nonnull, double);
__END_DECLS
