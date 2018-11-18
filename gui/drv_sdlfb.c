/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Driver for framebuffer graphics via the SDL 1.2 library.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl.h>

typedef struct ag_sdlfb_driver {
	struct ag_driver_sw _inherit;

	SDL_Surface *_Nullable s;	/* Display surface */
	SDL_Rect *_Nonnull dirty;	/* Video regions to update */
	Uint              nDirty;
	Uint            maxDirty;

	AG_ClipRect *clipRects;		/* Clipping rectangle stack */
	Uint        nClipRects;
} AG_DriverSDLFB;

static int nDrivers = 0;				/* Opened driver instances */
static int initedSDL = 0;				/* Used SDL_Init() */
static int initedSDLVideo = 0;				/* Used SDL_INIT_VIDEO */
static AG_EventSink *_Nullable sfbEventSpinner = NULL;	/* Standard event sink */
static AG_EventSink *_Nullable sfbEventEpilogue = NULL;	/* Standard event epilogue */

static void SDLFB_DrawRectFilled(void *_Nonnull, AG_Rect, AG_Color);
static void SDLFB_UpdateRegion(void *_Nonnull, AG_Rect);

static void
Init(void *_Nonnull obj)
{
	AG_DriverSDLFB *sfb = obj;

	sfb->s = NULL;
	sfb->nDirty = 0;
	sfb->dirty = Malloc(4*sizeof(SDL_Rect));
	sfb->maxDirty = 4;
	sfb->clipRects = NULL;
	sfb->nClipRects = 0;
}

static void
Destroy(void *_Nonnull obj)
{
	AG_DriverSDLFB *sfb = obj;

	Free(sfb->dirty);
	Free(sfb->clipRects);
}

/*
 * Generic driver operations
 */

static int
SDLFB_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverSDLFB *sfb = obj;
	
	if (nDrivers != 0) {
		AG_SetError("Multiple SDL displays are not supported");
		return (-1);
	}

	/* Initialize SDL's video subsystem. */
	if (!initedSDL) {
		if (SDL_Init(0) == -1) {
			AG_SetError("SDL_Init() failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDL = 1;
	}
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			AG_SetError("SDL_INIT_VIDEO failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDLVideo = 1;
	}
#if 0
	/* Use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);
	AG_DestroyEventSubsystem();
	AG_InitEventSubsystem(AG_SOFT_TIMERS);
#endif
	/* Initialize the main mouse and keyboard devices. */
	if ((drv->mouse = AG_MouseNew(sfb, "SDL mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(sfb, "SDL keyboard")) == NULL)
		goto fail;

	/* Configure the window caption */
	if (agProgName != NULL)
		SDL_WM_SetCaption(agProgName, agProgName);
	
	/*
	 * TODO where AG_SINK_READ capability and pipes are available,
	 * could we create a separate thread running SDL_WaitEvent() and
	 * sending notifications over a pipe, instead of using a spinner?
	 */
	if ((sfbEventSpinner = AG_AddEventSpinner(AG_SDL_EventSink, "%p", drv)) == NULL ||
	    (sfbEventEpilogue = AG_AddEventEpilogue(AG_SDL_EventEpilogue, NULL)) == NULL) {
		goto fail;
	}
	nDrivers = 1;
	return (0);
fail:
	if (sfbEventSpinner != NULL) { AG_DelEventSpinner(sfbEventSpinner); sfbEventSpinner = NULL; }
	if (sfbEventEpilogue != NULL) { AG_DelEventEpilogue(sfbEventEpilogue); sfbEventEpilogue = NULL; }
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
SDLFB_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverSDLFB *sfb = obj;
	
	AG_DelEventSpinner(sfbEventSpinner); sfbEventSpinner = NULL;
	AG_DelEventEpilogue(sfbEventEpilogue); sfbEventEpilogue = NULL;

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sfb));

	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDelete(drv->kbd); drv->kbd = NULL;
	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;

	nDrivers = 0;
}

static void
SDLFB_BeginRendering(void *_Nonnull obj)
{
//	AG_DriverSDLFB *sfb = obj;

//	SDL_LockSurface(sfb->s);
}

static void
SDLFB_RenderWindow(struct ag_window *_Nonnull win)
{
	AG_WidgetDraw(win);
	SDLFB_UpdateRegion(WIDGET(win)->drv,
	    AG_RECT(WIDGET(win)->x, WIDGET(win)->y,
	            WIDTH(win), HEIGHT(win)));
}

static void
SDLFB_EndRendering(void *_Nonnull obj)
{
	AG_DriverSDLFB *sfb = obj;

#ifdef AG_DEBUG
	if (sfb->nClipRects != 1)
		AG_FatalError("Inconsistent PushClipRect() / PopClipRect()");
#endif
	if (sfb->nDirty > 0) {
		SDL_UpdateRects(sfb->s, sfb->nDirty, sfb->dirty);
		sfb->nDirty = 0;
	}
//	SDL_UnlockSurface(sfb->s);
}

static void
SDLFB_FillRect(void *_Nonnull obj, AG_Rect r, AG_Color c)
{
	AG_DriverSDLFB *sfb = obj;
	SDL_Rect sr;

	sr.x = r.x;
	sr.y = r.y;
	sr.w = r.w;
	sr.h = r.h;
	SDL_FillRect(sfb->s, &sr,
	    SDL_MapRGB(sfb->s->format, c.r, c.g, c.b));
}

static void
SDLFB_UpdateRegion(void *_Nonnull obj, AG_Rect rp)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDLFB *sfb = obj;
	SDL_Rect *sr;
	AG_Rect2 r = AG_RectToRect2(rp);
	int n;

	/* TODO Compute intersections? */
	if (r.x1 < 0) { r.x1 = 0; }
	if (r.y1 < 0) { r.y1 = 0; }
	if (r.x2 > dsw->w) { r.x2 = dsw->w; r.w = r.x2-r.x1; }
	if (r.y2 > dsw->h) { r.y2 = dsw->h; r.h = r.y2-r.y1; }
	if (r.w < 0) { r.x1 = 0; r.x2 = r.w = dsw->w; }
	if (r.h < 0) { r.y1 = 0; r.y2 = r.h = dsw->h; }

	n = sfb->nDirty++;
	if (n+1 > sfb->maxDirty) {
		sfb->maxDirty *= 2;
		sfb->dirty = Realloc(sfb->dirty, sfb->maxDirty*sizeof(SDL_Rect));
	}
	sr = &sfb->dirty[n];
	sr->x = r.x1;
	sr->y = r.y1;
	sr->w = r.w;
	sr->h = r.h;
}

/*
 * Clipping and blending control (rendering context)
 */

static void
SDLFB_PushClipRect(void *_Nonnull obj, AG_Rect r)
{
	AG_DriverSDLFB *sfb = obj;
	AG_ClipRect *cr, *crPrev;
	SDL_Rect sr;

	sfb->clipRects = Realloc(sfb->clipRects, (sfb->nClipRects+1) *
	                                         sizeof(AG_ClipRect));
	crPrev = &sfb->clipRects[sfb->nClipRects-1];
	cr = &sfb->clipRects[sfb->nClipRects++];
	cr->r = AG_RectIntersect(crPrev->r, r);

	sr.x = (Sint16)cr->r.x;
	sr.y = (Sint16)cr->r.y;
	sr.w = (Uint16)cr->r.w;
	sr.h = (Uint16)cr->r.h;
	SDL_SetClipRect(sfb->s, &sr);
}

static void
SDLFB_PopClipRect(void *_Nonnull obj)
{
	AG_DriverSDLFB *sfb = obj;
	AG_ClipRect *cr;
	SDL_Rect sr;

#ifdef AG_DEBUG
	if (sfb->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &sfb->clipRects[sfb->nClipRects-2];
	sfb->nClipRects--;

	sr.x = (Sint16)cr->r.x;
	sr.y = (Sint16)cr->r.y;
	sr.w = (Uint16)cr->r.w;
	sr.h = (Uint16)cr->r.h;
	SDL_SetClipRect(sfb->s, &sr);
}

static void
SDLFB_PushBlendingMode(void *_Nonnull drv, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	/* No-op (handle blending on a per-blit basis) */
}
static void
SDLFB_PopBlendingMode(void *_Nonnull drv)
{
	/* No-op (handle blending on a per-blit basis) */
}

/*
 * Surface operations (rendering context)
 */

static void
SDLFB_BlitSurface(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull s, int x, int y)
{
	AG_DriverSDLFB *sfb = drv;

	AG_SDL_BlitSurface(s, NULL, sfb->s, x,y);
}

static void
SDLFB_BlitSurfaceFrom(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, const AG_Rect *_Nullable rSrc, int x, int y)
{
	AG_DriverSDLFB *sfb = drv;

	AG_SDL_BlitSurface(wid->surfaces[s], rSrc, sfb->s, x,y);
}

static void
SDLFB_BlitSurfaceGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull s, float w, float h)
{
	/* Not applicable */
}

static void
SDLFB_BlitSurfaceFromGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, float w, float h)
{
	/* Not applicable */
}

