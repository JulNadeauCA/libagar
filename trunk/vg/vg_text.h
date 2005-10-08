/*	$Csoft: vg_text.h,v 1.7 2005/06/04 04:48:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TEXT_H_
#define _AGAR_VG_TEXT_H_
#include "begin_code.h"

#define VG_TEXT_MAX	 256
#define VG_TEXT_MAX_PTRS 32

struct vg_text_args {
	SDL_Surface *su;		/* Text surface */
	char text[VG_TEXT_MAX];		/* Text buffer */
	double angle;			/* Angle of rotation (deg) */
	enum vg_alignment align;	/* Alignment around vertex */
	void *ptrs[VG_TEXT_MAX_PTRS];	/* Pointers (for polling) */
	int  nptrs;
};

__BEGIN_DECLS
void	VG_TextAlignment(struct vg *, enum vg_alignment);
void	VG_TextAngle(struct vg *, double);
void	VG_Printf(struct vg *, const char *, ...);
void	VG_PrintfP(struct vg *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TEXT_H_ */
