/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

typedef struct ag_primitive_ops {
	void (*Box)(void *, AG_Rect r, int z, Uint32 c);
	void (*BoxRounded)(void *, AG_Rect r, int z, int radius, Uint32 c);
	void (*BoxRoundedTop)(void *, AG_Rect r, int z, int radius, Uint32 c);
	void (*BoxDithered)(void *, AG_Rect r, int z, Uint32 c1, Uint32 c2);
	void (*Frame)(void *, AG_Rect r, int z, Uint32 c);
	void (*FrameBlended)(void *, AG_Rect r, Uint8 c[4], AG_BlendFn fn);
	void (*Circle)(void *, int x, int y, int radius, Uint32 c);
	void (*Circle2)(void *, int x, int y, int radius, Uint32 c);
	void (*Line)(void *, int x1, int y1, int x2, int y2, Uint32 c);
	void (*Line2)(void *, int x1, int y1, int x2, int y2, Uint32 c);
	void (*LineBlended)(void *, int x1, int y1, int x2, int y2,
		            Uint8 c[4], AG_BlendFn fn);
	void (*LineH)(void *, int x1, int x2, int y, Uint32 c);
	void (*LineV)(void *, int x, int y1, int y2, Uint32 c);
	void (*RectOutline)(void *, AG_Rect r, Uint32 c);
	void (*RectFilled)(void *, AG_Rect r, Uint32 c);
	void (*RectBlended)(void *, AG_Rect r, Uint8 c[4], AG_BlendFn fn);
	void (*Plus)(void *, AG_Rect r, Uint8 c[4], AG_BlendFn fn);
	void (*Minus)(void *, AG_Rect r, Uint8 c[4], AG_BlendFn fn);
	void (*Tiling)(void *, AG_Rect r, int tileSize, int scrollOffset,
	               Uint32 c1, Uint32 c2);
	void (*ArrowUp)(void *, int x, int y, int l, Uint32 c1, Uint32 c2);
	void (*ArrowDown)(void *, int x, int y, int l, Uint32 c1, Uint32 c2);
	void (*ArrowLeft)(void *, int x, int y, int l, Uint32 c1, Uint32 c2);
	void (*ArrowRight)(void *, int x, int y, int l, Uint32 c1, Uint32 c2);
} AG_PrimitiveOps;

extern AG_PrimitiveOps agPrim;

#define AG_DrawPixel AG_WidgetPutPixel
#define AG_DrawPixelBlended AG_WidgetBlendPixel32
#define AG_DrawPixelRGB AG_WidgetPutPixelRGB
#define AG_DrawBox agPrim.Box
#define AG_DrawBoxRounded agPrim.BoxRounded
#define AG_DrawBoxRoundedTop agPrim.BoxRoundedTop
#define AG_DrawBoxDithered agPrim.BoxDithered
#define AG_DrawFrame agPrim.Frame
#define AG_DrawFrameBlended agPrim.FrameBlended
#define AG_DrawCircle agPrim.Circle
#define AG_DrawCircle2 agPrim.Circle2
#define AG_DrawLine agPrim.Line
#define AG_DrawLine2 agPrim.Line2
#define AG_DrawLineBlended agPrim.LineBlended
#define AG_DrawLineH agPrim.LineH
#define AG_DrawLineV agPrim.LineV
#define AG_DrawRectOutline agPrim.RectOutline
#define AG_DrawRectFilled agPrim.RectFilled
#define AG_DrawRectBlended agPrim.RectBlended
#define AG_DrawPlus agPrim.Plus
#define AG_DrawMinus agPrim.Minus
#define AG_DrawTiling agPrim.Tiling
#define AG_DrawArrowUp agPrim.ArrowUp
#define AG_DrawArrowDown agPrim.ArrowDown
#define AG_DrawArrowLeft agPrim.ArrowLeft
#define AG_DrawArrowRight agPrim.ArrowRight

__BEGIN_DECLS
void AG_InitPrimitives(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