static void
SDLFB_BlitSurfaceFlippedGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, float w, float h)
{
	/* Not applicable */
}

static int
SDLFB_RenderToSurface(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull *_Nullable s)
{
	AG_DriverSDLFB *sfb = drv;
	int visiblePrev;
	SDL_Surface *sd;
	SDL_Rect sr;

	if ((sd = SDL_CreateRGBSurface(SDL_SWSURFACE, wid->w, wid->h, 32,
#if AG_BYTEORDER == AG_BIG_ENDIAN
 	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	)) == NULL) {
		return (-1);
	}

	/* XXX TODO render to offscreen buffer instead */
	AG_BeginRendering(AGDRIVER(sfb));
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering(AGDRIVER(sfb));

	sr.x = wid->rView.x1;
	sr.y = wid->rView.y1;
	sr.w = wid->w;
	sr.h = wid->h;
	SDL_BlitSurface(sfb->s, &sr, sd, NULL);
	if ((*s = AG_SDL_ImportSurface(sd)) == NULL) {
		SDL_FreeSurface(sd);
		return (-1);
	}
	SDL_FreeSurface(sd);
	return (0);
}

/*
 * Rendering operations (rendering context)
 */

/* Clipping test against active clipping rectangle of an SDL surface */
static __inline__ int
ClippedPixel(SDL_Surface *_Nonnull s, int x, int y)
{
	return (x < s->clip_rect.x || x >= s->clip_rect.x+s->clip_rect.w ||
	        y < s->clip_rect.y || y >= s->clip_rect.y+s->clip_rect.h);
}

static void
SDLFB_PutPixel(void *_Nonnull obj, int x, int y, AG_Color c)
{
	AG_DriverSDLFB *sfb = obj;
	SDL_Surface *s = sfb->s;
	Uint8 *p = (Uint8 *)s->pixels + y*s->pitch + x*s->format->BytesPerPixel;
	Uint8 r = AG_Hto8(c.r);
	Uint8 g = AG_Hto8(c.g);
	Uint8 b = AG_Hto8(c.b);

	if (ClippedPixel(s, x,y)) {
		return;
	}
	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, p,
	    SDL_MapRGB(s->format, r,g,b));
}

static void
SDLFB_PutPixel32(void *_Nonnull obj, int x, int y, Uint32 px)
{
	AG_DriverSDLFB *sfb = obj;
	SDL_Surface *s = sfb->s;
	Uint8 *p = (Uint8 *)s->pixels + y*s->pitch + x*s->format->BytesPerPixel;

	if (ClippedPixel(s, x,y)) {
		return;
	}
	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, p, px);
}

static void
SDLFB_PutPixelRGB8(void *_Nonnull obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	AG_DriverSDLFB *sfb = obj;

	SDLFB_PutPixel32(obj, x,y, SDL_MapRGB(sfb->s->format, r,g,b));
}

#if AG_MODEL == AG_LARGE

