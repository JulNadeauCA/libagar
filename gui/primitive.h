/*	Public domain	*/

/*
 * Primitive GUI rendering routines.
 */

#ifndef _AGAR_GUI_PRIMITIVE_H_
#define _AGAR_GUI_PRIMITIVE_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

__BEGIN_DECLS
/* Increment individual RGB components of a pixel. */
/* XXX TODO use SIMD where available */
static __inline__ AG_Color
AG_ColorShift(AG_Color C, Sint8 *shift)
{
	int r = C.r + shift[0];
	int g = C.g + shift[1];
	int b = C.b + shift[2];

	if (r > 255) { r = 255; } else if (r < 0) { r = 0; }
	if (g > 255) { g = 255; } else if (g < 0) { g = 0; }
	if (b > 255) { b = 255; } else if (b < 0) { b = 0; }

	C.r = (Uint8)r;
	C.g = (Uint8)g;
	C.b = (Uint8)b;
	return (C);
}

/*
 * Calls to rendering routines implemented by the underlying driver.
 */

/* Write a pixel (AG_Color argument) */
static __inline__ void
AG_PutPixel(void *obj, int x, int y, AG_Color C)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->putPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    C);
}

/* Write a pixel (32-bit videoFmt argument) */
static __inline__ void
AG_PutPixel32(void *obj, int x, int y, Uint32 c)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->putPixel32(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    c);
}

/* Write a pixel (RGB arguments) */
static __inline__ void
AG_PutPixelRGB(void *obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->putPixelRGB(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r,g,b);
}

/* Blend a pixel (AG_Color argument) */
static __inline__ void
AG_BlendPixel(void *obj, int x, int y, AG_Color C, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    C, fnSrc, AG_ALPHA_ZERO);
}

/* Blend a pixel (32-bit agSurfaceFmt argument) */
static __inline__ void
AG_BlendPixel32(void *obj, int x, int y, Uint32 px, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color C = AG_GetColorRGBA(px, agSurfaceFmt);

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    C, fnSrc, AG_ALPHA_ZERO);
}

/* Blend a pixel (RGBA arguments) */
static __inline__ void
AG_BlendPixelRGBA(void *obj, int x, int y, Uint8 c[4], AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    AG_ColorRGBA(c[0],c[1],c[2],c[3]),
	    fnSrc, AG_ALPHA_ZERO);
}

/* Render a line from two endpoints. */
static __inline__ void
AG_DrawLine(void *obj, int x1, int y1, int x2, int y2, AG_Color C)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLine(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.y1 + y1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y2,
	    C);
}

/* Render a horizontal line. */
static __inline__ void
AG_DrawLineH(void *obj, int x1, int x2, int y, AG_Color C)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineH(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y,
	    C);
}

/* Render a vertical line. */
static __inline__ void
AG_DrawLineV(void *obj, int x, int y1, int y2, AG_Color C)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineV(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y1,
	    wid->rView.y1 + y2,
	    C);
}

/* Render a line with blending. */
static __inline__ void
AG_DrawLineBlended(void *obj, int x1, int y1, int x2, int y2, AG_Color C,
    AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineBlended(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.y1 + y1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y2,
	    C, fnSrc, AG_ALPHA_ZERO);
}

/* Render an arrow pointing up. */
static __inline__ void
AG_DrawArrowUp(void *obj, int x0, int y0, int h, AG_Color c1, AG_Color c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[2]; 

	c[0] = c1;
	c[1] = c2;
	wid->drvOps->drawArrowUp(wid->drv,
	    wid->rView.x1 + x0,
	    wid->rView.y1 + y0,
	    h, c);
}

/* Render an arrow pointing down. */
static __inline__ void
AG_DrawArrowDown(void *obj, int x0, int y0, int h, AG_Color c1, AG_Color c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[2];

	c[0] = c1;
	c[1] = c2;
	wid->drvOps->drawArrowDown(wid->drv,
	    wid->rView.x1 + x0,
	    wid->rView.y1 + y0,
	    h, c);
}

/* Render an arrow pointing left. */
static __inline__ void
AG_DrawArrowLeft(void *obj, int x0, int y0, int h, AG_Color c1, AG_Color c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[2];

	c[0] = c1;
	c[1] = c2;
	wid->drvOps->drawArrowLeft(wid->drv,
	    wid->rView.x1 + x0,
	    wid->rView.y1 + y0,
	    h, c);
}

/* Render an arrow pointing right. */
static __inline__ void
AG_DrawArrowRight(void *obj, int x0, int y0, int h, AG_Color c1, AG_Color c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[2];

	c[0] = c1;
	c[1] = c2;
	wid->drvOps->drawArrowRight(wid->drv,
	    wid->rView.x1 + x0,
	    wid->rView.y1 + y0,
	    h, c);
}

