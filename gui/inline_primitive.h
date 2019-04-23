/*	Public domain	*/

/*
 * Write a pixel (AG_Color argument)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixel(void *_Nonnull obj, int x, int y, AG_Color C)
#else
void
ag_put_pixel(void *obj, int x, int y, AG_Color C)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->putPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    C);
}

/*
 * Write a pixel (32-bit value argument in agSurfaceFmt format)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixel32(void *_Nonnull obj, int x, int y, Uint32 c)
#else
void
ag_put_pixel_32(void *obj, int x, int y, Uint32 c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->putPixel32(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    c);
}

/*
 * Write a pixel (8-bit RGB component arguments)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixelRGB_8(void *_Nonnull obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
#else
void
ag_put_pixel_rgb_8(void *obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->putPixelRGB8(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r,g,b);
}

/*
 * Blend a pixel (AG_Color argument)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_BlendPixel(void *_Nonnull obj, int x, int y, AG_Color C, AG_AlphaFn fnSrc)
#else
void
ag_blend_pixel(void *obj, int x, int y, AG_Color C, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    C, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Blend a pixel (32-bit value argument in agSurfaceFmt format)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_BlendPixel32(void *_Nonnull obj, int x, int y, Uint32 px, AG_AlphaFn fnSrc)
#else
void
ag_blend_pixel_32(void *obj, int x, int y, Uint32 px, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c = AG_GetColor32(px, agSurfaceFmt);

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    c, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Blend a pixel (RGBA arguments)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_BlendPixelRGBA(void *_Nonnull obj, int x, int y, Uint8 c[_Nonnull 4],
    AG_AlphaFn fnSrc)
#else
void
ag_blend_pixel_rgba(void *obj, int x, int y, Uint8 c[4], AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    AG_ColorRGBA(c[0],c[1],c[2],c[3]),
	    fnSrc, AG_ALPHA_ZERO);
}

#if AG_MODEL == AG_LARGE					/* LG */

/*
 * Write a pixel (16-bit RGB components).
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixelRGB(void *_Nonnull obj, int x, int y,
    AG_Component r, AG_Component g, AG_Component b)
# else
void
ag_put_pixel_rgb(void *obj, int x, int y,
    AG_Component r, AG_Component g, AG_Component b)
# endif
{
	AG_PutPixelRGB_16(obj, x,y, r,g,b);
}

/*
 * Write a pixel (64-bit value argument in agSurfaceFmt format)
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixel64(void *_Nonnull obj, int x, int y, Uint64 px)
# else
void
ag_put_pixel_64(void *obj, int x, int y, Uint64 px)
# endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->putPixel64(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    px);
}

/*
 * Write a pixel (16-bit RGB component arguments)
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixelRGB_16(void *_Nonnull obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
# else
void
ag_put_pixel_rgb_16(void *obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
# endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->putPixelRGB16(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r,g,b);
}

/*
 * Blend a pixel (64-bit value argument in agSurfaceFmt format)
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_BlendPixel64(void *_Nonnull obj, int x, int y, Uint64 px, AG_AlphaFn fnSrc)
# else
void
ag_blend_pixel_64(void *obj, int x, int y, Uint64 px, AG_AlphaFn fnSrc)
# endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c = AG_GetColor32(px, agSurfaceFmt);

	wid->drvOps->blendPixel(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    c, fnSrc, AG_ALPHA_ZERO);
}

#else								/* !LG */

/*
 * Write a pixel (8-bit RGB components).
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_PutPixelRGB(void *_Nonnull obj, int x, int y,
    AG_Component r, AG_Component g, AG_Component b)
# else
void
ag_put_pixel_rgb(void *obj, int x, int y,
    AG_Component r, AG_Component g, AG_Component b)
# endif
{
	AG_PutPixelRGB_8(obj, x,y, r,g,b);
}

#endif /* !AG_LARGE */