static void
SDLFB_PutPixel64(void *_Nonnull obj, int x, int y, Uint64 px)
{
	AG_Driver *drv = obj;
	AG_DriverSDLFB *sfb = obj;
	SDL_Surface *s = sfb->s;
	Uint8 *p = (Uint8 *)s->pixels + y*s->pitch + x*s->format->BytesPerPixel;
	Uint16 r,g,b;
	Uint32 px32;

	if (ClippedPixel(s, x,y)) {
		return;
	}
	AG_GetColor64_RGB16(px, drv->videoFmt, &r,&g,&b);
	px32 = AG_MapPixel32_RGB16(drv->videoFmt, r,g,b);
	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, p, px32);
}

static void
SDLFB_PutPixelRGB16(void *_Nonnull obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
{
	AG_DriverSDLFB *sfb = obj;

	SDLFB_PutPixel32(obj, x,y,
	    SDL_MapRGB(sfb->s->format,
	        AG_16to8(r),
	        AG_16to8(g),
	        AG_16to8(b)));
}

#endif /* AG_LARGE */

static void
SDLFB_BlendPixel(void *_Nonnull obj, int x, int y, AG_Color C, AG_AlphaFn fnSrc,
    AG_AlphaFn fnDst)
{
	AG_DriverSDLFB *sfb = obj;
	SDL_Surface *s = sfb->s;
	Uint32 pxDst, pxNew;
	Uint8 dR, dG, dB, dA;
	int alpha;
	Uint8 *pDst = (Uint8 *)s->pixels +
	    y*s->pitch +
	    x*s->format->BytesPerPixel;

	if (ClippedPixel(s, x,y))
		return;

	/* Extract the current destination pixel color. */
	AG_PACKEDPIXEL_GET(s->format->BytesPerPixel, pxDst, pDst);

#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0)
	if ((s->flags & SDL_SRCCOLORKEY) && (pxDst == s->format->colorkey)) {
		pxNew = SDL_MapRGBA(s->format, AG_Hto8(C.r), AG_Hto8(C.g),
		                               AG_Hto8(C.b), AG_Hto8(C.a));
	 	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, pDst, pxNew);
		return;
	}
#endif /* SDL<1.3 */

	/* Blend the components and write the computed pixel value. */
	SDL_GetRGBA(pxDst, s->format, &dR, &dG, &dB, &dA);
	switch (fnSrc) {
	case AG_ALPHA_OVERLAY:		alpha = dA + AG_Hto8(C.a);	break;
	case AG_ALPHA_SRC:		alpha = AG_Hto8(C.a);		break;
	case AG_ALPHA_DST:		alpha = dA;			break;
	case AG_ALPHA_ONE_MINUS_DST:	alpha = 1 - dA;			break;
	case AG_ALPHA_ONE_MINUS_SRC:	alpha = 1 - AG_Hto8(C.a);	break;
	case AG_ALPHA_ZERO:		alpha = 0;			break;
	default:
	case AG_ALPHA_ONE:		alpha = 255;			break;
	}

	pxNew = SDL_MapRGBA(s->format,
	    dR + (((AG_Hto8(C.r) - dR)*C.a) >> 8),
	    dG + (((AG_Hto8(C.g) - dG)*C.a) >> 8),
	    dB + (((AG_Hto8(C.b) - dB)*C.a) >> 8),
	    (alpha < 0) ? 0 : (alpha > 255) ? 255 : alpha);

	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, pDst, pxNew);
}

static void
SDLFB_DrawLine(void *_Nonnull obj, int x1, int y1, int x2, int y2, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	int dx, dy;
	int inc1, inc2;
	int x, y, d;
	int xEnd, yEnd;
	int xDir, yDir;
	Uint32 c;
	
	/* XXX XXX TODO per-line clipping */

	c = SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b));
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			yDir = -1;
			xEnd = x1;
		} else {
			x = x1;
			y = y1;
			yDir = 1;
			xEnd = x2;
		}
		SDLFB_PutPixel32(obj, x,y, c);

		if (((y2-y1)*yDir) > 0) {
			while (x < xEnd) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				SDLFB_PutPixel32(obj, x,y, c);
			}
		} else {
			while (x < xEnd) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				SDLFB_PutPixel32(obj, x,y, c);
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yEnd = y1;
			xDir = -1;
		} else {
			y = y1;
			x = x1;
			yEnd = y2;
			xDir = 1;
		}
		SDLFB_PutPixel32(obj, x,y, c);

		if (((x2-x1)*xDir) > 0) {
			while (y < yEnd) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				SDLFB_PutPixel32(obj, x,y, c);
			}
		} else {
			while (y < yEnd) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				SDLFB_PutPixel32(obj, x,y, c);
			}
		}
	}
}

static __inline__ int
ClipHorizLine(AG_DriverSDLFB *_Nonnull sfb, int *_Nonnull x1, int *_Nonnull x2,
    int *_Nonnull y, int *_Nonnull dx)
{
	SDL_Rect *rd = &sfb->s->clip_rect;

	if (*y <  rd->y ||
	    *y >= rd->y+rd->h) {
		return (1);
	}
	if (*x1 > *x2) {
		*dx = *x2;
		*x2 = *x1;
		*x1 = *dx;
	}
	if (*x1 < rd->x)
		*x1 = rd->x;
	if (*x2 >= rd->x+rd->w)
		*x2 = rd->x + rd->w - 1;

	*dx = *x2 - *x1;
	return (*dx <= 0);
}
static __inline__ int
ClipVertLine(AG_DriverSDLFB *_Nonnull sfb, int *_Nonnull x,
    int *_Nonnull y1, int *_Nonnull y2, int *_Nonnull dy)
{
	SDL_Rect *rd = &sfb->s->clip_rect;

	if (*x <  rd->x ||
	    *x >= rd->x+rd->w) {
		return (1);
	}
	if (*y1 > *y2) {
		*dy = *y2;
		*y2 = *y1;
		*y1 = *dy;
	}
	if (*y1 < rd->y)
		*y1 = rd->y;
	if (*y2 >= rd->y+rd->h)
		*y2 = rd->y + rd->h - 1;

	*dy = *y2 - *y1;
	return (*dy <= 0);
}

