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
 * A dummy (no-op) Agar driver backend.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>

#ifdef AG_EVENT_LOOP
AG_EventSink *_Nullable dummyEventSink = NULL;
AG_EventSink *_Nullable dummyEventSpinner = NULL;
AG_EventSink *_Nullable dummyEventEpilogue = NULL;
#endif

static int nDrivers = 0;		/* Number of drivers open */

/* Driver instance data */
typedef struct ag_driver_dummy {
	struct ag_driver_mw _inherit;	/* AG_DriverMW -> AG_DriverDUMMY */
	Uint flags;			/* Some flags */
	Uint32 _pad;
	AG_Mutex lock;			/* Some locking device */
} AG_DriverDUMMY;

/* Internal cursor data */
typedef struct ag_cursor_dummy {
	struct ag_cursor _inherit;	/* (not an object) */
	int handle;			/* Some internal handle */
	Uint32 _pad;
} AG_CursorDUMMY;

AG_DriverMwClass agDriverDUMMY;

#if 0
#define AGDRIVER_IS_DUMMY(drv) AG_OfClass((drv),"AG_Driver:AG_DriverMw:AG_DriverDUMMY")
#else
#define AGDRIVER_IS_DUMMY(drv) (AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverDUMMY)
#endif

static int  DUMMY_InitGlobals(void);
static void DUMMY_PostResizeCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static void DUMMY_PostMoveCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static int  DUMMY_RaiseWindow(AG_Window *_Nonnull);
static int  DUMMY_SetInputFocus(AG_Window *_Nonnull);
static void DUMMY_SetTransientFor(AG_Window *, AG_Window *_Nullable);

static void
Init(void *_Nonnull obj)
{
	AG_DriverDUMMY *dum = obj;

	/* Are autoplacing features available? */
	/* AG_DriverMw *dmw = obj; */
	/* dmw->flags |= AG_DRIVER_MW_ANYPOS_AVAIL; */

	dum->flags = 0;
	AG_MutexInitRecursive(&dum->lock);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_DriverDUMMY *dum = obj;

	AG_MutexDestroy(&dum->lock);
}

static void
DUMMY_DestroyGlobals(void)
{
	if (nDrivers > 0)
		return;
#ifdef AG_EVENT_LOOP
	AG_DelEventSink(dummyEventSink);         dummyEventSink = NULL;
	AG_DelEventSpinner(dummyEventSpinner);   dummyEventSpinner = NULL;
	AG_DelEventEpilogue(dummyEventEpilogue); dummyEventEpilogue = NULL;
#endif
}

static int
DUMMY_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverDUMMY *dum = obj;

	if (DUMMY_InitGlobals() == -1)
		return (-1);

	nDrivers++;
	Debug(drv, "Open (%s)\n", (spec) ? spec : "");
	if ((drv->mouse = AG_MouseNew(dum, "Dummy mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(dum, "Dummy keyboard")) == NULL)
		goto fail;
	
	/* Driver manages rendering of window background. */
	drv->flags |= AG_DRIVER_WINDOW_BG;

	return (0);
fail:
	if (drv->kbd)   { AG_ObjectDelete(drv->kbd);   drv->kbd = NULL; }
	if (drv->mouse) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	if (--nDrivers == 0) {
		DUMMY_DestroyGlobals();
	}
	return (-1);
}

static void
DUMMY_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	Debug(drv, "Close\n");
	if (drv->kbd) {
		AG_ObjectDetach(drv->kbd);
		AG_ObjectDestroy(drv->kbd);
		drv->kbd = NULL;
	}
	if (drv->mouse) {
		AG_ObjectDetach(drv->mouse);
		AG_ObjectDestroy(drv->mouse);
		drv->mouse = NULL;
	}

	if (--nDrivers == 0)
		DUMMY_DestroyGlobals();
}

static int
DUMMY_GetDisplaySize(Uint *_Nonnull w, Uint *_Nonnull h)
{
	Debug(NULL, "GetDisplaySize (%p,%p)\n", w,h);
	*w = 640;
	*h = 480;
	return (0);
}

static __inline__ int
DUMMY_PendingEvents(void *_Nonnull drvCaller)
{
	/* Are there events waiting? */
	return (0);
}

static int
DUMMY_GetNextEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	/* Translate a low-level event to an AG_DriverEvent ... */
	return (0);
}

