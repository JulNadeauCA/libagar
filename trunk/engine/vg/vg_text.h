/*	$Csoft: vg_text.h,v 1.2 2004/04/22 12:36:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX		128
#define VG_FONT_FACE_MAX	32
#define VG_FONT_SIZE_MIN	4
#define VG_FONT_SIZE_MAX	48

__BEGIN_DECLS
void	vg_text_init(struct vg *, struct vg_element *);
void	vg_text_align(struct vg *, enum vg_alignment, double);
int	vg_text_font(struct vg *, const char *, int, int);
void	vg_text_printf(struct vg *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