static void
SDLFB_DrawLineH(void *_Nonnull obj, int x1, int x2, int y, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	Uint8 *pDst, *pEnd;
	Uint32 c;
	int dx;
	
	if (ClipHorizLine(sfb, &x1, &x2, &y, &dx))
		return;

	c = SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b));
	switch (sfb->s->format->BytesPerPixel) {
	case 4:
		pDst = (Uint8 *)sfb->s->pixels + y*sfb->s->pitch + x1*4;
		pEnd = pDst + (dx<<2);
		while (pDst < pEnd) {
			*(Uint32 *)pDst = c;
			pDst += 4;
		}
		break;
	case 3:
		pDst = (Uint8 *)sfb->s->pixels + y*sfb->s->pitch + x1*3;
		pEnd = pDst + dx*3;
		while (pDst < pEnd) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
			pDst[0] = (c >>16) & 0xff;
			pDst[1] = (c >>8) & 0xff;
			pDst[2] = c & 0xff;
#else
			pDst[2] = (c>>16) & 0xff;
			pDst[1] = (c>>8) & 0xff;
			pDst[0] = c & 0xff;
#endif
			pDst += 3;
		}
		break;
	case 2:
		pDst = (Uint8 *)sfb->s->pixels + y*sfb->s->pitch + x1*2;
		pEnd = pDst + (dx<<1);
		while (pDst < pEnd) {
			*(Uint16 *)pDst = c;
			pDst += 2;
		}
		break;
	default:
		pDst = (Uint8 *)sfb->s->pixels + y*sfb->s->pitch + x1;
		pEnd = pDst + dx;
		memset(pDst, c, pEnd-pDst);
		break;
	}
}

static void
SDLFB_DrawLineV(void *_Nonnull obj, int x, int y1, int y2, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	Uint8 *pDst, *pEnd;
	Uint32 c;
	int dy;

	if (ClipVertLine(sfb, &x, &y1, &y2, &dy))
		return;

	c = SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b));
	switch (sfb->s->format->BytesPerPixel) {
	case 4:
		pDst = (Uint8 *)sfb->s->pixels + y1*sfb->s->pitch + x*4;
		pEnd = pDst + dy*sfb->s->pitch;
		while (pDst < pEnd) {
			*(Uint32 *)pDst = c;
			pDst += sfb->s->pitch;
		}
		break;
	case 3:
		pDst = (Uint8 *)sfb->s->pixels + y1*sfb->s->pitch + x*3;
		pEnd = pDst + dy*sfb->s->pitch;
		while (pDst < pEnd) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
			pDst[0] = (c >>16) & 0xff;
			pDst[1] = (c >>8) & 0xff;
			pDst[2] = c & 0xff;
#else
			pDst[2] = (c>>16) & 0xff;
			pDst[1] = (c>>8) & 0xff;
			pDst[0] = c & 0xff;
#endif
			pDst += sfb->s->pitch;
		}
		break;
	case 2:
		pDst = (Uint8 *)sfb->s->pixels + y1*sfb->s->pitch + x*2;
		pEnd = pDst + dy*sfb->s->pitch;
		while (pDst < pEnd) {
			*(Uint16 *)pDst = c;
			pDst += sfb->s->pitch;
		}
		break;
	default:
		pDst = (Uint8 *)sfb->s->pixels + y1*sfb->s->pitch + x;
		pEnd = pDst + dy*sfb->s->pitch;
		while (pDst < pEnd) {
			*(Uint8 *)pDst = c;
			pDst += sfb->s->pitch;
		}
		break;
	}
}

static void
SDLFB_DrawLineBlended(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    AG_Color C, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydir = -1;
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydir = 1;
			xend = x2;
		}
		SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdir = -1;
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdir = 1;
		}
		SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);
			}
		}
	}
}

static void
SDLFB_DrawTriangle_FlatBot(void *_Nonnull obj, AG_Pt v1, AG_Pt v2, AG_Pt v3,
    AG_Color c)
{
	float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
	float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);
	float x1 = v1.x;
	float x2 = v1.x;
	int y;

	for (y = v1.y; y <= v2.y; y++) {
		SDLFB_DrawLineH(obj, x1,x2, y, c);
		x1 += invslope1;
		x2 += invslope2;
	}
}

static void
SDLFB_DrawTriangle_FlatTop(void *_Nonnull obj, AG_Pt v1, AG_Pt v2, AG_Pt v3,
    AG_Color c)
{
	float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
	float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);
	float x1 = v3.x;
	float x2 = v3.x;
	int y;

	for (y = v3.y; y <= v1.y; y--) {
		SDLFB_DrawLineH(obj, x1,x2, y, c);
		x1 -= invslope1;
		x2 -= invslope2;
	}
}

static void
SDLFB_DrawTriangle(void *_Nonnull obj, AG_Pt v1, AG_Pt v2, AG_Pt v3, AG_Color c)
{
	AG_Pt V1, V2, V3;

	/* Sort the three vertices by y coordinate ascending. */
	if (v3.y > v2.y &&
	    v3.y > v1.y) {
		V3 = v3;
		if (v2.y > v1.y) {
			V2=v2; V1=v1;
		} else {
			V2=v1; V1=v2;
		}
	} else if (v2.y > v1.y &&
	           v2.y > v3.y) {
		V3 = v2;
		if (v3.y > v1.y) {
			V2=v3; V1=v1;
		} else {
			V2=v1; V1=v3;
		}
	} else {
		V3 = v1;
		if (v3.y > v2.y) {
			V2=v3; V1=v2;
		} else {
			V2=v2; V1=v3;
		}
	}
	v1 = V1;
	v2 = V2;
	v3 = V3;

	if (v2.y == v3.y) {
		SDLFB_DrawTriangle_FlatBot(obj, v1,v2,v3, c);
	} else if (v1.y == v2.y) {
		SDLFB_DrawTriangle_FlatTop(obj, v1,v2,v3, c);
	} else {
		AG_Pt v4;

		v4.x = (int)(v1.x + ((float)(v2.y - v1.y) /
		                     (float)(v3.y - v1.y)) * (v3.x - v1.x));
		v4.y = v2.y;
		SDLFB_DrawTriangle_FlatBot(obj, v1,v2,v4, c);
		SDLFB_DrawTriangle_FlatTop(obj, v2,v4,v3, c);
	}
}

