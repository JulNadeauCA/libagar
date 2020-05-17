/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
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

/*
 * A dummy (no-op) Micro-Agar driver backend.
 */

#include <agar/core/core.h>

#include <agar/micro/gui.h>
#include <agar/micro/drv.h>
#include <agar/micro/window.h>

/* Driver instance data */
typedef struct ma_driver_dummy {
	struct ma_driver _inherit;	/* MA_Driver -> MA_DriverDUMMY */
	Uint16 flags;			/* Some flags */
} MA_DriverDUMMY;

MA_DriverClass maDriverDUMMY;

#define MADRIVER_IS_DUMMY(drv) \
       (MADRIVER_CLASS(drv) == (MA_DriverClass *)&maDriverDUMMY)

static void
Init(void *obj)
{
	MA_DriverDUMMY *dum = obj;

	dum->flags = 0;
}

static Sint8
DUMMY_Open(const char *_Nullable spec)
{
/*	Debug(obj, "Open driver (%s)\n", (spec != NULL) ? spec : ""); */
	Debug(maDriver, "Open driver\n");
	(void) spec;
	return (0);
}

static void
DUMMY_Close(void)
{
	Debug(maDriver, "Close driver\n");
}

static void
DUMMY_GetDisplaySize(Uint16 *w, Uint16 *h)
{
	Debug(NULL, "GetDisplaySize (%p,%p)\n", w,h);
	*w = 320;
	*h = 240;
}

static void
DUMMY_BeginRendering(void)
{
	/* Begin rendering cycle. */
	Debug(maDriver, "BeginRendering\n");
}

static void
DUMMY_RenderWindow(MA_Window *win)
{
	Debug(win, "RenderWindow\n");
/*	MA_WidgetDraw(win); */
}

static void
DUMMY_EndRendering(void)
{
	/* Finalize rendering cycle */
	Debug(maDriver, "EndRendering\n");
}

static void
DUMMY_BlitSurface(MA_Widget *obj, const MA_Surface *su, Uint8 x, Uint8 y)
{
	Debug(obj, "BlitSurface(%p -> %u,%u)\n", su, x,y);
}

static void
DUMMY_BlitSurfaceFrom(MA_Widget *obj, Uint8 suIdx, const MA_Rect *r,
                      Uint8 x, Uint8 y)
{
	if (r) {
		Debug(obj, "BlitSurfaceFrom(#%u [%u,%u:%u,%u] -> %u,%u)\n",
		      suIdx, x,y, r->x, r->y, r->w, r->h);
	} else {
		Debug(obj, "BlitSurfaceFrom(#%u -> %u,%u)\n", suIdx, x,y);
	}
}

static void
DUMMY_FillRect(MA_Widget *obj, const MA_Rect *r, const MA_Color *c)
{
	Debug(obj, "FillRect(%d,%d:%dx%d, %x:%x:%x:%x)\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_PutPixel(MA_Widget *obj, Uint8 x, Uint8 y, const MA_Color *c)
{
	Debug(obj, "PutPixel(%u,%u, [%x:%x:%x:%x])\n", x,y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_PutPixel_8(MA_Widget *obj, Uint8 x, Uint8 y, Uint8 px)
{
	Debug(obj, "PutPixel(%u,%u, $%x)\n", x,y, px);
}

static void
DUMMY_BlendPixel(MA_Widget *obj, Uint8 x, Uint8 y, const MA_Color *c)
{
	Debug(obj, "BlendPixel(%d,%d, [%x:%x:%x:%x])\n", x,y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLine(MA_Widget *obj, Uint8 x1, Uint8 y1, Uint8 x2, Uint8 y2,
               const MA_Color *c)
{
	Debug(obj, "DrawLine([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x1,y1, x2,y2,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLineH(MA_Widget *obj, Uint8 x1, Uint8 x2, Uint8 y, const MA_Color *c)
{
	Debug(obj, "DrawLineH([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x1,y, x2,y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLineV(MA_Widget *obj, Uint8 x, Uint8 y1, Uint8 y2, const MA_Color *c)
{
	Debug(obj, "DrawLineV([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x,y1, x,y2,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawPolygon(MA_Widget *obj, const Uint8 *pts, Uint8 nPts, const MA_Color *c)
{
#ifdef AG_DEBUG
	Uint8 i;

	Debug(obj, "DrawPolygon(%u, [%x:%x:%x:%x],", nPts,
	      c->r, c->g, c->b, c->a);

	for (i = 0; i < nPts; i++) {
		Debug(NULL, " (%u,%u)",
		    ((pts[i] & 0xf0) >> 4) << 1,
		     (pts[i] & 0x0f)       << 1;
	}
	Debug(NULL, ")\n");
#else
	(void) obj;
	(void) pts;
	(void) nPts;
	(void) c;
#endif
}

static void
DUMMY_DrawBox(MA_Widget *obj, const MA_Rect *r, Uint8 x, Uint8 y, const MA_Color *c)
{
	(void) obj;
	(void) r;
	(void) x;
	(void) y;
	(void) c;
}

static void
DUMMY_DrawBoxRounded(MA_Widget *obj, const MA_Rect *r, Sint8 z, Uint8 radius,
                     const MA_Color *c)
{
	(void) obj;
	(void) r;
	(void) z;
	(void) radius;
	(void) c;
}

static void
DUMMY_DrawBoxRoundedTop(MA_Widget *obj, const MA_Rect *r, Sint8 z, Uint8 radius,
                        const MA_Color *c)
{
	(void) obj;
	(void) r;
	(void) z;
	(void) radius;
	(void) c;
}

static void
DUMMY_DrawCircle(MA_Widget *obj, Uint8 x, Uint8 y, Uint8 radius,
                 const MA_Color *c)
{
	Debug(obj, "DrawCircle(%d,%d, %d, [%x:%x:%x:%x])\n", x,y, radius,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawCircleFilled(MA_Widget *obj, Uint8 x, Uint8 y, Uint8 radius,
                       const MA_Color *c)
{
	Debug(obj, "DrawCircleFilled(%d,%d, %d, [%x:%x:%x:%x])\n", x,y, radius,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawRectFilled(MA_Widget *obj, const MA_Rect *r, const MA_Color *c)
{
	Debug(obj, "DrawRectFilled([%d,%d %dx%d], [%x:%x:%x:%x])\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a);
}

MA_DriverClass maDriverDUMMY = {
	{
		"MA_Driver:MA_DriverDUMMY",
		sizeof(MA_DriverDUMMY),
		{ 1,6 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL,		/* edit */
	},
	"dummy",
	0,			/* flags */
	DUMMY_Open,
	DUMMY_Close,
	DUMMY_GetDisplaySize,
	DUMMY_BeginRendering,
	DUMMY_RenderWindow,
	DUMMY_EndRendering,
	DUMMY_BlitSurface,
	DUMMY_BlitSurfaceFrom,
	DUMMY_FillRect,
	DUMMY_PutPixel,
	DUMMY_PutPixel_8,
	DUMMY_BlendPixel,
	DUMMY_DrawLine,
	DUMMY_DrawLineH,
	DUMMY_DrawLineV,
	DUMMY_DrawPolygon,
	DUMMY_DrawBox,
	DUMMY_DrawBoxRounded,
	DUMMY_DrawBoxRoundedTop,
	DUMMY_DrawCircle,
	DUMMY_DrawCircleFilled,
	DUMMY_DrawRectFilled
};
