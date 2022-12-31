/*
 * Copyright (c) 2022 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Single-window driver for framebuffer graphics via SDL2.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl2.h>

/* #define DEBUG_DISPLAY */

typedef struct ag_sdl2fb_driver {
	struct ag_driver_sw _inherit;	/* AG_Driver -> AG_DriverSw */

	SDL_Window *_Nullable window;	/* SDL window */

	SDL_Rect *_Nonnull dirty;	/* Video regions to update */
	int               nDirty;
	Uint            maxDirty;

	AG_ClipRect *clipRects;		/* Clipping rectangle stack */
	Uint        nClipRects;
	
	Uint           nPolyInts;
	int  *_Nullable polyInts;	/* Sorted intersections for drawPolygon */
} AG_DriverSDL2FB;

static int nDrivers = 0;			/* Opened driver instances */
static int initedSDL = 0;			/* Inited TIMERS and EVENTS */
static int initedSDLVideo = 0;			/* Inited VIDEO */
static AG_EventSink *_Nullable sfbEventSpinner = NULL;
static AG_EventSink *_Nullable sfbEventEpilogue = NULL;

static void SDL2FB_DrawRectFilled(void *_Nonnull, const AG_Rect *_Nonnull,
                                 const AG_Color *_Nonnull);
static void SDL2FB_UpdateRegion(void *_Nonnull, const AG_Rect *_Nonnull);
static int CompareInts(const void *_Nonnull, const void *_Nonnull);

static void
Init(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;

	sfb->window = NULL;
	sfb->nDirty = 0;
	sfb->dirty = Malloc(4*sizeof(SDL_Rect));
	sfb->maxDirty = 4;
	sfb->clipRects = NULL;
	sfb->nClipRects = 0;
	sfb->nPolyInts = 0;
	sfb->polyInts = NULL;
}

static void
Destroy(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;

	Free(sfb->dirty);
	Free(sfb->clipRects);
	Free(sfb->polyInts);
}

/*
 * Generic driver operations
 */

static int
SDL2FB_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2FB *sfb = obj;
	
	if (nDrivers != 0) {
		AG_SetError("Multiple SDL2 driver instances are not supported");
		return (-1);
	}

	/* Initialize SDL's video subsystem. */
	if (!initedSDL) {
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS) == -1) {
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
		Debug(sfb, "SDL Video Initialized (%s)\n",
		    SDL_GetCurrentVideoDriver());
		initedSDLVideo = 1;
	}
#if 0
	/* Use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);
	AG_DestroyEventSubsystem();
	AG_InitEventSubsystem(AG_SOFT_TIMERS);
#endif
	if ((drv->mouse = AG_MouseNew(sfb, "SDL2 mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(sfb, "SDL2 keyboard")) == NULL)
		goto fail;

#if 0
	/* XXX SDL2 */
	/* Configure the window caption */
	if (agProgName != NULL)
		SDL_WM_SetCaption(agProgName, agProgName);
#endif
	/*
	 * TODO where AG_SINK_READ capability and pipes are available,
	 * could we create a separate thread running SDL_WaitEvent() and
	 * sending notifications over a pipe, instead of using a spinner?
	 */
	if ((sfbEventSpinner = AG_AddEventSpinner(AG_SDL2_EventSink_SW, "%p", drv)) == NULL ||
	    (sfbEventEpilogue = AG_AddEventEpilogue(AG_SDL2_EventEpilogue, NULL)) == NULL) {
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
SDL2FB_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2FB *sfb = obj;
	
	AG_DelEventSpinner(sfbEventSpinner); sfbEventSpinner = NULL;
	AG_DelEventEpilogue(sfbEventEpilogue); sfbEventEpilogue = NULL;

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sfb));

	if (initedSDLVideo) {
		Debug(sfb, "SDL Video Quit (%s)\n",
		    SDL_GetCurrentVideoDriver());
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDelete(drv->kbd); drv->kbd = NULL;
	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;

	nDrivers = 0;
}

static void
SDL2FB_BeginRendering(void *_Nonnull obj)
{
//	AG_DriverSDL2FB *sfb = obj;

//	SDL_LockSurface(sfb->s);
}

static void
SDL2FB_RenderWindow(struct ag_window *_Nonnull win)
{
	AG_Rect rd;

	AG_WidgetDraw(win);

	rd.x = WIDGET(win)->x;
	rd.y = WIDGET(win)->y;
	rd.w = WIDTH(win);
	rd.h = HEIGHT(win);

	SDL2FB_UpdateRegion(WIDGET(win)->drv, &rd);
}

static void
SDL2FB_EndRendering(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;

#ifdef AG_DEBUG
	if (sfb->nClipRects != 1)
		AG_FatalError("Inconsistent PushClipRect() / PopClipRect()");
#endif
	if (sfb->nDirty > 0) {
		SDL_UpdateWindowSurfaceRects(sfb->window,
		    sfb->dirty, sfb->nDirty);
		sfb->nDirty = 0;
	}
//	SDL_UnlockSurface(sfb->s);
}

static void
SDL2FB_FillRect(void *_Nonnull obj, const AG_Rect *r, const AG_Color *_Nonnull c)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Rect rSDL;

	rSDL.x = r->x;
	rSDL.y = r->y;
	rSDL.w = r->w;
	rSDL.h = r->h;

	SDL_FillRect(Swin, &rSDL,
	    SDL_MapRGB(Swin->format,
	        AG_Hto8(c->r),
		AG_Hto8(c->g),
		AG_Hto8(c->b)));
}

static void
SDL2FB_UpdateRegion(void *_Nonnull obj, const AG_Rect *_Nonnull rRegion)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDL2FB *sfb = obj;
	SDL_Rect *rSDL;
	AG_Rect2 r;
	Uint w = dsw->w;
	Uint h = dsw->h;
	int n;

	AG_RectToRect2(&r, rRegion);

	/* TODO Compute intersections? */
	if (r.x1 < 0) { r.x1 = 0; }
	if (r.y1 < 0) { r.y1 = 0; }
	if (r.x2 > w) { r.x2 = w; r.w = r.x2-r.x1; }
	if (r.y2 > h) { r.y2 = h; r.h = r.y2-r.y1; }
	if (r.w < 0)  { r.x1 = 0; r.x2 = r.w = w; }
	if (r.h < 0)  { r.y1 = 0; r.y2 = r.h = h; }

	n = sfb->nDirty++;
	if (n+1 > sfb->maxDirty) {
		sfb->maxDirty *= 2;
		sfb->dirty = Realloc(sfb->dirty, sfb->maxDirty*sizeof(SDL_Rect));
	}
	rSDL = &sfb->dirty[n];
	rSDL->x = r.x1;
	rSDL->y = r.y1;
	rSDL->w = r.w;
	rSDL->h = r.h;
}

