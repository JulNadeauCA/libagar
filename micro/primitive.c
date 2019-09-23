/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <agar/core/core.h>
#include <agar/micro/primitive.h>

/* Put pixel at x,y from a Color argument. */
void
AG_PutPixel(void *obj, Uint8 x, Uint8 y, const MA_Color *c)
{
	maDriverOps->putPixel(WIDGET(obj), x,y, c);
}

/* Put pixel at x,y from an explicit 4-bit value. */
void
AG_PutPixel_8(void *obj, Uint8 x, Uint8 y, Uint8 px)
{
	maDriverOps->putPixel8(WIDGET(obj), x,y, px);
}

/* Blend pixel at x,y against color c. */
void
AG_BlendPixel(void *obj, Uint8 x, Uint8 y, const MA_Color *c)
{
	maDriverOps->blendPixel(WIDGET(obj), x,y, c);
}

/* Line between two endpoints (x1,y1) and (x2,y2) inclusive. */
void
AG_DrawLine(void *obj, Uint8 x1, Uint8 y1, Uint8 x2, Uint8 y2, const MA_Color *c)
{
	maDriverOps->drawLine(WIDGET(obj), x1,y1, x2,y2, c);
}

/* Horizontal line between (x1,y) and (x2,y) inclusive. */
void
AG_DrawLineH(void *obj, Uint8 x1, Uint8 x2, Uint8 y, const MA_Color *c)
{
	maDriverOps->drawLineH(WIDGET(obj), x1,x2, y, c);
}

/* Vertical line between (x,y1) and (x,y2) inclusive. */
void
AG_DrawLineV(void *obj, Uint8 x, Uint8 y1, Uint8 y2, const MA_Color *c)
{
	maDriverOps->drawLineV(WIDGET(obj), x, y1,y2, c);
}

/*
 * Filled polygon with nPts points with 4-bit coordinates (XXXXYYYY)*2.
 */
void
AG_DrawPolygon(void *obj, const Uint8 *pts, Uint8 nPts, const MA_Color *c)
{
	maDriverOps->drawPolygon(WIDGET(obj), pts, nPts, c);
}

/* Circle outline of radius r (inclusive) centered around (x,y). */
void
AG_DrawCircle(void *obj, Uint8 x, Uint8 y, Uint8 r, const MA_Color *c)
{
	maDriverOps->drawCircle(WIDGET(obj), x,y, r, c);
}

/* Filled circle of radius r (inclusive) centered around (x,y). */
void
AG_DrawCircleFilled(void *obj, Uint8 x, Uint8 y, Uint8 r, const MA_Color *c)
{
	maDriverOps->drawCircleFilled(WIDGET(obj), x,y, r, c);
}

/* Rectangle outline around r inclusive. */
void
AG_DrawRect(void *obj, const MA_Rect *r, const MA_Color *c)
{
	maDriverOps->drawRectFilled(WIDGET(obj), r, c);
}

/* Filled rectangle around r inclusive. */
void
AG_DrawRectFilled(void *obj, const MA_Rect *r, const MA_Color *c)
{
	maDriverOps->drawRectFilled(WIDGET(obj), r, c);
}

/* 3D-style box of depth z around a rectangle r. */
void
AG_DrawBox(void *obj, const MA_Rect *r, Sint8 z, const MA_Color *c)
{
	MA_Color cRaisedOrSunk;
	
	MA_ColorAdd(&cRaisedOrSunk, c, (z < 0) ? &maSunkColor : &maRaisedColor);
	maDriverOps->drawRectFilled(obj, r, &cRaisedOrSunk);

	/* XXX overdraw */
	AG_DrawFrame(obj, r, z, c);
}

/* 3d-style frame of depth z around a rectangle r. */
void
AG_DrawFrame(void *obj, const MA_Rect *r, Sint8 z, const MA_Color *cBase)
{
	MA_Widget *wid = WIDGET(obj);
	MA_Color c[2];
	Uint8 y2, x2;
	
	MA_ColorAdd(&c[0], cBase, (z<0) ? &maLowColor  : &maHighColor);
	MA_ColorAdd(&c[1], cBase, (z<0) ? &maHighColor : &maLowColor);

	x2 = r->x + r->w - 1;
	y2 = r->y + r->h - 1;

	maDriverOps->drawLineH(wid, r->x, x2,   r->y, &c[0]);
	maDriverOps->drawLineV(wid, r->x, r->y, y2,   &c[0]);
	maDriverOps->drawLineH(wid, r->x, x2,   y2,   &c[1]);
	maDriverOps->drawLineV(wid, x2,   r->y, y2,   &c[1]);
}

/* 3d-style box of depth z with all 4 rounded corners around rectangle r. */
void
AG_DrawBoxRounded(void *obj, const MA_Rect *r, Sint8 z, Uint8 radius,
    const MA_Color *c)
{
	maDriverOps->drawBoxRounded(WIDGET(obj), r, z, radius, c);
}

/* 3d-style box of depth z with rounded top two corners around a rectangle r. */
void
AG_DrawBoxRoundedTop(void *obj, const MA_Rect *r, Sint8 z, Uint8 radius,
    const MA_Color *c)
{
	maDriverOps->drawBoxRoundedTop(WIDGET(obj), r, z, radius, c);
}
