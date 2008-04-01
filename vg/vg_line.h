/*	Public domain	*/

__BEGIN_DECLS
extern const VG_NodeOps vgLinesOps;
extern const VG_NodeOps vgLineStripOps;
extern const VG_NodeOps vgLineLoopOps;

void VG_DrawLineSegments(struct vg_view *, struct vg_node *);
void VG_DrawLineStrip(struct vg_view *, struct vg_node *);
void VG_DrawLineLoop(struct vg_view *, struct vg_node *);
void VG_LineExtent(struct vg_view *, struct vg_node *, struct vg_rect *);
float VG_LineIntersect(struct vg *, struct vg_node *, float *, float *);
float VG_ClosestLinePoint(struct vg *, float, float, float, float, float *, 
                          float *);
__END_DECLS
