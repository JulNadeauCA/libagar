/*	$Csoft: vg_primitive.h,v 1.7 2005/06/07 02:42:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_PRIMITIVE_H_
#define _AGAR_VG_PRIMITIVE_H_

#ifdef _AGAR_INTERNAL
#include <gui/view.h>
#else
#include <agar/gui/view.h>
#endif

#include "begin_code.h"

__BEGIN_DECLS
void	VG_CirclePrimitive(VG *, int, int, int, Uint32);
void	VG_ArcPrimitive(VG *, int, int, int, int, int, int,
		                 Uint32);
void	VG_LinePrimitive(VG *, int, int, int, int, Uint32);
void	VG_HLinePrimitive(VG *, int, int, int, Uint32);
void	VG_WuLinePrimitive(VG *, float, float, float,
		                    float, int, Uint32);
void	VG_RectPrimitive(VG *, int, int, int, int, Uint32);

static __inline__ void
VG_PutPixel(VG *vg, int x, int y, Uint32 c)
{
	Uint8 *d;

	if (AG_CLIPPED_PIXEL(vg->su, x, y)) {
		return;
	}
	d = (Uint8 *)vg->su->pixels + y*vg->su->pitch +
	                              x*vg->fmt->BytesPerPixel;
	switch (vg->fmt->BytesPerPixel) {
	case 4:
		*(Uint32 *)d = c;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		d[0] = (c>>16) & 0xff;
		d[1] = (c>>8) & 0xff;
		d[2] = c & 0xff;	
#else
		d[2] = (c>>16) & 0xff;
		d[1] = (c>>8) & 0xff;
		d[0] = c & 0xff;	
#endif
		break;
	case 2:
		*(Uint16 *)d = c;
		break;
	case 1:
		*d = c;
		break;
	}
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_PRIMITIVE_H_ */