static void
SDL2FB_UpdateTexture(void *_Nonnull obj, Uint texture, AG_Surface *_Nonnull S,
    AG_TexCoord *_Nullable c)
{
	/* No-op */
}

static void
SDL2FB_DeleteTexture(void *_Nonnull obj, Uint texture)
{
	/* No-op */
}

/*
 * Clipping and blending control (rendering context)
 */

static void
SDL2FB_PushClipRect(void *_Nonnull obj, const AG_Rect *_Nonnull r)
{
	AG_DriverSDL2FB *sfb = obj;
	AG_ClipRect *cr, *crPrev;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Rect rSDL;

	sfb->clipRects = Realloc(sfb->clipRects, (sfb->nClipRects+1) *
	                                         sizeof(AG_ClipRect));
	crPrev = &sfb->clipRects[sfb->nClipRects-1];
	cr = &sfb->clipRects[sfb->nClipRects++];

	AG_RectIntersect(&cr->r, &crPrev->r, r);

	rSDL.x = cr->r.x;
	rSDL.y = cr->r.y;
	rSDL.w = cr->r.w;
	rSDL.h = cr->r.h;

	SDL_SetClipRect(Swin, &rSDL);
}

static void
SDL2FB_PopClipRect(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	AG_ClipRect *cr;
	SDL_Rect rSDL;

#ifdef AG_DEBUG
	if (sfb->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &sfb->clipRects[--sfb->nClipRects - 1];
	rSDL.x = (Sint16)cr->r.x;
	rSDL.y = (Sint16)cr->r.y;
	rSDL.w = (Uint16)cr->r.w;
	rSDL.h = (Uint16)cr->r.h;

	SDL_SetClipRect(Swin, &rSDL);
}

static void
SDL2FB_PushBlendingMode(void *_Nonnull drv, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	/* No-op (handle blending on a per-blit basis) */
}
static void
SDL2FB_PopBlendingMode(void *_Nonnull drv)
{
	/* No-op (handle blending on a per-blit basis) */
}

/*
 * Surface operations (rendering context)
 */

static void
SDL2FB_BlitSurface(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull s, int x, int y)
{
	AG_DriverSDL2FB *sfb = drv;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);

	AG_SDL2_BlitSurface(s, NULL, Swin, x,y);
}

static void
SDL2FB_BlitSurfaceFrom(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, const AG_Rect *_Nullable rSrc, int x, int y)
{
	AG_DriverSDL2FB *sfb = drv;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);

	AG_SDL2_BlitSurface(wid->surfaces[s], rSrc, Swin, x,y);
}

#ifdef HAVE_OPENGL
static void
SDL2FB_BlitSurfaceGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull s, float w, float h)
{
	/* Not applicable */
}

static void
SDL2FB_BlitSurfaceFromGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, float w, float h)
{
	/* Not applicable */
}

static void
SDL2FB_BlitSurfaceFlippedGL(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    int s, float w, float h)
{
	/* Not applicable */
}
#endif /* HAVE_OPENGL */

static int
SDL2FB_RenderToSurface(void *_Nonnull drv, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull *_Nullable pS)
{
	AG_DriverSDL2FB *sfb = drv;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Surface *S;
	int visiblePrev;
	SDL_Rect sr;

	if ((S = SDL_CreateRGBSurface(SDL_SWSURFACE, wid->w, wid->h, 32,
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
	SDL_BlitSurface(Swin, &sr, S, NULL);
	if ((*pS = AG_SDL2_ImportSurface(S)) == NULL) {
		SDL_FreeSurface(S);
		return (-1);
	}
	SDL_FreeSurface(S);
	return (0);
}

/*
 * Rendering operations (rendering context)
 */

/* Clipping test against active clipping rectangle of an SDL surface */
static __inline__ int
ClippedPixel(SDL_Surface *_Nonnull S, int x, int y)
{
	return (x < S->clip_rect.x || x >= S->clip_rect.x + S->clip_rect.w ||
	        y < S->clip_rect.y || y >= S->clip_rect.y + S->clip_rect.h);
}

static void
SDL2FB_PutPixel(void *_Nonnull obj, int x, int y, const AG_Color *_Nonnull c)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint8 *p = (Uint8 *)Swin->pixels + y*Swin->pitch +
	                                   x*Swin->format->BytesPerPixel;

	if (ClippedPixel(Swin, x,y)) {
		return;
	}
	AG_PACKEDPIXEL_PUT(Swin->format->BytesPerPixel, p,
	    SDL_MapRGB(Swin->format,
	        AG_Hto8(c->r),
		AG_Hto8(c->g),
		AG_Hto8(c->b)));
}

static void
SDL2FB_PutPixel32(void *_Nonnull obj, int x, int y, Uint32 px)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint8 *p = (Uint8 *)Swin->pixels + y*Swin->pitch +
	                                   x*Swin->format->BytesPerPixel;
	if (ClippedPixel(Swin, x,y)) {
		return;
	}
	AG_PACKEDPIXEL_PUT(Swin->format->BytesPerPixel, p, px);
}

static void
SDL2FB_PutPixelRGB8(void *_Nonnull obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);

	SDL2FB_PutPixel32(obj, x,y, SDL_MapRGB(Swin->format, r,g,b));
}

#if AG_MODEL == AG_LARGE
static void
SDL2FB_PutPixel64(void *_Nonnull obj, int x, int y, Uint64 px)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint8 *p = (Uint8 *)Swin->pixels + y*Swin->pitch +
	                                   x*Swin->format->BytesPerPixel;
	Uint16 r,g,b;
	Uint32 px32;

	if (ClippedPixel(Swin, x,y)) {
		return;
	}
	AG_GetColor64_RGB16(px, drv->videoFmt, &r,&g,&b);
	px32 = AG_MapPixel32_RGB16(drv->videoFmt, r,g,b);
	AG_PACKEDPIXEL_PUT(Swin->format->BytesPerPixel, p, px32);
}

