/*	Public domain	*/

__BEGIN_DECLS
extern const VG_NodeOps vgTextOps;

void VG_TextAlignment(struct vg *, enum vg_alignment);
void VG_TextAngle(struct vg *, float);
void VG_Printf(struct vg *, const char *, ...);
void VG_PrintfP(struct vg *, const char *, ...);
__END_DECLS