static void
SDLFB_DrawArrow_Up(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int y1 = y0 - (h >> 1) + 1;
	int y2 = y1 + h-2;
	int xs = x0, xe = xs;
	int x, y;

	for (y = y1; y < y2; ++y) {
		for (x = xs; x <= xe; x++) {
			SDLFB_PutPixel32(obj, x,y, px);
		}
		xs--;
		xe++;
	}
}
		
static void
SDLFB_DrawArrow_Right(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int x1 = x0 - (h >> 1) + 1;
	int x2 = x1 + h-2;
	int ys = y0, ye = ys;
	int x, y;
	
	for (x = x2; x > x1; --x) {
		for (y = ys; y <= ye; y++) {
			SDLFB_PutPixel32(obj, x,y, px);
		}
		ys--;
		ye++;
	}
}

static void
SDLFB_DrawArrow_Down(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int y1 = y0 - (h >> 1) + 1;
	int y2 = y1 + h-2;
	int xs = x0, xe = xs;
	int x, y;
	
	for (y = y2; y > y1; --y) {
		for (x = xs; x <= xe; x++) {
			SDLFB_PutPixel32(obj, x,y, px);
		}
		xs--;
		xe++;
	}
}
		
static void
SDLFB_DrawArrow_Left(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int x1 = x0 - (h >> 1) + 1;
	int x2 = x1 + h-2;
	int ys = y0, ye = ys;
	int x, y;
	
	for (x = x1; x < x2; ++x) {
		for (y = ys; y <= ye; y++) {
			SDLFB_PutPixel32(obj, x,y, px);
		}
		ys--;
		ye++;
	}
}

static void
SDLFB_DrawArrow(void *_Nonnull obj, Uint8 angle, int x0, int y0, int h,
    AG_Color c)
{
	AG_DriverSDLFB *sfb = obj;
	static void (*pf[])(void *_Nonnull, int,int, int, Uint32) = {
		SDLFB_DrawArrow_Up,
		SDLFB_DrawArrow_Right,
		SDLFB_DrawArrow_Down,
		SDLFB_DrawArrow_Left,
	};
	Uint32 px = SDL_MapRGB(sfb->s->format, c.r, c.g, c.b);
#ifdef AG_DEBUG
	if (angle >= 4) { AG_FatalError("Bad angle"); }
#endif
	pf[angle](obj, x0,y0, h, px);
}

static void
SDLFB_DrawBoxRoundedTop(void *_Nonnull obj, AG_Rect r, int z, int rad,
    AG_Color c1, AG_Color c2, AG_Color c3)
{
	AG_DriverSDLFB *sfb = obj;
	AG_Rect rd;
	int v, e, u;
	int x, y, i;
	Uint32 c[3];
	
	c[0] = SDL_MapRGB(sfb->s->format, c1.r, c1.g, c1.b);
	c[1] = SDL_MapRGB(sfb->s->format, c2.r, c2.g, c2.b);
	c[2] = SDL_MapRGB(sfb->s->format, c3.r, c3.g, c3.b);
	
	rd.x = r.x+rad;					/* Center rect */
	rd.y = r.y+rad;
	rd.w = r.w - rad*2;
	rd.h = r.h - rad;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.y = r.y;					/* Top rect */
	rd.h = r.h;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.x = r.x;					/* Left rect */
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.x = r.x+r.w-rad;				/* Right rect */
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad;
	SDLFB_DrawRectFilled(obj, rd, c1);

	/* Top, left and right lines */
	SDLFB_DrawLineH(obj, r.x+rad,   r.x+r.w-rad, r.y,	c1);
	SDLFB_DrawLineV(obj, r.x,       r.y+rad,     r.y+r.h,	c2);
	SDLFB_DrawLineV(obj, r.x+r.w-1, r.y+rad,     r.y+r.h,	c3);

	/* Top left and top right rounded edges */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	while (x <= y) {
		SDLFB_PutPixel32(obj, r.x+rad-x, r.y+rad-y,         c[1]);
		SDLFB_PutPixel32(obj, r.x+rad-y, r.y+rad-x,         c[1]);
		SDLFB_PutPixel32(obj, r.x-rad+(r.w-1)+x, r.y+rad-y, c[2]);
		SDLFB_PutPixel32(obj, r.x-rad+(r.w-1)+y, r.y+rad-x, c[2]);
		for (i = 0; i < x; i++) {
			SDLFB_PutPixel32(obj, r.x+rad-i, r.y+rad-y,         c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+(r.w-1)+i, r.y+rad-y, c[0]);
		}
		for (i = 0; i < y; i++) {
			SDLFB_PutPixel32(obj, r.x+rad-i, r.y+rad-x,         c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+(r.w-1)+i, r.y+rad-x, c[0]);
		}
		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
	}
}