static void
SDL2FB_PutPixelRGB16(void *_Nonnull obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);

	SDL2FB_PutPixel32(obj, x,y,
	    SDL_MapRGB(Swin->format,
	        AG_16to8(r),
	        AG_16to8(g),
	        AG_16to8(b)));
}
#endif /* AG_LARGE */

static void
SDL2FB_BlendPixel(void *_Nonnull obj, int x, int y, const AG_Color *c,
    AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint32 pxDst, pxNew;
	int BytesPerPixel = Swin->format->BytesPerPixel, a;
	Uint8 *pDst = (Uint8 *)Swin->pixels + y*Swin->pitch + x*BytesPerPixel;
	Uint8 dR,dG,dB,dA, Ca;

	if (ClippedPixel(Swin, x,y))
		return;

	/* Extract the current destination pixel color. */
	AG_PACKEDPIXEL_GET(BytesPerPixel, pxDst, pDst);

	Ca = AG_Hto8(c->a);
#if 0 /* SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0) */
	if ((S->flags & SDL_SRCCOLORKEY) && (pxDst == S->format->colorkey)) {
		pxNew = SDL_MapRGBA(S->format,
		    AG_Hto8(c->r),
		    AG_Hto8(c->g),
		    AG_Hto8(c->b), Ca);
	 	AG_PACKEDPIXEL_PUT(BytesPerPixel, pDst, pxNew);
		return;
	}
#endif /* SDL<1.3 */
	
	/* Blend the components and write the computed pixel value. */
	SDL_GetRGBA(pxDst, Swin->format, &dR,&dG,&dB,&dA);
	switch (fnSrc) {
	case AG_ALPHA_OVERLAY:		a = dA+Ca;			break;
	case AG_ALPHA_SRC:		a = Ca;				break;
	case AG_ALPHA_DST:		a = dA;				break;
	case AG_ALPHA_ONE_MINUS_DST:	a = 1 - dA;			break;
	case AG_ALPHA_ONE_MINUS_SRC:	a = 1 - Ca;			break;
	case AG_ALPHA_ZERO:		a = 0;				break;
	default:
	case AG_ALPHA_ONE:		a = 255;			break;
	}

	pxNew = SDL_MapRGBA(Swin->format,
	    dR + (((AG_Hto8(c->r) - dR)*Ca) >> 8),
	    dG + (((AG_Hto8(c->g) - dG)*Ca) >> 8),
	    dB + (((AG_Hto8(c->b) - dB)*Ca) >> 8),
	    (a < 0) ? 0 : (a > 255) ? 255 : a);

	AG_PACKEDPIXEL_PUT(BytesPerPixel, pDst, pxNew);
}

static void
SDL2FB_DrawLine(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull C)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint32 c;
	int dx, dy;
	int inc1, inc2;
	int x, y, d;
	int xEnd, yEnd;
	int xDir, yDir;
	
	/* XXX XXX TODO per-line clipping */

	c = SDL_MapRGB(Swin->format,
	    AG_Hto8(C->r),
	    AG_Hto8(C->g),
	    AG_Hto8(C->b));
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	if (dy <= dx) {
		d = (dy << 1) - dx;
		inc1 = (dy << 1);
		inc2 = (dy-dx) << 1;
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
		SDL2FB_PutPixel32(obj, x,y, c);

		if (((y2-y1)*yDir) > 0) {
			while (x < xEnd) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				SDL2FB_PutPixel32(obj, x,y, c);
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
				SDL2FB_PutPixel32(obj, x,y, c);
			}
		}		
	} else {
		d = (dx << 1) - dy;
		inc1 = dx << 1;
		inc2 = (dx-dy) << 1;
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
		SDL2FB_PutPixel32(obj, x,y, c);

		if (((x2-x1)*xDir) > 0) {
			while (y < yEnd) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				SDL2FB_PutPixel32(obj, x,y, c);
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
				SDL2FB_PutPixel32(obj, x,y, c);
			}
		}
	}
}

static __inline__ int
ClipHorizLine(AG_DriverSDL2FB *_Nonnull sfb, int *_Nonnull x1, int *_Nonnull x2,
    int *_Nonnull y, int *_Nonnull dx)
{
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Rect *rd = &Swin->clip_rect;

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
ClipVertLine(AG_DriverSDL2FB *_Nonnull sfb, int *_Nonnull x,
    int *_Nonnull y1, int *_Nonnull y2, int *_Nonnull dy)
{
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Rect *rd = &Swin->clip_rect;

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
SDL2FB_DrawLineH(void *_Nonnull obj, int x1, int x2, int y, const AG_Color *C)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint8 *pDst, *pEnd;
	Uint32 c;
	int dx;
	
	if (ClipHorizLine(sfb, &x1, &x2, &y, &dx))
		return;

	c = SDL_MapRGB(Swin->format,
	    AG_Hto8(C->r),
	    AG_Hto8(C->g),
	    AG_Hto8(C->b));

	switch (Swin->format->BytesPerPixel) {
	case 4:
		pDst = (Uint8 *)Swin->pixels + y*Swin->pitch + (x1 << 2);
		pEnd = pDst + (dx << 2);
		while (pDst <= pEnd) {
			*(Uint32 *)pDst = c;
			pDst += 4;
		}
		break;
	case 3:
		pDst = (Uint8 *)Swin->pixels + y*Swin->pitch + x1*3;
		pEnd = pDst + dx*3;
		while (pDst <= pEnd) {
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
		pDst = (Uint8 *)Swin->pixels + y*Swin->pitch + (x1 << 1);
		pEnd = pDst + (dx << 1);
		while (pDst <= pEnd) {
			*(Uint16 *)pDst = c;
			pDst += 2;
		}
		break;
	default:
		pDst = (Uint8 *)Swin->pixels + y*Swin->pitch + x1;
		pEnd = pDst + dx;
		memset(pDst, c, pEnd-pDst);
		break;
	}
}

static void
SDL2FB_DrawLineV(void *_Nonnull obj, int x, int y1, int y2,
    const AG_Color *_Nonnull C)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint8 *pDst, *pEnd;
	Uint32 c;
	int dy;

	if (ClipVertLine(sfb, &x, &y1, &y2, &dy))
		return;

	c = SDL_MapRGB(Swin->format,
	    AG_Hto8(C->r),
	    AG_Hto8(C->g),
	    AG_Hto8(C->b));

	switch (Swin->format->BytesPerPixel) {
	case 4:
		pDst = (Uint8 *)Swin->pixels + y1*Swin->pitch + (x << 2);
		pEnd = pDst + dy*Swin->pitch;
		while (pDst < pEnd) {
			*(Uint32 *)pDst = c;
			pDst += Swin->pitch;
		}
		break;
	case 3:
		pDst = (Uint8 *)Swin->pixels + y1*Swin->pitch + x*3;
		pEnd = pDst + dy*Swin->pitch;
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
			pDst += Swin->pitch;
		}
		break;
	case 2:
		pDst = (Uint8 *)Swin->pixels + y1*Swin->pitch + (x << 1);
		pEnd = pDst + dy*Swin->pitch;
		while (pDst < pEnd) {
			*(Uint16 *)pDst = c;
			pDst += Swin->pitch;
		}
		break;
	default:
		pDst = (Uint8 *)Swin->pixels + y1*Swin->pitch + x;
		pEnd = pDst + dy*Swin->pitch;
		while (pDst < pEnd) {
			*(Uint8 *)pDst = c;
			pDst += Swin->pitch;
		}
		break;
	}
}

