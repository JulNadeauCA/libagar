/*	Public domain	*/

typedef struct vg_arc {
	struct vg_node _inherit;
	VG_Point *_Nullable p;		/* Centerpoint */
	double r;			/* Arc radius */
	float a1;			/* Start angle (degs) */
	float a2;			/* End angle (degs) */
} VG_Arc;

#define VGARC(p) ((VG_Arc *)(p))

__BEGIN_DECLS
extern VG_NodeOps vgArcOps;

VG_Arc *_Nonnull VG_ArcNew(void *_Nullable, VG_Point *_Nonnull,
                           float, float,float);

void VG_ArcCenter(VG_Arc *_Nonnull, VG_Point *_Nonnull);
void VG_ArcRadius(VG_Arc *_Nonnull, double);
__END_DECLS