/*
 * Draw a line between two given endpoints.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawLine(void *_Nonnull obj, int x1, int y1, int x2, int y2, AG_Color C)
#else
void
ag_draw_line(void *obj, int x1, int y1, int x2, int y2, AG_Color C)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLine(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.y1 + y1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y2,
	    C);
}

/*
 * Draw a horizontal line.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawLineH(void *_Nonnull obj, int x1, int x2, int y, AG_Color C)
#else
void
ag_draw_line_h(void *obj, int x1, int x2, int y, AG_Color C)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineH(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y,
	    C);
}

/*
 * Draw a vertical line.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawLineV(void *_Nonnull obj, int x, int y1, int y2, AG_Color C)
#else
void
ag_draw_line_v(void *obj, int x, int y1, int y2, AG_Color C)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineV(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y1,
	    wid->rView.y1 + y2,
	    C);
}

/*
 * Draw a line with blending.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawLineBlended(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    AG_Color C, AG_AlphaFn fnSrc)
#else
void
ag_draw_line_blended(void *obj, int x1, int y1, int x2, int y2, AG_Color C,
    AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawLineBlended(wid->drv,
	    wid->rView.x1 + x1,
	    wid->rView.y1 + y1,
	    wid->rView.x1 + x2,
	    wid->rView.y1 + y2,
	    C, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Render a triangle between three given points.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawTriangle(void *_Nonnull obj, AG_Pt v1, AG_Pt v2, AG_Pt v3, AG_Color c)
#else
void
ag_draw_triangle(void *obj, AG_Pt v1, AG_Pt v2, AG_Pt v3, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	v1.x += wid->rView.x1; v1.y += wid->rView.y1;
	v2.x += wid->rView.x1; v2.y += wid->rView.y1;
	v3.x += wid->rView.x1; v3.y += wid->rView.y1;

	wid->drvOps->drawTriangle(wid->drv, v1,v2,v3, c);
}

/*
 * Render an arrow pointing north.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawArrowUp(void *_Nonnull obj, int x0, int y0, int h, AG_Color c)
#else
void
ag_draw_arrow_up(void *obj, int x0, int y0, int h, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawArrow(wid->drv, 0,
	    wid->rView.x1+x0,
	    wid->rView.y1+y0, h, c);
}

/*
 * Render an arrow pointing east.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawArrowRight(void *_Nonnull obj, int x0, int y0, int h, AG_Color c)
#else
void
ag_draw_arrow_right(void *obj, int x0, int y0, int h, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawArrow(wid->drv, 1,
	    wid->rView.x1+x0,
	    wid->rView.y1+y0, h, c);
}

/*
 * Render an arrow pointing south.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawArrowDown(void *_Nonnull obj, int x0, int y0, int h, AG_Color c)
#else
void
ag_draw_arrow_down(void *obj, int x0, int y0, int h, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawArrow(wid->drv, 2,
	    wid->rView.x1+x0,
	    wid->rView.y1+y0, h, c);
}

/*
 * Render an arrow pointing west.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawArrowLeft(void *_Nonnull obj, int x0, int y0, int h, AG_Color c)
#else
void
ag_draw_arrow_left(void *obj, int x0, int y0, int h, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->drawArrow(wid->drv, 3,
	    wid->rView.x1+x0,
	    wid->rView.y1+y0, h, c);
}

/*
 * Render a 3D-style box with all rounded edges
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawBoxRounded(void *_Nonnull obj, AG_Rect r, int z, int rad, AG_Color cBg)
#else
void
ag_draw_box_rounded(void *obj, AG_Rect r, int z, int rad, AG_Color cBg)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[3];
	
	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	c[0] = AG_ColorAdd(cBg,  (z<0) ? agSunkColor : agRaisedColor);
	c[1] = AG_ColorAdd(c[0], (z<0) ? agLowColor  : agHighColor);
	c[2] = AG_ColorAdd(c[0], (z<0) ? agHighColor : agLowColor);
	wid->drvOps->drawBoxRounded(wid->drv, r, z, rad, c[0], c[1], c[2]);
}

/*
 * Render a 3D-style box with only the two top edges rounded.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawBoxRoundedTop(void *_Nonnull obj, AG_Rect r, int z, int rad, AG_Color cBg)
#else
void
ag_draw_box_rounded_top(void *obj, AG_Rect r, int z, int rad, AG_Color cBg)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Color c[3];

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	c[0] = cBg;
	c[1] = AG_ColorAdd(c[0], (z<0) ? agLowColor  : agHighColor);
	c[2] = AG_ColorAdd(c[0], (z<0) ? agHighColor : agLowColor);
	wid->drvOps->drawBoxRoundedTop(wid->drv, r, z, rad, c[0], c[1], c[2]);
}

/*
 * Draw a circle outline.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawCircle(void *_Nonnull obj, int x, int y, int r, AG_Color c)
#else
void
ag_draw_circle(void *obj, int x, int y, int r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->drawCircle(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r, c);
}

/*
 * Draw a filled circle.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawCircleFilled(void *_Nonnull obj, int x, int y, int r, AG_Color c)
#else
void
ag_draw_circle_filled(void *obj, int x, int y, int r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->drawCircleFilled(wid->drv,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y,
	    r, c);
}

/*
 * Draw a filled rectangle (possibly blended)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawRect(void *_Nonnull obj, AG_Rect r, AG_Color c)
#else
void
ag_draw_rect(void *obj, AG_Rect r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	if (c.a < AG_OPAQUE) {
		wid->drvOps->drawRectBlended(wid->drv, r, c,
		    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	} else {
		wid->drvOps->drawRectFilled(wid->drv, r, c);
	}
}

/*
 * Draw a filled rectangle (always opaque).
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawRectFilled(void *_Nonnull obj, AG_Rect r, AG_Color c)
#else
void
ag_draw_rect_filled(void *obj, AG_Rect r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	wid->drvOps->drawRectFilled(wid->drv, r, c);
}

/*
 * Draw a filled rectangle with blending (and given source blending function)
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawRectBlended(void *_Nonnull obj, AG_Rect r, AG_Color c, AG_AlphaFn fnSrc)
#else
void
ag_draw_rect_blended(void *obj, AG_Rect r, AG_Color c, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	wid->drvOps->drawRectBlended(wid->drv, r, c,
	    fnSrc, AG_ALPHA_ONE_MINUS_SRC);
}

/*
 * Render a filled rectangle with stipple pattern.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawRectDithered(void *_Nonnull obj, AG_Rect r, AG_Color c)
#else
void
ag_draw_rect_dithered(void *obj, AG_Rect r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	wid->drvOps->drawRectDithered(wid->drv, r, c);
}

/*
 * Render a 3D-style frame
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawFrame(void *_Nonnull obj, AG_Rect r, int z, AG_Color cBase)
#else
void
ag_draw_frame(void *obj, AG_Rect r, int z, AG_Color cBase)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	AG_Color c[2];
	int y2, x2;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	c[0] = AG_ColorAdd(cBase, (z<0) ? agLowColor  : agHighColor);
	c[1] = AG_ColorAdd(cBase, (z<0) ? agHighColor : agLowColor);
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;

	if (c[0].a < AG_OPAQUE) {
		drvOps->drawLineBlended(drv, r.x, r.y, x2,  r.y, c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, r.x, r.y, r.x, y2,  c[0], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	} else {
		drvOps->drawLineH(drv, r.x, x2,  r.y, c[0]);
		drvOps->drawLineV(drv, r.x, r.y, y2,  c[0]);
	}
	if (c[1].a < AG_OPAQUE) {
		drvOps->drawLineBlended(drv, r.x, y2,  x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
		drvOps->drawLineBlended(drv, x2,  r.y, x2, y2, c[1], AG_ALPHA_SRC, AG_ALPHA_ZERO);
	} else {
		drvOps->drawLineH(drv, r.x, x2,  y2, c[1]);
		drvOps->drawLineV(drv, x2,  r.y, y2, c[1]);
	}
}

/*
 * Render a 3D-style box
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawBox(void *_Nonnull obj, AG_Rect r, int z, AG_Color c)
#else
void
ag_draw_box(void *obj, AG_Rect r, int z, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_Rect rOffs;

	c = AG_ColorAdd(c, (z < 0) ? agSunkColor : agRaisedColor);
	rOffs = r;
	rOffs.x += wid->rView.x1;
	rOffs.y += wid->rView.y1;
	if (c.a < AG_OPAQUE) {
		wid->drvOps->drawRectBlended(drv, rOffs, c,
		    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	} else {
		wid->drvOps->drawRectFilled(drv, rOffs, c);
	}
	AG_DrawFrame(wid, r, z, c);
}

/*
 * Render a textured 3D-style box in "disabled" style.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawBoxDisabled(void *_Nonnull obj, AG_Rect r, int z, AG_Color cBox,
    AG_Color cDither)
#else
void
ag_draw_box_disabled(void *obj, AG_Rect r, int z, AG_Color cBox, AG_Color cDither)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Rect rOffs;

	cDither = AG_ColorAdd(cDither, (z<0) ? agSunkColor : agRaisedColor);
	rOffs = r;
	rOffs.x += wid->rView.x1;
	rOffs.y += wid->rView.y1;
	wid->drvOps->drawRectFilled(wid->drv, rOffs, cBox);
	AG_DrawFrame(wid, r, z, cBox);
	wid->drvOps->drawRectDithered(wid->drv, rOffs, cDither);
}

/*
 * Render 3D-style frame using a specific blending mode.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawFrameBlended(void *_Nonnull obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#else
void
ag_draw_frame_blended(void *obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	int x2, y2;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;
	drvOps->drawLineBlended(drv, r.x, r.y, x2,  r.y, C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, r.x, r.y, r.x, y2,  C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, r.x, y2,  x2,  y2,  C, fnSrc, AG_ALPHA_ZERO);
	drvOps->drawLineBlended(drv, x2,  r.y, x2,  y2,  C, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Draw a rectangle outline only.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawRectOutline(void *_Nonnull obj, AG_Rect r, AG_Color c)
#else
void
ag_draw_rect_outline(void *obj, AG_Rect r, AG_Color c)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;
	int x2, y2;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	x2 = r.x+r.w - 1;
	y2 = r.y+r.h - 1;
	if (c.a < AG_OPAQUE) {
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

/*
 * Render a plus `+' sign.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawPlus(void *_Nonnull obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#else
void
ag_draw_plus(void *obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	int x1, y1;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	x1 = r.x + (r.w >> 1);
	y1 = r.y + (r.h >> 1);
	wid->drvOps->drawLineBlended(drv, x1,  r.y, x1,      r.y+r.h, C, fnSrc, AG_ALPHA_ZERO);
	wid->drvOps->drawLineBlended(drv, r.x, y1,  r.x+r.w, y1,      C, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Render a minus `-' sign.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawMinus(void *_Nonnull obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#else
void
ag_draw_minus(void *obj, AG_Rect r, AG_Color C, AG_AlphaFn fnSrc)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;
	int x, y;

	r.x += wid->rView.x1;
	r.y += wid->rView.y1;
	x = r.x + (r.w >> 1);
	y = r.y + (r.h >> 1);
	wid->drvOps->drawLineBlended(wid->drv, x,y, r.x+r.w, y, C, fnSrc, AG_ALPHA_ZERO);
}

/*
 * Render a 3D-style line.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_DrawLine2(void *_Nonnull obj, int x1, int y1, int x2, int y2, AG_Color color)
#else
void
ag_draw_line_2(void *obj, int x1, int y1, int x2, int y2, AG_Color color)
#endif
{
	AG_Widget *wid = (AG_Widget *)obj;

	x1 += wid->rView.x1;
	y1 += wid->rView.y1;
	x2 += wid->rView.x1;
	y2 += wid->rView.y1;
	wid->drvOps->drawLine(wid->drv, x1,y1, x2,y2,
	    AG_ColorAdd(color, agHighColor));
	wid->drvOps->drawLine(wid->drv, x1+1,y1+1, x2+1,y2+1,
	    AG_ColorAdd(color, agLowColor));
}