static void
SDL2FB_DrawLineBlended(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull c, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = (dy << 1) - dx;
		inc1 = (dy << 1);
		inc2 = (dy-dx) << 1;
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
		SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);
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
				SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);
			}
		}		
	} else {
		d = (dx << 1) - dy;
		inc1 = (dx << 1);
		inc2 = (dx-dy) << 1;
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
		SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);
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
				SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);
			}
		}
	}
}

static void
SDL2FB_DrawLineW(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull color, float width)
{
	/* TODO */
	(void)width;
	SDL2FB_DrawLine(obj, x1,y1, x2,y2, color);
}

static void
SDL2FB_DrawLineW_Sti16(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull color, float width, Uint16 mask)
{
	/* TODO */
	(void)width;
	(void)mask;
	SDL2FB_DrawLine(obj, x1,y1, x2,y2, color);
}

static void
SDL2FB_DrawTriangle_FlatBot(void *_Nonnull obj, const AG_Pt *_Nonnull v1,
    const AG_Pt *_Nonnull v2, const AG_Pt *_Nonnull v3,
    const AG_Color *_Nonnull c)
{
	float invslope1 = (v2->x - v1->x) / (v2->y - v1->y);
	float invslope2 = (v3->x - v1->x) / (v3->y - v1->y);
	float x1 = v1->x;
	float x2 = v1->x;
	int y;

	for (y = v1->y; y <= v2->y; y++) {
		SDL2FB_DrawLineH(obj, x1,x2, y, c);
		x1 += invslope1;
		x2 += invslope2;
	}
}

static void
SDL2FB_DrawTriangle_FlatTop(void *_Nonnull obj, const AG_Pt *_Nonnull v1,
    const AG_Pt *_Nonnull v2, const AG_Pt *_Nonnull v3,
    const AG_Color *_Nonnull c)
{
	float invslope1 = (v3->x - v1->x) / (v3->y - v1->y);
	float invslope2 = (v3->x - v2->x) / (v3->y - v2->y);
	float x1 = v3->x;
	float x2 = v3->x;
	int y;

	for (y = v3->y; y <= v1->y; y--) {
		SDL2FB_DrawLineH(obj, x1,x2, y, c);
		x1 -= invslope1;
		x2 -= invslope2;
	}
}

static void
SDL2FB_DrawTriangle(void *_Nonnull obj, const AG_Pt *v1, const AG_Pt *v2,
    const AG_Pt *v3, const AG_Color *_Nonnull c)
{
	AG_Pt V1, V2, V3;

	/* Sort the three vertices by y coordinate ascending. */
	if (v3->y > v2->y &&
	    v3->y > v1->y) {
		V3 = *v3;
		if (v2->y > v1->y) {
			V2=*v2; V1=*v1;
		} else {
			V2=*v1; V1=*v2;
		}
	} else if (v2->y > v1->y &&
	           v2->y > v3->y) {
		V3 = *v2;
		if (v3->y > v1->y) {
			V2=*v3; V1=*v1;
		} else {
			V2=*v1; V1=*v3;
		}
	} else {
		V3 = *v1;
		if (v3->y > v2->y) {
			V2=*v3; V1=*v2;
		} else {
			V2=*v2; V1=*v3;
		}
	}
	v1 = &V1;
	v2 = &V2;
	v3 = &V3;

	if (v2->y == v3->y) {
		SDL2FB_DrawTriangle_FlatBot(obj, v1,v2,v3, c);
	} else if (v1->y == v2->y) {
		SDL2FB_DrawTriangle_FlatTop(obj, v1,v2,v3, c);
	} else {
		AG_Pt v4;

		v4.x = (int)(v1->x + ((float)(v2->y - v1->y) /
		                      (float)(v3->y - v1->y)) * (v3->x - v1->x));
		v4.y = v2->y;
		SDL2FB_DrawTriangle_FlatBot(obj, v1,v2,&v4, c);
		SDL2FB_DrawTriangle_FlatTop(obj, v2,&v4,v3, c);
	}
}