/* Render a 3D-style box with rounded edges. */
static __inline__ void
AG_DrawBoxRounded(void *obj, AG_Rect r, int z, int rad, AG_Color cBg)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[3];
	
	AG_WidgetOffsetRect(wid, &r);
	
	c[0] = AG_ColorShift(cBg, (z<0) ? agSunkColorShift : agRaisedColorShift);
	c[1] = AG_ColorShift(c[0], (z<0) ? agLowColorShift : agHighColorShift);
	c[2] = AG_ColorShift(c[0], (z<0) ? agHighColorShift : agLowColorShift);
	wid->drvOps->drawBoxRounded(wid->drv, r, z, rad, c);
}

/* Render a 3D-style box with rounded top edges. */
static __inline__ void
AG_DrawBoxRoundedTop(void *obj, AG_Rect r, int z, int rad, AG_Color cBg)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[3];

	AG_WidgetOffsetRect(wid, &r);
	c[0] = cBg;
	c[1] = AG_ColorShift(c[0], (z<0)?agLowColorShift:agHighColorShift);
	c[2] = AG_ColorShift(c[0], (z<0)?agHighColorShift:agLowColorShift);
	wid->drvOps->drawBoxRoundedTop(wid->drv, r, z, rad, c);
}

/* Render a circle of specified radius. */
static __inline__ void
AG_DrawCircle(void *obj, int x, int y, int r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->drawCircle(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r, c);
}

/* Render a circle of specified radius. */
static __inline__ void
AG_DrawCircleFilled(void *obj, int x, int y, int r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->drawCircleFilled(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r, c);
}

