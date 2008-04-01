/*	Public domain	*/

#define VG_BLOCK_NAME_MAX 128

typedef struct vg_block {
	char name[VG_BLOCK_NAME_MAX];
	int flags;
#define VG_BLOCK_NOSAVE	0x01		/* Don't save with drawing */
	VG_Vtx pos;			/* Position in vg */
	float theta;			/* Angle of rotation */
	int selected;
	AG_TAILQ_HEAD(,vg_node) nodes;
	AG_TAILQ_ENTRY(vg_block) vgbs;
} VG_Block;

struct ag_window;

__BEGIN_DECLS
VG_Block	  *VG_BeginBlock(struct vg *, const char *, int);
void	   	   VG_SelectBlock(struct vg *, VG_Block *);
void	   	   VG_EndBlock(struct vg *);
VG_Block	  *VG_GetBlock(struct vg *, const char *);
void		   VG_MoveBlock(struct vg *, VG_Block *, float, float, int);
void	   	   VG_BlockTheta(struct vg *, VG_Block *, float);
void		   VG_RotateBlock(struct vg *, VG_Block *, float);
void		   VG_ClearBlock(struct vg *, VG_Block *);
void		   VG_DestroyBlock(struct vg *, VG_Block *);
struct ag_window  *VG_BlockEditor(struct vg *);
void		   VG_BlockExtent(struct vg_view *, VG_Block *, VG_Rect *);
void	   	   VG_Abs2Rel(struct vg *, const VG_Vtx *, float *, float *);
void	 	   VG_Rel2Abs(struct vg *, float, float, VG_Vtx *);
VG_Block	  *VG_BlockClosest(struct vg *, float, float);

/* Convert block VG coordinates to absolute. */
static __inline__ void
VG_BlockOffset(VG *vg, VG_Vtx *vtx)
{
	if (vg->curBlock != NULL) {
		vtx->x += vg->curBlock->pos.x;
		vtx->y += vg->curBlock->pos.y;
	}
}
__END_DECLS