static void
SDL2FB_DrawPolygon(void *obj, const AG_Pt *pts, Uint nPts, const AG_Color *c)
{
	AG_Widget *wid = WIDGET(obj);
	AG_DriverSDL2FB *sfb = (AG_DriverSDL2FB *)wid->drv;
	int y, x1, y1, x2, y2;
	int miny, maxy;
	int i, i1, i2;
	Uint nPolyInts;

	/* Allocate/resize the array of intersections. */
	if (sfb->polyInts == NULL) {
		sfb->nPolyInts = nPts;
		sfb->polyInts = Malloc(nPts*sizeof(int));
	} else {
		if (nPts > sfb->nPolyInts) {
			sfb->nPolyInts = nPts;
			sfb->polyInts = Realloc(sfb->polyInts, nPts*sizeof(int));
		}
	}

	/* Find Y maxima */
	miny = pts[0].y;
	maxy = miny;
	for (i=1; i < nPts; i++) {
		int vy;
	
		vy = pts[i].y;
		if (vy < miny) {
			miny = vy;
		} else if (vy > maxy) {
			maxy = vy;
		}
	}

	/* Find the intersections. */
	for (y = miny; y <= maxy; y++) {
		nPolyInts = 0;
		for (i=0; i < nPts; i++) {
			if (i == 0) {
				i1 = nPts - 1;
				i2 = 0;
			} else {
				i1 = i - 1;
				i2 = i;
			}
			y1 = pts[i1].y;
			y2 = pts[i2].y;
			if (y1 < y2) {
				x1 = pts[i1].x;
				x2 = pts[i2].x;
			} else if (y1 > y2) {
				x2 = pts[i1].x;
				y2 = pts[i1].y;
				x1 = pts[i2].x;
				y1 = pts[i2].y;
			} else {
				continue;
			}
			if (((y >= y1) && (y < y2)) ||
			    ((y == maxy) && (y > y1) && (y <= y2))) {
				sfb->polyInts[nPolyInts++] =
				    (((y - y1) << 16) / (y2 - y1)) *
				     (x2 - x1) + (x1 << 16);
			} 
		}
		qsort(sfb->polyInts, nPolyInts, sizeof(int), CompareInts);

		for (i=0; i < nPolyInts; i+=2) {
			int xa, xb;

			xa = sfb->polyInts[i] + 1;
			xa = (xa >> 16) + ((xa & 0x8000) >> 15);
			xb = sfb->polyInts[i+1] - 1;
			xb = (xb >> 16) + ((xb & 0x8000) >> 15);
			SDL2FB_DrawLineH(sfb, xa,xb, y, c);
		}
	}
}

static void
SDL2FB_DrawPolygon_Sti32(void *_Nonnull obj, const AG_Pt *_Nonnull pts, Uint nPts,
    const AG_Color *_Nonnull c, const Uint8 *_Nonnull stipple)
{
	/* TODO */
	Debug(obj, "drawPolygon_Sti32() not implemented yet\n");
}

static int
CompareInts(const void *_Nonnull p1, const void *_Nonnull p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}

static void
SDL2FB_DrawArrow_Up(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int y1 = y0 - (h >> 1) + 1;
	int y2 = y1 + h-2;
	int xs = x0, xe = xs;
	int x, y;

	for (y = y1; y < y2; ++y) {
		for (x = xs; x <= xe; x++) {
			SDL2FB_PutPixel32(obj, x,y, px);
		}
		xs--;
		xe++;
	}
}
		
static void
SDL2FB_DrawArrow_Right(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int x1 = x0 - (h >> 1) + 1;
	int x2 = x1 + h-2;
	int ys = y0, ye = ys;
	int x, y;
	
	for (x = x2; x > x1; --x) {
		for (y = ys; y <= ye; y++) {
			SDL2FB_PutPixel32(obj, x,y, px);
		}
		ys--;
		ye++;
	}
}

static void
SDL2FB_DrawArrow_Down(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int y1 = y0 - (h >> 1) + 1;
	int y2 = y1 + h-2;
	int xs = x0, xe = xs;
	int x, y;
	
	for (y = y2; y > y1; --y) {
		for (x = xs; x <= xe; x++) {
			SDL2FB_PutPixel32(obj, x,y, px);
		}
		xs--;
		xe++;
	}
}
		
static void
SDL2FB_DrawArrow_Left(void *_Nonnull obj, int x0, int y0, int h, Uint32 px)
{
	int x1 = x0 - (h >> 1) + 1;
	int x2 = x1 + h-2;
	int ys = y0, ye = ys;
	int x, y;
	
	for (x = x1; x < x2; ++x) {
		for (y = ys; y <= ye; y++) {
			SDL2FB_PutPixel32(obj, x,y, px);
		}
		ys--;
		ye++;
	}
}

static void
SDL2FB_DrawArrow(void *_Nonnull obj, Uint8 angle, int x0, int y0, int h,
    const AG_Color *_Nonnull c)
{
	AG_DriverSDL2FB *sfb = obj;
	static void (*pf[])(void *_Nonnull, int,int, int, Uint32) = {
		SDL2FB_DrawArrow_Up,
		SDL2FB_DrawArrow_Right,
		SDL2FB_DrawArrow_Down,
		SDL2FB_DrawArrow_Left,
	};
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	Uint32 px;
	
	px = SDL_MapRGB(Swin->format,
	    AG_Hto8(c->r),
	    AG_Hto8(c->g),
	    AG_Hto8(c->b));
#ifdef AG_DEBUG
	if (angle >= 4) { AG_FatalError("Bad angle"); }
#endif
	pf[angle](obj, x0,y0, h, px);
}