static void
SDLFB_DrawBoxRounded(void *_Nonnull obj, AG_Rect r, int z, int rad,
    AG_Color c1, AG_Color c2, AG_Color c3)
{
	AG_DriverSDLFB *sfb = obj;
	AG_Rect rd;
	int v, e, u;
	int x, y, i;
	int w1 = r.w - 1;
	Uint32 c[3];
	
	if (rad*2 > r.w || rad*2 > r.h) {
		rad = MIN(r.w/2, r.h/2);
	}
	if (r.w < 4 || r.h < 4)
		return;
	
	c[0] = SDL_MapRGB(sfb->s->format, c1.r, c1.g, c1.b);
	c[1] = SDL_MapRGB(sfb->s->format, c2.r, c2.g, c2.b);
	c[2] = SDL_MapRGB(sfb->s->format, c3.r, c3.g, c3.b);
	
	rd.x = r.x + rad;					/* Center */
	rd.y = r.y + rad;
	rd.w = r.w - rad*2;
	rd.h = r.h - rad*2;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.y = r.y;						/* Top */
	rd.h = rad;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.y = r.y+r.h - rad;					/* Bottom */
	rd.h = rad;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.x = r.x;						/* Left */
	rd.y = r.y + rad;
	rd.w = rad;
	rd.h = r.h - rad*2;
	SDLFB_DrawRectFilled(obj, rd, c1);
	rd.x = r.x + r.w - rad;					/* Right */
	rd.y = r.y + rad;
	rd.w = rad;
	rd.h = r.h - rad*2;
	SDLFB_DrawRectFilled(obj, rd, c1);

	/* Rounded edges */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	while (x <= y) {
		SDLFB_PutPixel32(obj, r.x+rad-x,    r.y+rad-y,     	c[1]);
		SDLFB_PutPixel32(obj, r.x+rad-y,    r.y+rad-x,     	c[1]);
		SDLFB_PutPixel32(obj, r.x-rad+w1+x, r.y+rad-y,     	c[2]);
		SDLFB_PutPixel32(obj, r.x-rad+w1+y, r.y+rad-x,     	c[2]);

		SDLFB_PutPixel32(obj, r.x+rad-x,    r.y+r.h-rad+y, 	c[1]);
		SDLFB_PutPixel32(obj, r.x+rad-y,    r.y+r.h-rad+x, 	c[1]);
		SDLFB_PutPixel32(obj, r.x-rad+w1+x, r.y+r.h-rad+y, 	c[2]);
		SDLFB_PutPixel32(obj, r.x-rad+w1+y, r.y+r.h-rad+x, 	c[2]);

		for (i = 0; i < x; i++) {
			SDLFB_PutPixel32(obj, r.x+rad-i,    r.y+rad-y,	c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+w1+i, r.y+rad-y,	c[0]);
			SDLFB_PutPixel32(obj, r.x+rad-i,    r.y+r.h-rad+y,	c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+w1+i, r.y+r.h-rad+y,	c[0]);
		}
		for (i = 0; i < y; i++) {
			SDLFB_PutPixel32(obj, r.x+rad-i,    r.y+rad-x,	c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+w1+i, r.y+rad-x,	c[0]);
			SDLFB_PutPixel32(obj, r.x+rad-i,    r.y+r.h-rad+x,	c[0]);
			SDLFB_PutPixel32(obj, r.x-rad+w1+i, r.y+r.h-rad+x,	c[0]);
		}
		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
	}
	
	/* Contour lines */
	SDLFB_DrawLineH(obj, r.x+rad,   r.x+r.w-rad,   r.y,         c1);
	SDLFB_DrawLineH(obj, r.x+rad/2, r.x+r.w-rad/2, r.y,         c2);
	SDLFB_DrawLineH(obj, r.x+rad/2, r.x+r.w-rad/2, r.y+r.h,     c3);
	SDLFB_DrawLineV(obj, r.x,       r.y+rad,       r.y+r.h-rad, c2);
	SDLFB_DrawLineV(obj, r.x+w1,    r.y+rad,       r.y+r.h-rad, c3);
}

static void
SDLFB_DrawCircle(void *_Nonnull obj, int x1, int y1, int radius, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;
	Uint32 c;

	c = SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b));
	while (x < y) {
		SDLFB_PutPixel32(obj, x1+x, y1+y, c);
		SDLFB_PutPixel32(obj, x1+x, y1-y, c);
		SDLFB_PutPixel32(obj, x1-x, y1+y, c);
		SDLFB_PutPixel32(obj, x1-x, y1-y, c);
		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		SDLFB_PutPixel32(obj, x1+y, y1+x, c);
		SDLFB_PutPixel32(obj, x1+y, y1-x, c);
		SDLFB_PutPixel32(obj, x1-y, y1+x, c);
		SDLFB_PutPixel32(obj, x1-y, y1-x, c);
	}
	SDLFB_PutPixel32(obj, x1-radius, y1, c);
	SDLFB_PutPixel32(obj, x1+radius, y1, c);
}

static void
SDLFB_DrawCircleFilled(void *_Nonnull obj, int x1, int y1, int radius, AG_Color C)
{
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		SDLFB_DrawLineV(obj, x1+x, y1+y, y1-y, C);
		SDLFB_DrawLineV(obj, x1-x, y1+y, y1-y, C);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		SDLFB_DrawLineV(obj, x1+y, y1+x, y1-x, C);
		SDLFB_DrawLineV(obj, x1-y, y1+x, y1-x, C);
	}
}

static void
SDLFB_DrawRectFilled(void *_Nonnull obj, AG_Rect r, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	SDL_Rect rd;

	rd.x = (Sint16)r.x;
	rd.y = (Sint16)r.y;
	rd.w = (Uint16)r.w;
	rd.h = (Uint16)r.h;
	SDL_FillRect(sfb->s, &rd,
	    SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b)));
}

static void
SDLFB_DrawRectBlended(void *_Nonnull obj, AG_Rect r, AG_Color C,
    AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	AG_DriverSDLFB *sfb = obj;
	int x, y;

	for (y = r.y; y < r.y+r.h; y++) {
		for (x = r.x; x < r.x+r.w; x++) {
			if (ClippedPixel(sfb->s, x,y)) {
				continue;
			}
			SDLFB_BlendPixel(obj, x,y, C, fnSrc, fnDst);
		}
	}
}

static void
SDLFB_DrawRectDithered(void *_Nonnull obj, AG_Rect r, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;
	int x, y;
	int flag = 0;
	Uint32 c;
	
	/* XXX inefficient */
	c = SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b));
	for (y = r.y; y < r.y+r.h-2; y++) {
		flag = !flag;
		for (x = r.x+1+flag; x < r.x+r.w-2; x+=2)
			SDLFB_PutPixel32(obj, x,y, c);
	}
}

