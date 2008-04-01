/*	Public domain	*/

__BEGIN_DECLS
extern const VG_NodeOps vgArcOps;

void VG_ArcBox(struct vg *, float, float);
void VG_ArcRange(struct vg *, float, float);
void VG_Arc3Points(struct vg *, VG_Vtx v[3]);
__END_DECLS