static void
SDL2FB_DrawBoxRoundedTop(void *_Nonnull obj, const AG_Rect *r, int z, int rad,
    const AG_Color *_Nonnull c1, const AG_Color *_Nonnull c2,
    const AG_Color *_Nonnull c3)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_PixelFormat *pf = Swin->format;
	AG_Rect rd;
	int rx = r->x;
	int ry = r->y;
	int rw = r->w;
	int rh = r->h;
	int x2 = rx + rad;
	int x3 = rx - rad + rw - 1;
	int y2 = ry + rad;
	int v, e, u;
	int x, y, i;
	Uint32 c[3];
	
	c[0] = SDL_MapRGB(pf, c1->r, c1->g, c1->b);
	c[1] = SDL_MapRGB(pf, c2->r, c2->g, c2->b);
	c[2] = SDL_MapRGB(pf, c3->r, c3->g, c3->b);
	
	rd.x = x2;					/* Center rect */
	rd.y = y2;
	rd.w = rw - (rad << 1);
	rd.h = rh - rad;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.y = ry;					/* Top rect */
	rd.h = rh;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.x = rx;					/* Left rect */
	rd.y = y2;
	rd.w = rad;
	rd.h = rh - rad;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.x = rx + rw - rad;				/* Right rect */
	rd.y = y2;
	rd.w = rad;
	rd.h = rh - rad;
	SDL2FB_DrawRectFilled(obj, &rd, c1);

	/* Top, left and right lines */
	SDL2FB_DrawLineH(obj, x2,      rx+rw-rad, ry,    c1);
	SDL2FB_DrawLineV(obj, rx,      y2,        ry+rh, c2);
	SDL2FB_DrawLineV(obj, rx+rw-1, y2,        ry+rh, c3);

	/* Top left and top right rounded edges */
	v = (rad << 1) - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;

	while (x <= y) {
		SDL2FB_PutPixel32(obj, x2-x, y2-y, c[1]);
		SDL2FB_PutPixel32(obj, x2-y, y2-x, c[1]);
		SDL2FB_PutPixel32(obj, x3+x, y2-y, c[2]);
		SDL2FB_PutPixel32(obj, x3+y, y2-x, c[2]);
		for (i = 0; i < x; i++) {
			SDL2FB_PutPixel32(obj, x2-i, y2-y, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y2-y, c[0]);
		}
		for (i = 0; i < y; i++) {
			SDL2FB_PutPixel32(obj, x2-i, y2-x, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y2-x, c[0]);
		}
		e += u;
		u += 2;
		if (v < (e << 1)) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
	}
}

static void
SDL2FB_DrawBoxRounded(void *_Nonnull obj, const AG_Rect *r, int z, int rad,
    const AG_Color *_Nonnull c1, const AG_Color *_Nonnull c2,
    const AG_Color *_Nonnull c3)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_PixelFormat *pf = Swin->format;
	AG_Rect rd;
	Uint32 c[3];
	int v, e, u;
	int x, y, i;
	int rx = r->x, ry = r->y;
	int rw = r->w, rh = r->h;
	int w1 = rw - 1;
	int x2 = rx + rad;
	int y2 = ry + rad;
	int x3 = rx - rad + w1;
	int y3 = ry + rh - rad;
	int rad2 = (rad << 1), rad_2 = (rad >> 1);
	
	if (rw < 4 || rh < 4) {
		return;
	}
	if (rad2 > rw || rad2 > rh)
		rad = MIN(rw >> 1, rh >> 1);
	
	c[0] = SDL_MapRGB(pf, c1->r, c1->g, c1->b);
	c[1] = SDL_MapRGB(pf, c2->r, c2->g, c2->b);
	c[2] = SDL_MapRGB(pf, c3->r, c3->g, c3->b);
	
	rd.x = rx + rad;					/* Center */
	rd.y = ry + rad;
	rd.w = rw - rad2;
	rd.h = rh - rad2;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.y = ry;						/* Top */
	rd.h = rad;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.y = y3;						/* Bottom */
	rd.h = rad;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.x = rx;						/* Left */
	rd.y = ry + rad;
	rd.w = rad;
	rd.h = rh - rad2;
	SDL2FB_DrawRectFilled(obj, &rd, c1);
	rd.x = rx + rw - rad;					/* Right */
	rd.y = ry + rad;
	rd.w = rad;
	rd.h = rh - rad2;
	SDL2FB_DrawRectFilled(obj, &rd, c1);

	/* Rounded edges */
	v = (rad << 1) - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	while (x <= y) {
		SDL2FB_PutPixel32(obj, x2-x, y2-y, c[1]);
		SDL2FB_PutPixel32(obj, x2-y, y2-x, c[1]);
		SDL2FB_PutPixel32(obj, x3+x, y2-y, c[2]);
		SDL2FB_PutPixel32(obj, x3+y, y2-x, c[2]);

		SDL2FB_PutPixel32(obj, x2-x, y3+y, c[1]);
		SDL2FB_PutPixel32(obj, x2-y, y3+x, c[1]);
		SDL2FB_PutPixel32(obj, x3+x, y3+y, c[2]);
		SDL2FB_PutPixel32(obj, x3+y, y3+x, c[2]);

		for (i = 0; i < x; i++) {
			SDL2FB_PutPixel32(obj, x2-i, y2-y, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y2-y, c[0]);
			SDL2FB_PutPixel32(obj, x2-i, y3+y, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y3+y, c[0]);
		}
		for (i = 0; i < y; i++) {
			SDL2FB_PutPixel32(obj, x2-i, y2-x, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y2-x, c[0]);
			SDL2FB_PutPixel32(obj, x2-i, y3+x, c[0]);
			SDL2FB_PutPixel32(obj, x3+i, y3+x, c[0]);
		}
		e += u;
		u += 2;
		if (v < (e >> 1)) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
	}
	
	/* Contour lines */
	SDL2FB_DrawLineH(obj, x2,       rx+rw-rad,   ry,    c1);
	SDL2FB_DrawLineH(obj, rx+rad_2, rx+rw-rad_2, ry,    c2);
	SDL2FB_DrawLineH(obj, rx+rad_2, rx+rw-rad_2, ry+rh, c3);
	SDL2FB_DrawLineV(obj, rx,       y2,          y3,    c2);
	SDL2FB_DrawLineV(obj, rx+w1,    y2,          y3,    c3);
}

static void
SDL2FB_DrawCircle(void *_Nonnull obj, int x1, int y1, int radius,
    const AG_Color *_Nonnull C)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	int v = (radius << 1) - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;
	Uint32 c;

	c = SDL_MapRGB(Swin->format,
	    AG_Hto8(C->r),
	    AG_Hto8(C->g),
	    AG_Hto8(C->b));

	while (x < y) {
		SDL2FB_PutPixel32(obj, x1+x, y1+y, c);
		SDL2FB_PutPixel32(obj, x1+x, y1-y, c);
		SDL2FB_PutPixel32(obj, x1-x, y1+y, c);
		SDL2FB_PutPixel32(obj, x1-x, y1-y, c);
		e += u;
		u += 2;
		if (v < (e << 1)) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		SDL2FB_PutPixel32(obj, x1+y, y1+x, c);
		SDL2FB_PutPixel32(obj, x1+y, y1-x, c);
		SDL2FB_PutPixel32(obj, x1-y, y1+x, c);
		SDL2FB_PutPixel32(obj, x1-y, y1-x, c);
	}
	SDL2FB_PutPixel32(obj, x1-radius, y1, c);
	SDL2FB_PutPixel32(obj, x1+radius, y1, c);
}