static void
SDLFB_UpdateGlyph(void *_Nonnull drv, AG_Glyph *_Nonnull gl)
{
	/* Nothing to do */
}

static void
SDLFB_DrawGlyph(void *_Nonnull drv, const AG_Glyph *_Nonnull gl, int x, int y)
{
	AG_DriverSDLFB *sfb = drv;
	
	AG_SDL_BlitSurface(gl->su, NULL, sfb->s, x,y);
}

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverSDLFB *_Nonnull sfb, int wView, int hView)
{
	AG_ClipRect *cr;

	/* Rectangle 0 always covers the whole view. */
	if ((sfb->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	cr = &sfb->clipRects[0];
	cr->r = AG_RECT(0, 0, wView, hView);
	sfb->nClipRects = 1;
	return (0);
}

/*
 * Single-display specific operations.
 */

static int
SDLFB_OpenVideo(void *_Nonnull obj, Uint w, Uint h, int depth, Uint flags)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLFB *sfb = obj;
	Uint32 sFlags = 0;
	int newDepth;

	/* Set the requested display options. */
	if (flags & AG_VIDEO_HWSURFACE) {
		sFlags |= SDL_HWSURFACE;
	} else {
		sFlags |= SDL_SWSURFACE;
	}
	if (flags & AG_VIDEO_RESIZABLE) { sFlags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_ASYNCBLIT) { sFlags |= SDL_ASYNCBLIT; }
	if (flags & AG_VIDEO_ANYFORMAT) { sFlags |= SDL_ANYFORMAT; }
	if (flags & AG_VIDEO_HWPALETTE) { sFlags |= SDL_HWPALETTE; }
	if (flags & AG_VIDEO_DOUBLEBUF) { sFlags |= SDL_DOUBLEBUF; }
	if (flags & AG_VIDEO_FULLSCREEN) { sFlags |= SDL_FULLSCREEN; }
	if (flags & AG_VIDEO_NOFRAME) { sFlags |= SDL_NOFRAME; }

	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;

	/* Set the video mode. Force hardware palette in 8bpp. */
	AG_SDL_GetPrefDisplaySettings(drv, &w, &h, &depth);
	Verbose(_("SDLFB: Setting mode %ux%u (%d bpp)\n"), w, h, depth);
	newDepth = SDL_VideoModeOK(w, h, depth, sFlags);
	if (newDepth == 8) {
		Verbose(_("SDLFB: Using hardware palette\n"));
		sFlags |= SDL_HWPALETTE;
	}
	if ((sfb->s = SDL_SetVideoMode((int)w, (int)h, newDepth, sFlags))
	    == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s", w, h, newDepth,
		    SDL_GetError());
		return (-1);
	}
	SDL_EnableUNICODE(1);

	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sfb->s)) == NULL) {
		goto fail;
	}
	dsw->w = sfb->s->w;
	dsw->h = sfb->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

#if AG_MODEL == AG_LARGE
	Verbose(_("SDLFB: New display (%d-bpp; %08lx,%08lx,%08lx)\n"),
	     drv->videoFmt->BitsPerPixel, 
	     drv->videoFmt->Rmask,
	     drv->videoFmt->Gmask,
	     drv->videoFmt->Bmask);
#else
	Verbose(_("SDLFB: New display (%d-bpp; %04x,%04x,%04x)\n"),
	     drv->videoFmt->BitsPerPixel, 
	     drv->videoFmt->Rmask,
	     drv->videoFmt->Gmask,
	     drv->videoFmt->Bmask);
#endif

	/* Initialize clipping rectangles. */
	if (InitClipRects(sfb, dsw->w, dsw->h) == -1)
		goto fail;
	
	/* Create the cursors. */
	if (AG_SDL_InitDefaultCursor(sfb) == -1 ||
	    AG_InitStockCursors(drv) == -1)
		goto fail;

	/* Set background color. */
	SDL_FillRect(sfb->s, NULL, SDL_MapRGB(sfb->s->format,
	    dsw->bgColor.r, dsw->bgColor.g, dsw->bgColor.b));
	SDL_UpdateRect(sfb->s, 0, 0, (Sint32)w, (Sint32)h);

	if (flags & AG_VIDEO_FULLSCREEN) {
		if (SDL_WM_ToggleFullScreen(sfb->s))
			dsw->flags |= AG_DRIVER_SW_FULLSCREEN;
	}
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static int
SDLFB_OpenVideoContext(void *_Nonnull obj, void *_Nonnull ctx, Uint flags)
{
	AG_DriverSDLFB *sfb = obj;
	AG_DriverSw *dsw = obj;
	AG_Driver *drv = obj;

	/* Set the requested display options. */
	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;

	/* Use the given display surface. */
	sfb->s = (SDL_Surface *)ctx;
	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sfb->s)) == NULL) {
		goto fail;
	}
	dsw->w = sfb->s->w;
	dsw->h = sfb->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

#if AG_MODEL == AG_LARGE
	Verbose(_("SDLFB: Using existing display (%d-bpp; %08lx,%08lx,%08lx)\n"),
	     (int)drv->videoFmt->BitsPerPixel, 
	     drv->videoFmt->Rmask,
	     drv->videoFmt->Gmask,
	     drv->videoFmt->Bmask);
#else
	Verbose(_("SDLFB: Using existing display (%d-bpp; %04x,%04x,%04x)\n"),
	     (int)drv->videoFmt->BitsPerPixel, 
	     drv->videoFmt->Rmask,
	     drv->videoFmt->Gmask,
	     drv->videoFmt->Bmask);
