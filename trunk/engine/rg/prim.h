/*	$Csoft: prim.h,v 1.6 2005/05/30 01:29:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PRIM_H_
#define _AGAR_RG_PRIM_H_
#include "begin_code.h"

struct rg_tile;

enum rg_prim_blend_mode {
	RG_PRIM_OVERLAY_ALPHA,
	RG_PRIM_AVERAGE_ALPHA,
	RG_PRIM_SRC_ALPHA,
	RG_PRIM_DST_ALPHA,
};

__BEGIN_DECLS
void RG_RGB2HSV(Uint8, Uint8, Uint8, float *, float *, float *);
void RG_HSV2RGB(float, float, float, Uint8 *, Uint8 *, Uint8 *);

void RG_ColorRGB(struct rg_tile *, Uint8, Uint8, Uint8);
void RG_ColorRGBA(struct rg_tile *, Uint8, Uint8, Uint8, Uint8);
void RG_ColorHSV(struct rg_tile *, float, float, float);
void RG_ColorHSVA(struct rg_tile *, float, float, float, Uint8);
void RG_ColorUint32(struct rg_tile *, Uint32);

void RG_PutPixel(SDL_Surface *, int, int, Uint32);
void RG_BlendRGB(SDL_Surface *, int, int, enum rg_prim_blend_mode,
	            Uint8, Uint8, Uint8, Uint8);
void RG_Circle2(struct rg_tile *, int, int, int);
void RG_Line(struct rg_tile *, int, int, int, int);
void RG_WuLine(struct rg_tile *, double, double, double, double);
void RG_HLine(struct rg_tile *, int, int, int, Uint32);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_PRIM_H_ */