static void
SDL2FB_DrawCircleFilled(void *_Nonnull obj, int x1, int y1, int radius,
    const AG_Color *_Nonnull c)
{
	int v = (radius << 1) - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		SDL2FB_DrawLineV(obj, x1+x, y1+y, y1-y, c);
		SDL2FB_DrawLineV(obj, x1-x, y1+y, y1-y, c);

		e += u;
		u += 2;
		if (v < (e << 1)) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		SDL2FB_DrawLineV(obj, x1+y, y1+x, y1-x, c);
		SDL2FB_DrawLineV(obj, x1-y, y1+x, y1-x, c);
	}
}

static void
SDL2FB_DrawRectFilled(void *_Nonnull obj, const AG_Rect *r,
    const AG_Color *_Nonnull c)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	SDL_Rect rd;

	rd.x = (Sint16)r->x;
	rd.y = (Sint16)r->y;
	rd.w = (Uint16)r->w;
	rd.h = (Uint16)r->h;

	SDL_FillRect(Swin, &rd,
	    SDL_MapRGB(Swin->format,
	        AG_Hto8(c->r),
	        AG_Hto8(c->g),
	        AG_Hto8(c->b)));
}

static void
SDL2FB_DrawRectBlended(void *_Nonnull obj, const AG_Rect *r,
    const AG_Color *_Nonnull c, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	int x, y;
	int rx = r->x;
	int ry = r->y;
	int x2 = rx + r->w;
	int y2 = ry + r->h;

	for (y = ry; y < y2; y++) {
		for (x = rx; x < x2; x++) {
			if (ClippedPixel(Swin, x,y)) {
				continue;
			}
			SDL2FB_BlendPixel(obj, x,y, c, fnSrc, fnDst);
		}
	}
}

static void
SDL2FB_DrawRectDithered(void *_Nonnull obj, const AG_Rect *r, const AG_Color *C)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	int ry = r->y;
	int rx = r->x;
	int x2 = rx + r->w - 2;
	int y2 = ry + r->h - 2;
	int x, y;
	int flag = 0;
	Uint32 c;
	
	c = SDL_MapRGB(Swin->format,
	    AG_Hto8(C->r),
	    AG_Hto8(C->g),
	    AG_Hto8(C->b));

	/* XXX inefficient/ugly */
	for (y = ry; y < y2; y++) {
		flag = !flag;
		for (x = rx+1+flag; x < x2; x+=2)
			SDL2FB_PutPixel32(obj, x,y, c);
	}
}

static void
SDL2FB_UpdateGlyph(void *_Nonnull drv, AG_Glyph *_Nonnull G)
{
	/* Nothing to do */
}

static void
SDL2FB_DrawGlyph(void *_Nonnull drv, const AG_Glyph *_Nonnull G, int x, int y)
{
	AG_DriverSDL2FB *sfb = drv;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	
	AG_SDL2_BlitSurface(G->su, NULL, Swin, x,y);
}

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverSDL2FB *_Nonnull sfb, int wView, int hView)
{
	AG_ClipRect *cr;

	/* Rectangle 0 always covers the whole view. */
	if ((sfb->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	cr = &sfb->clipRects[0];
	cr->r.x = 0;
	cr->r.y = 0;
	cr->r.w = wView;
	cr->r.h = hView;
	sfb->nClipRects = 1;
	return (0);
}

/*
 * Single-display specific operations.
 */

static int
SDL2FB_OpenVideo(void *_Nonnull obj, Uint w, Uint h, int depth, Uint flags)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDL2FB *sfb = obj;
	AG_PixelFormat *pf;
	SDL_Surface *Swin;
	SDL_WindowFlags swFlags = SDL_WINDOW_RESIZABLE;
	int numVideoDisplays, i;

	numVideoDisplays = SDL_GetNumVideoDisplays();
	Debug(sfb, "SDL: %d displays\n", numVideoDisplays);
	for (i = 0; i < numVideoDisplays; i++) {
		Debug(sfb, "SDL Display %d: %s\n", i, SDL_GetDisplayName(i));
	}

	if (flags & AG_VIDEO_FULLSCREEN) {
		swFlags |= SDL_WINDOW_FULLSCREEN;
		dsw->flags |= AG_DRIVER_SW_FULLSCREEN;
	}
	if (flags & AG_VIDEO_OPENGL)      { swFlags |= SDL_WINDOW_OPENGL; }
	if (flags & AG_VIDEO_NOFRAME)     { swFlags |= SDL_WINDOW_BORDERLESS; }
 	if (flags & AG_VIDEO_FIXED)       { swFlags &= ~(SDL_WINDOW_RESIZABLE); }

	if (flags & AG_VIDEO_OVERLAY)     { dsw->flags |= AG_DRIVER_SW_OVERLAY; }
	if (flags & AG_VIDEO_BGPOPUPMENU) { dsw->flags |= AG_DRIVER_SW_BGPOPUP; }

	/* Set the video mode. Force hardware palette in 8bpp. */
	AG_SDL2_GetPrefDisplaySettings(drv, &w, &h, &depth);

#ifdef DEBUG_DISPLAY
	Debug(sfb, "Opened display (%u x %u x %d bpp)\n", w, h, depth);
#endif
	sfb->window = SDL_CreateWindow(agProgName,
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    w, h, swFlags);
	if (sfb->window == NULL) {
		AG_SetError("SDL_CreateWindow(%d,%d, 0x%x): %s", w, h, swFlags,
		    SDL_GetError());
		return (-1);
	}

	Swin = SDL_GetWindowSurface(sfb->window);
	pf = drv->videoFmt = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatRGBA(pf,
	    Swin->format->BitsPerPixel,
	    Swin->format->Rmask,
	    Swin->format->Gmask,
	    Swin->format->Bmask,
	    Swin->format->Amask);

	dsw->w = Swin->w;
	dsw->h = Swin->h;
	dsw->depth = Swin->format->BitsPerPixel;

#ifdef DEBUG_DISPLAY
	Debug(sfb, "New display (%d x %d x %d bpp)\n",
	    Swin->w, Swin->h, Swin->format->BitsPerPixel);
#endif
	/* Initialize clipping rectangles. */
	if (InitClipRects(sfb, dsw->w, dsw->h) == -1)
		goto fail;
	
	/* Create the cursors. */
	AG_SDL2_InitDefaultCursor(sfb);
	AG_InitStockCursors(drv);

	/* Set background color. */
	SDL_FillRect(Swin, NULL, SDL_MapRGB(Swin->format,
	    AG_Hto8(dsw->bgColor.r),
	    AG_Hto8(dsw->bgColor.g),
	    AG_Hto8(dsw->bgColor.b)));

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
SDL2FB_OpenVideoContext(void *_Nonnull obj, void *_Nonnull ctx, Uint flags)
{
	AG_SetError("openVideoContext() not implemented yet");
	return (-1);
}

static void
SDL2FB_CloseVideo(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;

	SDL_DestroyWindow(sfb->window);
	sfb->window = NULL;

	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
SDL2FB_VideoResize(void *_Nonnull obj, Uint w, Uint h)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDL2FB *sfb = obj;
	AG_ClipRect *cr0;
	int wRet, hRet;

	Debug(sfb, "VideoResize request (%u x %u)\n", w,h);
	SDL_SetWindowSize(sfb->window, w,h);
	SDL_GetWindowSize(sfb->window, &wRet, &hRet);
	dsw->w = wRet;
	dsw->h = hRet;
	Debug(sfb, "VideoResize returned (%u x %u)\n", wRet, hRet);

	/* Update clipping rectangle 0. */
	cr0 = &sfb->clipRects[0];
	cr0->r.w = w;
	cr0->r.h = h;

	return (0);
}

static AG_Surface *
SDL2FB_VideoCapture(void *_Nonnull obj)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);

	return AG_SDL2_ImportSurface(Swin);
}