#endif

	/* Initialize clipping rectangles. */
	if (InitClipRects(sfb, dsw->w, dsw->h) == -1)
		goto fail;
	
	/* Create the cursors. */
	if (AG_SDL_InitDefaultCursor(sfb) == -1 ||
	    AG_InitStockCursors(drv) == -1)
		goto fail;
	
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
SDLFB_CloseVideo(void *_Nonnull obj)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDLFB *sfb = obj;

	if (dsw->flags & AG_DRIVER_SW_FULLSCREEN) {
		SDL_WM_ToggleFullScreen(sfb->s);
		dsw->flags &= ~(AG_DRIVER_SW_FULLSCREEN);
	}
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
SDLFB_VideoResize(void *_Nonnull obj, Uint w, Uint h)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDLFB *sfb = obj;
	Uint32 sFlags;
	SDL_Surface *su;
	AG_ClipRect *cr0;

	sFlags = sfb->s->flags & (SDL_SWSURFACE|SDL_HWSURFACE|SDL_ASYNCBLIT|
				  SDL_ANYFORMAT|SDL_HWPALETTE|SDL_DOUBLEBUF|
				  SDL_FULLSCREEN|SDL_OPENGL|SDL_OPENGLBLIT|
				  SDL_RESIZABLE|SDL_NOFRAME);
	                          
	if ((su = SDL_SetVideoMode(w, h, 0, sFlags)) == NULL) {
		AG_SetError("Cannot resize display to %ux%u: %s", w, h,
		    SDL_GetError());
		return (-1);
	}
	sfb->s = su;

	dsw->w = su->w;
	dsw->h = su->h;
	dsw->depth = (Uint)su->format->BitsPerPixel;

	/* Update clipping rectangle 0. */
	cr0 = &sfb->clipRects[0];
	cr0->r.w = w;
	cr0->r.h = h;
	/* Clear the background. */
	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY)) {
		SDL_FillRect(sfb->s, NULL,
		    SDL_MapRGB(sfb->s->format, dsw->bgColor.r, dsw->bgColor.g, dsw->bgColor.b));
		SDL_UpdateRect(sfb->s, 0, 0, (Sint32)w, (Sint32)h);
	}
	return (0);
}

static AG_Surface *
SDLFB_VideoCapture(void *_Nonnull obj)
{
	AG_DriverSDLFB *sfb = obj;

	return AG_SDL_ImportSurface(sfb->s);
}

static void
SDLFB_VideoClear(void *_Nonnull obj, AG_Color C)
{
	AG_DriverSDLFB *sfb = obj;

	SDL_FillRect(sfb->s, NULL,
	    SDL_MapRGB(sfb->s->format, AG_Hto8(C.r), AG_Hto8(C.g), AG_Hto8(C.b)));
	SDL_UpdateRect(sfb->s, 0, 0, sfb->s->w, sfb->s->h);
}

static int
SDLFB_SetVideoContext(void *_Nonnull obj, void *_Nonnull pSurface)
{
	AG_DriverSDLFB *sfb = obj;
	AG_DriverSw *dsw = obj;
	SDL_Surface *su = pSurface;
	AG_ClipRect *cr0;

	sfb->s = su;
	dsw->w = su->w;
	dsw->h = su->h;
	dsw->depth = (Uint)su->format->BitsPerPixel;
	
	/* Update clipping rectangle 0. */
	cr0 = &sfb->clipRects[0];
	cr0->r.w = su->w;
	cr0->r.h = su->h;
	return (0);
}

AG_DriverSwClass agDriverSDLFB = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDLFB",
			sizeof(AG_DriverSDLFB),
			{ 1,6 },
			Init,
			NULL,	/* reset */
			Destroy,
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
		},
		"sdlfb",
		AG_FRAMEBUFFER,
		AG_WM_SINGLE,
		AG_DRIVER_SDL,
		SDLFB_Open,
		SDLFB_Close,
		AG_SDL_GetDisplaySize,
		AG_SDL_BeginEventProcessing,
		AG_SDL_PendingEvents,
		AG_SDL_GetNextEvent,
		AG_SDL_ProcessEvent,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDLFB_BeginRendering,
		SDLFB_RenderWindow,
		SDLFB_EndRendering,
		SDLFB_FillRect,
		SDLFB_UpdateRegion,
		NULL,				/* uploadTexture */
		NULL,				/* updateTexture */
		NULL,				/* deleteTexture */
		AG_SDL_SetRefreshRate,
		SDLFB_PushClipRect,
		SDLFB_PopClipRect,
		SDLFB_PushBlendingMode,
		SDLFB_PopBlendingMode,
		AG_SDL_CreateCursor,
		AG_SDL_FreeCursor,
		AG_SDL_SetCursor,
		AG_SDL_UnsetCursor,
		AG_SDL_GetCursorVisibility,
		AG_SDL_SetCursorVisibility,
		SDLFB_BlitSurface,
		SDLFB_BlitSurfaceFrom,
		SDLFB_BlitSurfaceGL,
		SDLFB_BlitSurfaceFromGL,
		SDLFB_BlitSurfaceFlippedGL,
		NULL,				/* backupSurfaces */
		NULL,				/* restoreSurfaces */
		SDLFB_RenderToSurface,
		SDLFB_PutPixel,
		SDLFB_PutPixel32,
		SDLFB_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		SDLFB_PutPixel64,
		SDLFB_PutPixelRGB16,
#endif
		SDLFB_BlendPixel,
		SDLFB_DrawLine,
		SDLFB_DrawLineH,
		SDLFB_DrawLineV,
		SDLFB_DrawLineBlended,
		SDLFB_DrawTriangle,
		SDLFB_DrawArrow,
		SDLFB_DrawBoxRounded,
		SDLFB_DrawBoxRoundedTop,
		SDLFB_DrawCircle,
		SDLFB_DrawCircleFilled,
		SDLFB_DrawRectFilled,
		SDLFB_DrawRectBlended,
		SDLFB_DrawRectDithered,
		SDLFB_UpdateGlyph,
		SDLFB_DrawGlyph,
		NULL				/* deleteList */
	},
	0,
	SDLFB_OpenVideo,
	SDLFB_OpenVideoContext,
	SDLFB_SetVideoContext,
	SDLFB_CloseVideo,
	SDLFB_VideoResize,
	SDLFB_VideoCapture,
	SDLFB_VideoClear
};
