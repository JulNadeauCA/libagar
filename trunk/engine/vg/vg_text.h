/*	$Csoft: vg_text.h,v 1.1 2004/03/30 16:04:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX	256

__BEGIN_DECLS
void	vg_text_init(struct vg *, struct vg_element *);
void	vg_text_align(struct vg *, double, double, enum vg_alignment, double);
void	vg_text_printf(struct vg *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
