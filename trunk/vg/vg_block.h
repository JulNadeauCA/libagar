/*	$Csoft: vg_block.h,v 1.8 2005/06/04 04:48:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_BLOCK_H_
#define _AGAR_VG_BLOCK_H_

#include "begin_code.h"

#define VG_BLOCK_NAME_MAX 128

typedef struct vg_block {
	char name[VG_BLOCK_NAME_MAX];
	int flags;
#define VG_BLOCK_NOSAVE	0x01		/* Don't save with drawing */
	VG_Vtx pos;			/* Position in vg */
	VG_Vtx origin;			/* Block origin */
	float theta;			/* Angle of rotation */
	int selected;
	TAILQ_HEAD(,vg_element) vges;
	TAILQ_ENTRY(vg_block) vgbs;
} VG_Block;

struct ag_window;

__BEGIN_DECLS
VG_Block	  *VG_BeginBlock(struct vg *, const char *, int);
__inline__ void	   VG_SelectBlock(struct vg *, VG_Block *);
__inline__ void	   VG_EndBlock(struct vg *);
VG_Block	  *VG_GetBlock(struct vg *, const char *);
void		   VG_MoveBlock(struct vg *, VG_Block *, float, float, int);
__inline__ void	   VG_BlockTheta(struct vg *, VG_Block *, float);
void		   VG_RotateBlock(struct vg *, VG_Block *, float);
void		   VG_ClearBlock(struct vg *, VG_Block *);
void		   VG_DestroyBlock(struct vg *, VG_Block *);
void		   VG_BlockOffset(struct vg *, VG_Vtx *);
struct ag_window  *VG_BlockEditor(struct vg *);
void		   VG_BlockExtent(struct vg *, VG_Block *, VG_Rect *);
__inline__ void	   VG_Abs2Rel(struct vg *, const VG_Vtx *, float *, float *);
__inline__ void	   VG_Rel2Abs(struct vg *, float, float, VG_Vtx *);
VG_Block	  *VG_BlockClosest(struct vg *, float, float);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_BLOCK_H_ */