static void
SDL2FB_VideoClear(void *_Nonnull obj, const AG_Color *c)
{
	AG_DriverSDL2FB *sfb = obj;
	SDL_Surface *Swin = SDL_GetWindowSurface(sfb->window);
	AG_Rect rd;

	SDL_FillRect(Swin, NULL,
	    SDL_MapRGB(Swin->format,
	        AG_Hto8(c->r),
		AG_Hto8(c->g),
		AG_Hto8(c->b)));

	rd.x = 0;
	rd.y = 0;
	rd.w = Swin->w;
	rd.h = Swin->h;
	SDL2FB_UpdateRegion(sfb, &rd);
}

static int
SDL2FB_SetVideoContext(void *_Nonnull obj, void *_Nonnull pSurface)
{
	AG_SetError("setVideoContext() not implemented yet");
	return (-1);
}

AG_DriverSwClass agDriverSDL2FB = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDL2FB",
			sizeof(AG_DriverSDL2FB),
			{ 1,7 },
			Init,
			NULL,		/* reset */
			Destroy,
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"sdl2fb",
		AG_FRAMEBUFFER,
		AG_WM_SINGLE,
		AG_DRIVER_SDL2,
		SDL2FB_Open,
		SDL2FB_Close,
		AG_SDL2_GetDisplaySize,
		AG_SDL2_BeginEventProcessing,
		AG_SDL2_PendingEvents,
		AG_SDL2_GetNextEvent,
		AG_SDL2_ProcessEvent_SW,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDL2FB_BeginRendering,
		SDL2FB_RenderWindow,
		SDL2FB_EndRendering,
		SDL2FB_FillRect,
		SDL2FB_UpdateRegion,
		NULL,				/* uploadTexture */
		SDL2FB_UpdateTexture,
		SDL2FB_DeleteTexture,
		AG_SDL2_SetRefreshRate,
		SDL2FB_PushClipRect,
		SDL2FB_PopClipRect,
		SDL2FB_PushBlendingMode,
		SDL2FB_PopBlendingMode,
		AG_SDL2_CreateCursor,
		AG_SDL2_FreeCursor,
		AG_SDL2_SetCursor,
		AG_SDL2_UnsetCursor,
		AG_SDL2_GetCursorVisibility,
		AG_SDL2_SetCursorVisibility,
		SDL2FB_BlitSurface,
		SDL2FB_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		SDL2FB_BlitSurfaceGL,
		SDL2FB_BlitSurfaceFromGL,
		SDL2FB_BlitSurfaceFlippedGL,
#endif
		NULL,				/* backupSurfaces */
		NULL,				/* restoreSurfaces */
		SDL2FB_RenderToSurface,
		SDL2FB_PutPixel,
		SDL2FB_PutPixel32,
		SDL2FB_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		SDL2FB_PutPixel64,
		SDL2FB_PutPixelRGB16,
#endif
		SDL2FB_BlendPixel,
		SDL2FB_DrawLine,
		SDL2FB_DrawLineH,
		SDL2FB_DrawLineV,
		SDL2FB_DrawLineBlended,
		SDL2FB_DrawLineW,
		SDL2FB_DrawLineW_Sti16,
		SDL2FB_DrawTriangle,
		SDL2FB_DrawPolygon,
		SDL2FB_DrawPolygon_Sti32,
		SDL2FB_DrawArrow,
		SDL2FB_DrawBoxRounded,
		SDL2FB_DrawBoxRoundedTop,
		SDL2FB_DrawCircle,
		SDL2FB_DrawCircleFilled,
		SDL2FB_DrawRectFilled,
		SDL2FB_DrawRectBlended,
		SDL2FB_DrawRectDithered,
		SDL2FB_UpdateGlyph,
		SDL2FB_DrawGlyph,
		NULL,				/* deleteList */
		NULL,				/* getClipboardText */
		NULL,				/* setClipboardText */
		NULL				/* setMouseAutoCapture */
	},
	0,
	SDL2FB_OpenVideo,
	SDL2FB_OpenVideoContext,
	SDL2FB_SetVideoContext,
	SDL2FB_CloseVideo,
	SDL2FB_VideoResize,
	SDL2FB_VideoCapture,
	SDL2FB_VideoClear
};
