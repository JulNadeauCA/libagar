/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

typedef struct ag_primitive_ops {
	void (*box)(void *, int x, int y, int w, int h, int depth, Uint32 c);
	void (*box_chamfered)(void *, SDL_Rect *rBox, int depth, int radius,
		              Uint32 c);
	void (*box_dithered)(void *, int x, int y, int w, int h, int depth,
		             Uint32 c1, Uint32 c2);
	void (*frame)(void *, int x, int y, int w, int h, int z, Uint32 c);
	void (*frame_blended)(void *, int x, int y, int w, int h,
		              Uint8 c[4], enum ag_blend_func fn);
	void (*circle)(void *, int x, int y, int radius, Uint32 c);
	void (*circle2)(void *, int x, int y, int radius, Uint32 c);
	void (*line)(void *, int x1, int y1, int x2, int y2, Uint32 c);
	void (*line2)(void *, int x1, int y1, int x2, int y2, Uint32 c);
	void (*line_blended)(void *, int x1, int y1, int x2, int y2,
		             Uint8 c[4], enum ag_blend_func fn);
	void (*hline)(void *, int x1, int x2, int y, Uint32 c);
	void (*vline)(void *, int x, int y1, int y2, Uint32 c);
	void (*rect_outlined)(void *, int x, int y, int w, int h, Uint32 c);
	void (*rect_filled)(void *, int x, int y, int w, int h, Uint32 c);
	void (*rect_blended)(void *, int x, int y, int w, int h, Uint8 c[4],
	                     enum ag_blend_func fn);
	void (*plus)(void *, int x, int y, int w, int h, Uint8 c[4],
	             enum ag_blend_func fn);
	void (*minus)(void *, int x, int y, int w, int h, Uint8 c[4],
	              enum ag_blend_func fn);
	void (*tiling)(void *, SDL_Rect rd, int tileSize, int scrollOffset,
	               Uint32 c1, Uint32 c2);
	void (*arrow_up)(void *, int x, int y, int len, Uint32 c1, Uint32 c2);
	void (*arrow_down)(void *, int x, int y, int len, Uint32 c1, Uint32 c2);
	void (*arrow_left)(void *, int x, int y, int len, Uint32 c1, Uint32 c2);
	void (*arrow_right)(void *, int x, int y, int len, Uint32 c1,Uint32 c2);
} AG_PrimitiveOps;

extern AG_PrimitiveOps agPrim;

__BEGIN_DECLS
void AG_InitPrimitives(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
