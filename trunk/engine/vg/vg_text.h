/*	$Csoft: vg_text.h,v 1.4 2004/05/12 04:51:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX		256
		
struct vg_text_args {
	SDL_Surface *su;		/* Text surface */
	char text[VG_TEXT_MAX];		/* Text buffer */
	double angle;			/* Angle of rotation (deg) */
	enum vg_alignment align;	/* Alignment around vertex */
};

__BEGIN_DECLS
void	vg_text_init(struct vg *, struct vg_element *);
void	vg_text_destroy(struct vg *, struct vg_element *);
void	vg_text_align(struct vg *, enum vg_alignment);
void	vg_text_angle(struct vg *, double);
void	vg_printf(struct vg *, const char *, ...);
void	vg_draw_text(struct vg *, struct vg_element *);
void	vg_text_bbox(struct vg *, struct vg_element *, struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
