/*	$Csoft: vg.h,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX	256

__BEGIN_DECLS
void	vg_text_align(struct vg *, double, double, enum vg_alignment, double);
void	vg_text_printf(struct vg *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
