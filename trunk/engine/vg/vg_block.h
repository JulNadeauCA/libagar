/*	$Csoft: vg_block.h,v 1.2 2004/05/06 08:47:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_BLOCK_H_
#define _AGAR_VG_BLOCK_H_

#include <engine/widget/window.h>

#include "begin_code.h"

#define VG_BLOCK_NAME_MAX 128

struct vg_block {
	char name[VG_BLOCK_NAME_MAX];
	int flags;
#define VG_BLOCK_NOSAVE	0x01		/* Don't save with drawing */
	struct vg_vertex pos;		/* Position in vg */
	struct vg_vertex origin;	/* Internal block origin */
	TAILQ_HEAD(,vg_element) vges;
	TAILQ_ENTRY(vg_block) vgbs;
};

__BEGIN_DECLS
struct vg_block	  *vg_begin_block(struct vg *, const char *, int);
__inline__ void	   vg_select_block(struct vg *, struct vg_block *);
__inline__ void	   vg_end_block(struct vg *);
struct vg_block	  *vg_get_block(struct vg *, const char *);
void		   vg_move_block(struct vg *, struct vg_block *,
		                 double, double, int);
void		   vg_rotate_block(struct vg *, struct vg_block *, double);
void		   vg_destroy_block(struct vg *, struct vg_block *);
void		   vg_block_offset(struct vg *, struct vg_vertex *);
struct window	  *vg_block_editor(struct vg *);
__inline__ void	   vg_abs2rel(struct vg *, const struct vg_vertex *, double *,
			      double *);
__inline__ void	   vg_rel2abs(struct vg *, double, double, struct vg_vertex *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_BLOCK_H_ */
