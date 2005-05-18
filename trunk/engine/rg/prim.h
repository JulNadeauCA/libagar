/*	$Csoft: prim.h,v 1.4 2005/04/10 09:09:02 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PRIM_H_
#define _AGAR_RG_PRIM_H_
#include "begin_code.h"

struct tile;

enum prim_blend_mode {
	PRIM_OVERLAY_ALPHA,
	PRIM_AVERAGE_ALPHA,
	PRIM_SRC_ALPHA,
	PRIM_DST_ALPHA,
};

__BEGIN_DECLS
void prim_rgb2hsv(Uint8, Uint8, Uint8, float *, float *, float *);
void prim_hsv2rgb(float, float, float, Uint8 *, Uint8 *, Uint8 *);

void prim_color_rgb(struct tile *, Uint8, Uint8, Uint8);
void prim_color_rgba(struct tile *, Uint8, Uint8, Uint8, Uint8);
void prim_color_hsv(struct tile *, float, float, float);
void prim_color_hsva(struct tile *, float, float, float, Uint8);
void prim_color_u32(struct tile *, Uint32);

void prim_put_pixel(SDL_Surface *, int, int, Uint32);
void prim_blend_rgb(SDL_Surface *, int, int, enum prim_blend_mode,
	            Uint8, Uint8, Uint8, Uint8);
void prim_circle2(struct tile *, int, int, int);
void prim_line(struct tile *, int, int, int, int);
void prim_wuline(struct tile *, double, double, double, double);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_PRIM_H_ */