static int
DUMMY_ProcessEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	AG_Driver *drv;
	AG_SizeAlloc a;
	int rv = 1;

	/* Process a translated AG_DriverEvent. */

	if (dev->win == NULL ||
	    dev->win->flags & AG_WINDOW_DETACHING)
		return (0);

	AG_LockVFS(&agDrivers);
	drv = WIDGET(dev->win)->drv;

	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(dev->win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel,
		    drv->mouse->btnState);
		AG_MouseCursorUpdate(dev->win,
		     dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(dev->win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(dev->win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		if (agWindowFocused != dev->win) {
			agWindowFocused = dev->win;
			AG_PostEvent(dev->win, "window-gainfocus", NULL);
		}
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (agWindowFocused == dev->win) {
			AG_PostEvent(dev->win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		a.x = dev->data.videoresize.x;
		a.y = dev->data.videoresize.y;
		a.w = dev->data.videoresize.w;
		a.h = dev->data.videoresize.h;
		if (a.x != WIDGET(dev->win)->x || a.y != WIDGET(dev->win)->y) {
			DUMMY_PostMoveCallback(dev->win, &a);
		}
		if (a.w != WIDTH(dev->win) || a.h != HEIGHT(dev->win)) {
			DUMMY_PostResizeCallback(dev->win, &a);
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(dev->win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		dev->win->dirty = 1;
		break;
	default:
		rv = 0;
		break;
	}

	AG_UnlockVFS(&agDrivers);
	return (rv);
}

static void
DUMMY_BeginRendering(void *_Nonnull obj)
{
	/* Begin rendering cycle. */
	Debug(obj, "BeginRendering\n");
}

static void
DUMMY_RenderWindow(AG_Window *_Nonnull win)
{
	Debug(win, "RenderWindow\n");
	AG_WidgetDraw(win);
}

static void
DUMMY_EndRendering(void *_Nonnull obj)
{
	/* Finalize rendering cycle */
	Debug(obj, "EndRendering\n");
}

static void
DUMMY_FillRect(void *_Nonnull obj, const AG_Rect *_Nonnull r, const AG_Color *_Nonnull c)
{
	Debug(obj, "FillRect(%d,%d:%dx%d, %x:%x:%x:%x)\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_UploadTexture(void *_Nonnull obj, Uint *_Nonnull rv,
    AG_Surface *_Nonnull S, AG_TexCoord *_Nullable tc)
{
	Debug(obj, "StdUploadTexture([%dx%d %d-bpp])\n", S->w, S->h,
	    S->format.BitsPerPixel);
	
	/* Upload texture and return handle... */
	
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)S->w;
		tc->h = (float)S->h;
	}
	*rv = 0;
}

static void
DUMMY_UpdateTexture(void *_Nonnull obj, Uint texture, AG_Surface *_Nonnull S,
    AG_TexCoord *_Nullable tc)
{
	Debug(obj, "UpdateTexture(#%u, [%dx%d %d bpp])\n", texture,
	    S->w, S->h, S->format.BitsPerPixel);

	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)S->w;
		tc->h = (float)S->h;
	}
}

static void
DUMMY_DeleteTexture(void *_Nonnull obj, Uint texture)
{
	Debug(obj, "DeleteTexture(#%d)\n", texture);
}

/*
 * Clipping and blending control (rendering context)
 */
static void
DUMMY_PushClipRect(void *_Nonnull obj, const AG_Rect *_Nonnull r)
{
	Debug(obj, "PushClipRect([%d,%d %dx%d])\n", r->x, r->y, r->w, r->h);
}

static void
DUMMY_PopClipRect(void *_Nonnull obj)
{
	Debug(obj, "PopClipRect\n");
}

static void
DUMMY_PushBlendingMode(void *_Nonnull obj, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	Debug(obj, "PushBlendingMode(%d, %d)\n", fnSrc, fnDst);
}

static void
DUMMY_PopBlendingMode(void *_Nonnull obj)
{
	Debug(obj, "PopBlendingMode\n");
}

static void
DUMMY_BlitSurface(void *_Nonnull obj, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull S, int x, int y)
{
	Debug(obj, "BlitSurface(%s, [%dx%d %d-bpp], %d,%d)\n",
	    OBJECT(wid)->name, S->w, S->h, S->format.BitsPerPixel, x,y);
}

static void
DUMMY_BlitSurfaceFrom(void *_Nonnull obj, AG_Widget *_Nonnull wid, int name,
    const AG_Rect *_Nullable r, int x, int y)
{
	if (r) {
		Debug(obj, "BlitSurfaceFrom(%s:#%d, [%d,%d %dx%d], %d,%d)\n",
		    OBJECT(wid)->name, name, r->x, r->y, r->w, r->h, x,y);
	} else {
		Debug(obj, "BlitSurfaceFrom(%s:#%d, %d,%d)\n",
		    OBJECT(wid)->name, name, x,y);
	}
}

#ifdef HAVE_OPENGL
static void
DUMMY_BlitSurfaceGL(void *_Nonnull obj, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull S, float w, float h)
{
	Debug(obj, "BlitSurfaceGL(%s, [%dx%d %d-bpp], %fx%f)\n",
	    OBJECT(wid)->name, S->w, S->h, S->format.BitsPerPixel, w,h);
}

static void
DUMMY_BlitSurfaceFromGL(void *_Nonnull obj, AG_Widget *_Nonnull wid, int name,
    float w, float h)
{
	Debug(obj, "BlitSurfaceFromGL(%s:#%d, %fx%f)\n", OBJECT(wid)->name,
	    name, w,h);
}

static void
DUMMY_BlitSurfaceFlippedGL(void *_Nonnull obj, AG_Widget *_Nonnull wid, int name,
    float w, float h)
{
	Debug(obj, "BlitSurfaceFlippedGL(%s:#%d, %fx%f)\n", OBJECT(wid)->name,
	    name, w,h);
}
#endif /* HAVE_OPENGL */

static void
DUMMY_BackupSurfaces(void *_Nonnull obj, AG_Widget *_Nonnull wid)
{
	Debug(obj, "BackupSurfaces(%s)\n", OBJECT(wid)->name);
}

static void
DUMMY_RestoreSurfaces(void *_Nonnull obj, AG_Widget *_Nonnull wid)
{
	Debug(obj, "RestoreSurfaces(%s)\n", OBJECT(wid)->name);
}

static int
DUMMY_RenderToSurface(void *_Nonnull obj, AG_Widget *_Nonnull wid,
    AG_Surface *_Nonnull *_Nullable S)
{
	Debug(obj, "RenderToSurface(%s)\n", OBJECT(wid)->name);
	AG_SetError("Unimplemented");
	return (-1);
}

static void
DUMMY_PutPixel(void *_Nonnull obj, int x, int y, const AG_Color *_Nonnull c)
{
	Debug(obj, "PutPixel(%d,%d, [%x:%x:%x:%x])\n", x,y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_PutPixel32(void *_Nonnull obj, int x, int y, Uint32 px)
{
	Debug(obj, "PutPixel32(%d,%d, 0x%x)\n", x,y, px);
}

static void
DUMMY_PutPixelRGB8(void *_Nonnull obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	Debug(obj, "PutPixelRGB8(%d,%d, [%d,%d,%d])\n", x,y, r,g,b);
}

#if AG_MODEL == AG_LARGE
static void
DUMMY_PutPixel64(void *_Nonnull obj, int x, int y, Uint64 px)
{
	Debug(obj, "PutPixel64(%d,%d, 0x%llx)\n", x,y, (unsigned long long)px);
}

static void
DUMMY_PutPixelRGB16(void *_Nonnull obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
{
	Debug(obj, "PutPixelRGB16(%d,%d, [%x:%x:%x])\n", x,y, r,g,b);
}
#endif /* HAVE_64BIT */

static void
DUMMY_BlendPixel(void *_Nonnull obj, int x, int y, const AG_Color *_Nonnull c,
    AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	Debug(obj, "BlendPixel(%d,%d, [%x:%x:%x:%x], %d,%d)\n", x,y,
	    c->r, c->g, c->b, c->a,
	    fnSrc, fnDst);
}

static void
DUMMY_DrawLine(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawLine([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x1,y1, x2,y2,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLineH(void *_Nonnull obj, int x1, int x2, int y,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawLineH([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x1,y, x2,y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLineV(void *_Nonnull obj, int x, int y1, int y2,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawLineV([%d,%d -> %d,%d], [%x:%x:%x:%x])\n", x,y1, x,y2,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawLineBlended(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull c, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	Debug(obj, "DrawLineBlended([%d,%d -> %d,%d], [%x:%x:%x:%x], %d,%d)\n",
	    x1,y1, x2,y2,
	    c->r, c->g, c->b, c->a,
	    fnSrc, fnDst);
}

static void
DUMMY_DrawLineW(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull c, float width)
{
	Debug(obj, "DrawLineW([%d,%d -> %d,%d], [%x:%x:%x:%x], R%.02f)\n",
	    x1,y1, x2,y2, c->r, c->g, c->b, c->a, width);
}

static void
DUMMY_DrawLineW_Sti16(void *_Nonnull obj, int x1, int y1, int x2, int y2,
    const AG_Color *_Nonnull c, float width, Uint16 mask)
{
	Debug(obj, "DrawLineW([%d,%d -> %d,%d], [%x:%x:%x:%x], R%.02f, Sti 0x%x)\n",
	    x1,y1, x2,y2, c->r, c->g, c->b, c->a, width, mask);
}

static void
DUMMY_DrawTriangle(void *_Nonnull obj, const AG_Pt *_Nonnull v1,
    const AG_Pt *_Nonnull v2, const AG_Pt *_Nonnull v3,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawTriangle([%d,%d], [%d,%d], [%d,%d], [%x:%x:%x:%x])\n",
	    v1->x, v1->y, v2->x, v2->y, v3->x, v3->y,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawPolygon(void *_Nonnull obj, const AG_Pt *_Nonnull pts, Uint nPts,
    const AG_Color *_Nonnull c)
{
#ifdef AG_DEBUG
	Uint i;
	Debug(obj, "DrawPolygon(%u, [%x:%x:%x:%x],", nPts, c->r, c->g, c->b, c->a);
	for (i = 0; i < nPts; i++) { Debug(NULL, " (%d,%d)", pts[i].x, pts[i].y); }
	Debug(NULL, ")\n");
#endif
}

static void
DUMMY_DrawPolygon_Sti32(void *_Nonnull obj, const AG_Pt *_Nonnull pts, Uint nPts,
    const AG_Color *_Nonnull c, const Uint8 *_Nonnull stipplePattern)
{
#ifdef AG_DEBUG
	Uint i;
	Debug(obj, "DrawPolygon(%u, [%x:%x:%x:%x], %p, ", nPts, c->r, c->g, c->b, c->a,
	    stipplePattern);
	for (i = 0; i < nPts; i++) { Debug(NULL, " (%d,%d)", pts[i].x, pts[i].y); }
	Debug(NULL, ")\n");
#endif
}

static void
DUMMY_DrawArrow(void *_Nonnull obj, Uint8 angle, int x0, int y0, int h,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawArrow(%d, %d,%d, %d, [%x:%x:%x:%x])\n", angle, x0,y0, h,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawBoxRounded(void *_Nonnull obj, const AG_Rect *_Nonnull r, int z,
    int radius, const AG_Color *_Nonnull c1, const AG_Color *_Nonnull c2,
    const AG_Color *_Nonnull c3)
{
	Debug(obj, "DrawBoxRounded([%d,%d, %dx%d], %d, R%d, [%x:%x:%x:%x])\n",
	    r->x, r->y, r->w, r->h,
	    z, radius,
	    c1->r, c1->g, c1->b, c1->a);
}

static void
DUMMY_DrawBoxRoundedTop(void *_Nonnull obj, const AG_Rect *_Nonnull r, int z,
    int radius, const AG_Color *_Nonnull c1, const AG_Color *_Nonnull c2,
    const AG_Color *_Nonnull c3)
{
	Debug(obj, "DrawBoxRoundedTop([%d,%d, %dx%d], %d, R%d, [%x:%x:%x:%x])\n",
	    r->x, r->y, r->w, r->h,
	    z, radius,
	    c1->r, c1->g, c1->b, c1->a);
}

static void
DUMMY_DrawCircle(void *_Nonnull obj, int x, int y, int radius,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawCircle(%d,%d, %d, [%x:%x:%x:%x])\n", x,y, radius,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawCircleFilled(void *_Nonnull obj, int x, int y, int radius,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawCircleFilled(%d,%d, %d, [%x:%x:%x:%x])\n", x,y, radius,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawRectFilled(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawRectFilled([%d,%d %dx%d], [%x:%x:%x:%x])\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_DrawRectBlended(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull c, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	Debug(obj, "DrawRectBlended([%d,%d %dx%d], [%x:%x:%x:%x], %d,%d)\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a,
	    fnSrc, fnDst);
}

static void
DUMMY_DrawRectDithered(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull c)
{
	Debug(obj, "DrawRectDithered([%d,%d %dx%d], [%x:%x:%x:%x])\n",
	    r->x, r->y, r->w, r->h,
	    c->r, c->g, c->b, c->a);
}

static void
DUMMY_UpdateGlyph(void *_Nonnull obj, AG_Glyph *_Nonnull gl)
{
	Debug(obj, "UpdateGlyph(%s, [%x:%x:%x:%x], '%c', %p)\n",
	    OBJECT(gl->font)->name,
	    gl->color.r, gl->color.g, gl->color.b, gl->color.a,
	    (char)gl->ch, gl->su);
}

static void
DUMMY_DrawGlyph(void *_Nonnull obj, const AG_Glyph *_Nonnull gl, int x, int y)
{
	Debug(obj, "DrawGlyph(%s, [%x:%x:%x:%x], '%c', [%dx%d %d-bpp], %d,%d)\n",
	    OBJECT(gl->font)->name,
	    gl->color.r, gl->color.g, gl->color.b, gl->color.a,
	    (char)gl->ch, gl->su->w, gl->su->h, gl->su->format.BitsPerPixel,
	    x,y);
}

static void
DUMMY_DeleteList(void *_Nonnull obj, Uint name)
{
	Debug(obj, "DeleteList(#%d)\n", name);
}

/*
 * Cursor operations
 */

static AG_Cursor *
DUMMY_CreateCursor(void *_Nonnull obj, Uint w, Uint h, const Uint8 *_Nonnull data,
    const Uint8 *_Nonnull mask, int xHot, int yHot)
{
	AG_Cursor *ac;
	AG_CursorDUMMY *acDUMMY;
	const Uint size = w*h;
	
	Debug(obj, "CreateCursor (%ux%u, %p,%p, %d,%d)\n", w,h, data, mask, xHot, yHot);

	if ((acDUMMY = TryMalloc(sizeof(AG_CursorDUMMY))) == NULL)
		return (NULL);
	/*
	 * Generic part
	 */
	ac = AGCURSOR(acDUMMY);
	if ((ac->data = TryMalloc(size)) == NULL) {
		goto fail;
	}
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		goto fail;
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;
	/*
	 * Driver-specific part
	 */
	acDUMMY->handle = 1234;
	return (ac);
fail:
	free(ac);
	return (NULL);
}

static void
DUMMY_FreeCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
#ifdef AG_DEBUG
	AG_Driver *drv = obj;
	Debug(drv, "FreeCursor (%p)\n", ac);
#endif
	free(ac);
}

static int
DUMMY_SetCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;
	
	Debug(drv, "SetCursor (%p)\n", ac);

	if (drv->activeCursor == ac)
		return (0);

	/* Change cursor ... */

	drv->activeCursor = ac;
	return (0);
}

static void
DUMMY_UnsetCursor(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	
	Debug(drv, "UnsetCursor (%p)\n", obj);
	
	if (drv->activeCursor == TAILQ_FIRST(&drv->cursors))
		return;

	/* Revert to default cursor... */

	drv->activeCursor = TAILQ_FIRST(&drv->cursors);
}

static int
DUMMY_GetCursorVisibility(void *_Nonnull obj)
{
	/* Determine cursor visibility... */
	Debug(obj, "GetCursorVisibility\n");
	return (1);
}

static void
DUMMY_SetCursorVisibility(void *_Nonnull obj, int flag)
{
	/* Set cursor visibility... */
	Debug(obj, "SetCursorVisibility (%d)\n", flag);
}

/*
 * Window operations
 */

static void
InitDefaultCursor(AG_Driver *_Nonnull drv)
{
	AG_Cursor *ac;
	AG_CursorDUMMY *acDUMMY;
	
	acDUMMY = Malloc(sizeof(AG_CursorDUMMY));
	acDUMMY->handle = 1;

	/* Initialize the default cursor. */
	ac = AGCURSOR(acDUMMY);
	AG_CursorInit(ac);
	TAILQ_INSERT_HEAD(&drv->cursors, ac, cursors);
	drv->nCursors++;
}

static int
DUMMY_OpenWindow(AG_Window *_Nonnull win, const AG_Rect *_Nonnull r,
    int depthReq, Uint mwFlags)
{
	AG_Driver *drv = WIDGET(win)->drv;
/*	AG_DriverDUMMY *dum = (AG_DriverDUMMY *)drv; */

	if (depthReq == 0)
		depthReq = 32;

	Debug(drv, "OpenWindow (%s, [%d,%d:%dx%d], %d-bpp, 0x%x)\n",
	    OBJECT(win)->name,
	    r->x, r->y, r->w, r->h,
	    depthReq, mwFlags);

	/* Set the "recommended" standard pixel format. */
	if ((drv->videoFmt = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (-1);
	}
#if AG_MODEL == AG_LARGE
	if (depthReq == 48) {				/* Deep color */
# if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0xffff000000000000,
			0x0000ffff00000000,
			0x00000000ffff0000);
# else
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0x000000000000ffff,
			0x00000000ffff0000,
			0x0000ffff00000000);
# endif
	} else
#endif /* AG_LARGE */
	{						/* True Color */
#if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0xff000000,
			0x00ff0000,
			0x0000ff00);
#else
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0x000000ff,
			0x0000ff00,
			0x00ff0000);
#endif
	}

	/* Create the built-in cursors. */
	InitDefaultCursor(drv);
	AG_InitStockCursors(drv);
	return (0);
}

static void
DUMMY_CloseWindow(AG_Window *_Nonnull win)
{
#ifdef AG_DEBUG
	AG_Driver *drv = WIDGET(win)->drv;
	Debug(drv, "CloseWindow (%s)\n", OBJECT(win)->name);
#endif
}

static int
DUMMY_MapWindow(AG_Window *_Nonnull win)
{
#ifdef AG_DEBUG
	AG_DriverDUMMY *dum = (AG_DriverDUMMY *)WIDGET(win)->drv;

	Debug(win, "MapWindow (%s)\n", OBJECT(dum)->name);
#endif
	return (0);
}

static int
DUMMY_UnmapWindow(AG_Window *_Nonnull win)
{
#ifdef AG_DEBUG
	AG_DriverDUMMY *dum = (AG_DriverDUMMY *)WIDGET(win)->drv;
	Debug(win, "UnmapWindow (%s)\n", OBJECT(dum)->name);
#endif
	return (0);
}

static int
DUMMY_RaiseWindow(AG_Window *_Nonnull win)
{
	Debug(win, "RaiseWindow\n");
	return (0);
}

static int
DUMMY_LowerWindow(AG_Window *_Nonnull win)
{
	Debug(win, "LowerWindow\n");
	return (0);
}

static int
DUMMY_ReparentWindow(AG_Window *_Nonnull win, AG_Window *_Nonnull winParent,
    int x, int y)
{
	Debug(win, "ReparentWindow (%s, %d,%d)\n", OBJECT(winParent)->name, x,y);
	return (0);
}

static int
DUMMY_GetInputFocus(AG_Window *_Nonnull *_Nonnull rv)
{
	AG_SetError("GetInputFocus is unavailable");
	return (-1);
}

static int
DUMMY_SetInputFocus(AG_Window *_Nonnull win)
{
	Debug(win, "SetInputFocus\n");
	return (0);
}

static void
DUMMY_PreResizeCallback(AG_Window *_Nonnull win)
{
	/* Prepare for a window resize */
	Debug(win, "PreResizeCallback\n");
}

static void
DUMMY_PostResizeCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	/* Finalize a window resize operation */
	Debug(win, "PostResizeCallback\n");
}

static void
DUMMY_PostMoveCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	/* Finalize a window move operation */
	Debug(win, "PostMoveCallback (%d,%d, %dx%d)\n",
	    a->x, a->y, a->w, a->h);
}

static int
DUMMY_MoveWindow(AG_Window *_Nonnull win, int x, int y)
{
	Debug(win, "MoveWindow (%d,%d)\n", x,y);
	return (0);
}

static int
DUMMY_ResizeWindow(AG_Window *_Nonnull win, Uint w, Uint h)
{
	Debug(win, "ResizeWindow (%ux%u)\n", w,h);
	return (0);
}

static int
DUMMY_MoveResizeWindow(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	Debug(win, "MoveResizeWindow (%d,%d, %ux%u)\n",
	    a->x, a->y, a->w, a->h);
	return (0);
}

static int
DUMMY_SetBorderWidth(AG_Window *_Nonnull win, Uint width)
{
	Debug(win, "SetBorderWidth (%u)\n", width);
	return (0);
}

static int
DUMMY_SetWindowCaption(AG_Window *_Nonnull win, const char *_Nonnull s)
{
	Debug(win, "SetWindowCaption (\"%s\")\n", s);
	return (0);
}

static void
DUMMY_SetTransientFor(AG_Window *_Nonnull win, AG_Window *_Nullable forParent)
{
	Debug(win, "SetTransientFor (%s)\n",
	    (forParent) ? OBJECT(forParent)->name : "null");
}

static int
DUMMY_SetOpacity(AG_Window *_Nonnull win, float f)
{
	Debug(win, "SetOpacity (%f)\n", f);
	return (0);
}

static void
DUMMY_TweakAlignment(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a,
    Uint wMax, Uint hMax)
{
#ifdef AG_DEBUG
	Debug(win, "TweakAlignment (%s, [%d,%d %dx%d], max=%ux%u)\n",
	    agWindowAlignmentNames[win->alignment],
	    a->x, a->y, a->w, a->h,
	    wMax, hMax);
#endif
}

#ifdef AG_EVENT_LOOP
/*
 * Standard AG_EventLoop() event sink.
 */
static int
DUMMY_EventSink(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
# if 0
	AG_DriverEvent dev;
	/*
	 * Check for events. If there are pending events, process them.
	 */
	while (/* pending events? */ 1) {
		if (DUMMY_GetNextEvent(NULL, &dev) == 1)
			DUMMY_ProcessEvent(NULL, &dev);
	}
# else
	/* Just spin */
	AG_Delay(1);
# endif
	return (1);
}

static int
DUMMY_EventEpilogue(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
/*	AG_DriverEvent dev; */

	/* Finalize event processing... */

	AG_WindowDrawQueued();
	AG_WindowProcessQueued();

	/* Flush any output buffers... */

	return (0);
}
#endif /* AG_EVENT_LOOP */

static int
DUMMY_InitGlobals(void)
{
	if (nDrivers > 0)
		return (0);

	/* Open any global display handles ... */

	/* Query any extensions we might use ... */

	/*
	 * Initialize any other global resource shared between driver
	 * instances (tables, maps, locking devices...)
	 */
#ifdef AG_EVENT_LOOP
# if 0
	/* Set up polling on a file descriptor. */
	{
		int fd = open(...);

		if ((dummyEventSink = AG_AddEventSink(AG_SINK_READ, fd, 0,
		    DUMMY_EventSink, NULL)) == NULL)
			goto fail;
	}
# else
	/* Just spin */
	if ((dummyEventSpinner = AG_AddEventSpinner(DUMMY_EventSink, NULL))
	    == NULL)
		goto fail;
# endif
	/* Set up an event-processing finalization routine */
	if ((dummyEventEpilogue = AG_AddEventEpilogue(DUMMY_EventEpilogue, NULL)) == NULL)
		goto fail;
#endif
	return (0);
fail:
#ifdef AG_EVENT_LOOP
	if (dummyEventSink)     { AG_DelEventSink(dummyEventSink);         dummyEventSink = NULL; }
	if (dummyEventSpinner)  { AG_DelEventSpinner(dummyEventSpinner);   dummyEventSpinner = NULL; }
	if (dummyEventEpilogue) { AG_DelEventEpilogue(dummyEventEpilogue); dummyEventEpilogue = NULL; }
#endif
	return (-1);
}

AG_DriverMwClass agDriverDUMMY = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverDUMMY",
			sizeof(AG_DriverDUMMY),
			{ 1,6 },
			Init,
			NULL,		/* reset */
			Destroy,
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"dummy",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES,
		DUMMY_Open,
		DUMMY_Close,
		DUMMY_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		DUMMY_PendingEvents,
		DUMMY_GetNextEvent,
		DUMMY_ProcessEvent,
		NULL,			/* genericEventLoop */
		NULL,			/* endEventProcessing */
		NULL,			/* terminate */
		DUMMY_BeginRendering,
		DUMMY_RenderWindow,
		DUMMY_EndRendering,
		DUMMY_FillRect,
		NULL,			/* updateRegion */
		DUMMY_UploadTexture,
		DUMMY_UpdateTexture,
		DUMMY_DeleteTexture,
		NULL,			/* setRefreshRate */
		DUMMY_PushClipRect,
		DUMMY_PopClipRect,
		DUMMY_PushBlendingMode,
		DUMMY_PopBlendingMode,
		DUMMY_CreateCursor,
		DUMMY_FreeCursor,
		DUMMY_SetCursor,
		DUMMY_UnsetCursor,
		DUMMY_GetCursorVisibility,
		DUMMY_SetCursorVisibility,
		DUMMY_BlitSurface,
		DUMMY_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		DUMMY_BlitSurfaceGL,
		DUMMY_BlitSurfaceFromGL,
		DUMMY_BlitSurfaceFlippedGL,
#endif
		DUMMY_BackupSurfaces,
		DUMMY_RestoreSurfaces,
		DUMMY_RenderToSurface,
		DUMMY_PutPixel,
		DUMMY_PutPixel32,
		DUMMY_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		DUMMY_PutPixel64,
		DUMMY_PutPixelRGB16,
#endif
		DUMMY_BlendPixel,
		DUMMY_DrawLine,
		DUMMY_DrawLineH,
		DUMMY_DrawLineV,
		DUMMY_DrawLineBlended,
		DUMMY_DrawLineW,
		DUMMY_DrawLineW_Sti16,
		DUMMY_DrawTriangle,
		DUMMY_DrawPolygon,
		DUMMY_DrawPolygon_Sti32,
		DUMMY_DrawArrow,
		DUMMY_DrawBoxRounded,
		DUMMY_DrawBoxRoundedTop,
		DUMMY_DrawCircle,
		DUMMY_DrawCircleFilled,
		DUMMY_DrawRectFilled,
		DUMMY_DrawRectBlended,
		DUMMY_DrawRectDithered,
		DUMMY_UpdateGlyph,
		DUMMY_DrawGlyph,
		DUMMY_DeleteList,
		NULL,				/* getClipboardText */
		NULL				/* setClipboardText */
	},
	DUMMY_OpenWindow,
	DUMMY_CloseWindow,
	DUMMY_MapWindow,
	DUMMY_UnmapWindow,
	DUMMY_RaiseWindow,
	DUMMY_LowerWindow,
	DUMMY_ReparentWindow,
	DUMMY_GetInputFocus,
	DUMMY_SetInputFocus,
	DUMMY_MoveWindow,
	DUMMY_ResizeWindow,
	DUMMY_MoveResizeWindow,
	DUMMY_PreResizeCallback,
	DUMMY_PostResizeCallback,
	DUMMY_SetBorderWidth,
	DUMMY_SetWindowCaption,
	DUMMY_SetTransientFor,
	DUMMY_SetOpacity,
	DUMMY_TweakAlignment
};