/* Render a filled rectangle (opaque or transparent). */
static __inline__ void
AG_DrawRect(void *obj, AG_Rect r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	AG_WidgetOffsetRect(wid, &r);
	if (c.a < AG_ALPHA_OPAQUE) {
		wid->drvOps->drawRectBlended(wid->drv, r, c,
		    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	} else {
		wid->drvOps->drawRectFilled(wid->drv, r, c);
	}
}

/* Render a filled rectangle (opaque). */
static __inline__ void
AG_DrawRectFilled(void *obj, AG_Rect r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;

	AG_WidgetOffsetRect(wid, &r);
	wid->drvOps->drawRectFilled(wid->drv, r, c);
}

/* Render a filled rectangle (transparent). */
static __inline__ void
AG_DrawRectBlended(void *obj, AG_Rect r, AG_Color c, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	AG_WidgetOffsetRect(wid, &r);
	wid->drvOps->drawRectBlended(wid->drv, r, c,
	    fnSrc, AG_ALPHA_ONE_MINUS_SRC);
}

/* Render a filled rectangle with dithering. */
static __inline__ void
AG_DrawRectDithered(void *obj, AG_Rect r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	AG_WidgetOffsetRect(wid, &r);
	wid->drvOps->drawRectDithered(wid->drv, r, c);
}


/* Render a 3D-style frame. */
static __inline__ void
AG_DrawFrame(void *obj, AG_Rect r, int z, AG_Color cBase)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	AG_Color c[2];
	int y2, x2;

	AG_WidgetOffsetRect(wid, &r);
	c[0] = AG_ColorShift(cBase, (z<0)?agLowColorShift:agHighColorShift);
	c[1] = AG_ColorShift(cBase, (z<0)?agHighColorShift:agLowColorShift);
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;

	if (c[0].a < AG_ALPHA_OPAQUE) {
		drvOps->drawLineBlended(drv, r.x, r.y, x2,  r.y, c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, r.x, r.y, r.x, y2,  c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	} else {
		drvOps->drawLineH(drv, r.x, x2,  r.y, c[0]);
		drvOps->drawLineV(drv, r.x, r.y, y2,  c[0]);
	}
	if (c[1].a < AG_ALPHA_OPAQUE) {
		drvOps->drawLineBlended(drv, r.x, y2,  x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, x2,  r.y, x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	} else {
		drvOps->drawLineH(drv, r.x, x2,  y2, c[1]);
		drvOps->drawLineV(drv, x2,  r.y, y2, c[1]);
	}
}

/*
 * Miscellaneous, utility rendering routines.
 */

/* Render a 3D-style box. */
static __inline__ void
AG_DrawBox(void *obj, AG_Rect r, int z, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_Rect rOffs;

	c = AG_ColorShift(c, (z < 0) ? agSunkColorShift : agRaisedColorShift);
	rOffs = r;
	AG_WidgetOffsetRect(wid, &rOffs);
	if (c.a < AG_ALPHA_OPAQUE) {
		wid->drvOps->drawRectBlended(drv, rOffs, c,
		    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	} else {
		wid->drvOps->drawRectFilled(drv, rOffs, c);
	}
	AG_DrawFrame(wid, r, z, c);
}

/* Render a 3D-style box with disabled control-style dithering. */
static __inline__ void
AG_DrawBoxDisabled(void *obj, AG_Rect r, int z, AG_Color cBox, AG_Color cDither)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Rect rOffs;

	cDither = AG_ColorShift(cDither, (z < 0) ? agSunkColorShift : agRaisedColorShift);
	rOffs = r;
	AG_WidgetOffsetRect(wid, &rOffs);
	wid->drvOps->drawRectFilled(wid->drv, rOffs, cBox);
	AG_DrawFrame(wid, r, z, cBox);
	wid->drvOps->drawRectDithered(wid->drv, rOffs, cDither);
}

/* Render 3D-style frame using a specific blending mode. */
static __inline__ void
AG_DrawFrameBlended(void *obj, AG_Rect r, AG_Color C, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	int x2, y2;

	AG_WidgetOffsetRect(wid, &r);
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;
	drvOps->drawLineBlended(drv, r.x, r.y, x2,  r.y, C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, r.x, r.y, r.x, y2,  C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, r.x, y2,  x2,  y2,  C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, x2,  r.y, x2,  y2,  C, fnSrc, AG_ALPHA_ZERO);
}

/* Render a rectangle outline. */
static __inline__ void
AG_DrawRectOutline(void *obj, AG_Rect r, AG_Color c)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	int x2, y2;

	AG_WidgetOffsetRect(wid, &r);
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;
	if (c.a < AG_ALPHA_OPAQUE) {
		drvOps->drawLineBlended(drv, r.x, r.y, x2,  r.y, c, AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, r.x, r.y, x2,  y2,  c, AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, r.x, r.y, r.x, y2,  c, AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, x2,  r.y, r.x, y2,  c, AG_ALPHA_SRC, AG_ALPHA_ZERO);
	} else {
		drvOps->drawLineH(drv, r.x, x2,  r.y, c);
		drvOps->drawLineH(drv, r.x, x2,  y2,  c);
		drvOps->drawLineV(drv, r.x, r.y, y2,  c);
		drvOps->drawLineV(drv, x2,  r.y, y2,  c);
	}
}

/* Render a [+] sign. */
static __inline__ void
AG_DrawPlus(void *obj, AG_Rect r, AG_Color C, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	int x1, y1;

	AG_WidgetOffsetRect(wid, &r);
	x1 = r.x + r.w/2;
	y1 = r.y + r.h/2;
	wid->drvOps->drawLineBlended(drv, x1,  r.y, x1,      r.y+r.h, C, fnSrc, AG_ALPHA_ZERO);
	wid->drvOps->drawLineBlended(drv, r.x, y1,  r.x+r.w, y1,      C, fnSrc, AG_ALPHA_ZERO);
}

/* Render a [-] sign. */
static __inline__ void
AG_DrawMinus(void *obj, AG_Rect r, AG_Color C, AG_BlendFn fnSrc)
{
	AG_Widget *wid = (AG_Widget *)obj;
	int x, y;

	AG_WidgetOffsetRect(wid, &r);
	x = r.x + r.w/2;
	y = r.y + r.h/2;
	wid->drvOps->drawLineBlended(wid->drv, x,y, r.x+r.w, y, C, fnSrc, AG_ALPHA_ZERO);
}

/* Render a 3D-style line. */
static __inline__ void
AG_DrawLine2(void *obj, int x1, int y1, int x2, int y2, AG_Color color)
{
	AG_Widget *wid = (AG_Widget *)obj;

	x1 += wid->rView.x1;
	y1 += wid->rView.y1;
	x2 += wid->rView.x1;
	y2 += wid->rView.y1;
	wid->drvOps->drawLine(wid->drv, x1, y1, x2, y2,
	    AG_ColorShift(color, agHighColorShift));
	wid->drvOps->drawLine(wid->drv, x1+1, y1+1, x2+1, y2+1,
	    AG_ColorShift(color, agLowColorShift));
}

/* Render a gimp-style background tiling. */
static __inline__ void
AG_DrawTiling(void *obj, AG_Rect r, int tsz, int offs, AG_Color c1, AG_Color c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	int alt1 = 0, alt2 = 0;
	AG_Rect rt;

	AG_WidgetOffsetRect(wid, &r);

	rt.w = tsz;
	rt.h = tsz;

	/* XXX inelegant */
	for (rt.y = r.y-tsz+offs;
	     rt.y < r.y+r.h;
	     rt.y += tsz) {
		for (rt.x = r.x-tsz+offs;
		     rt.x < r.x+r.w;
		     rt.x += tsz) {
			if (alt1++ == 1) {
				wid->drvOps->drawRectFilled(drv, rt, c1);
				alt1 = 0;
			} else {
				wid->drvOps->drawRectFilled(drv, rt, c2);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_GUI_PRIMITIVE_H_ */
