/*	Public domain	*/

#ifndef _AGAR_RG_PRIM_H_
#define _AGAR_RG_PRIM_H_
#include <agar/rg/begin.h>

struct rg_tile;

enum rg_prim_blend_mode {
	RG_PRIM_OVERLAY_ALPHA,
	RG_PRIM_AVERAGE_ALPHA,
	RG_PRIM_SRC_ALPHA,
	RG_PRIM_DST_ALPHA,
};

__BEGIN_DECLS
void RG_ColorRGB(struct rg_tile *, AG_Color C);
void RG_ColorRGBA(struct rg_tile *, AG_Color C);
void RG_ColorHSV(struct rg_tile *, float, float, float);
void RG_ColorHSVA(struct rg_tile *, float, float, float, Uint8);
void RG_ColorUint32(struct rg_tile *, Uint32);

void RG_BlendRGB(AG_Surface *, int, int, enum rg_prim_blend_mode, AG_Color C);
void RG_Circle2(struct rg_tile *, int, int, int);
void RG_Line(struct rg_tile *, int, int, int, int);
void RG_WuLine(struct rg_tile *, float, float, float, float);
void RG_HLine(struct rg_tile *, int, int, int, Uint32);

static __inline__ void
RG_PutPixel(AG_Surface *su, int x, int y, Uint32 pc)
{
	Uint8 *dst;
	    
	if (x < 0 || y < 0 || x >= su->w || y >= su->h) {
		return;
	}
	dst = (Uint8 *)su->pixels + y*su->pitch + (x << 2);
	*(Uint32 *)dst = pc;
}

__END_DECLS

#include <agar/rg/close.h>
#endif	/* _AGAR_RG_PRIM_H_ */
