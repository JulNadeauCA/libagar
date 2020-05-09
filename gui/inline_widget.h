/*
 * Copyright (c) 2001-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Return widget's "enabled" status flag.
 * The Widget must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetEnabled(const void *_Nonnull obj)
#else
int
ag_widget_enabled(const void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	return !(wid->flags & AG_WIDGET_DISABLED);
}

#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetDisabled(const void *_Nonnull obj)
#else
int
ag_widget_disabled(const void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	return (wid->flags & AG_WIDGET_DISABLED);
}

/*
 * Return widget's "visible" status flag.
 * The Widget must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetVisible(const void *_Nonnull obj)
#else
int
ag_widget_visible(const void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	return (wid->flags & AG_WIDGET_VISIBLE);
}

/*
 * Test whether a widget (and its parent window) both hold focus.
 * The Widget and agDrivers VFS must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetIsFocused(const void *_Nonnull obj)
#else
int
ag_widget_is_focused(const void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	return ((wid->flags & AG_WIDGET_FOCUSED) &&
                (wid->window == NULL || agWindowFocused == wid->window));
}

/*
 * Test whether a widget is focused in relation to its parent window
 * (which itself may not necessarily be focused). The Widget must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetIsFocusedInWindow(const void *_Nonnull obj)
#else
int
ag_widget_is_focused_in_window(const void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	return (wid->flags & AG_WIDGET_FOCUSED);
}

/*
 * Test whether the point at display coordinates x,y lies within
 * the widget's allocated area.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetArea(const void *_Nonnull obj, int x, int y)
#else
int
ag_widget_area(const void *obj, int x, int y)
#endif
{
	const AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");

	return (x >= wid->rView.x1 && y >= wid->rView.y1 &&
	        x < wid->rView.x2 && y < wid->rView.y2);
}

/*
 * Test whether relative widget coordinates x,y lie within the
 * widget's allocated area.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_WidgetRelativeArea(const void *_Nonnull obj, int x, int y)
#else
int
ag_widget_relative_area(const void *obj, int x, int y)
#endif
{
	AG_OBJECT_ISA(obj, "AG_Widget:*");

	return (x >= 0 &&
	        y >= 0 &&
	        x < WIDTH(obj) &&
		y < HEIGHT(obj));
}

/*
 * Expand to fill all available space in parent container, both
 * horizontally and vertically.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_Expand(void *_Nonnull wid)
#else
void
ag_expand(void *wid)
#endif
{
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

	AGWIDGET(wid)->flags |= AG_WIDGET_EXPAND;

	AG_ObjectUnlock(wid);
}

/*
 * Expand to fill available space in parent container, horizontally.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ExpandHoriz(void *_Nonnull wid)
#else
void
ag_expand_horiz(void *wid)
#endif
{
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

	AGWIDGET(wid)->flags |= AG_WIDGET_HFILL;

	AG_ObjectUnlock(wid);
}

/*
 * Expand to fill available space in parent container, vertically.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ExpandVert(void *_Nonnull wid)
#else
void
ag_expand_vert(void *wid)
#endif
{
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

	AGWIDGET(wid)->flags |= AG_WIDGET_VFILL;

	AG_ObjectUnlock(wid);
}

/*
 * Request that all computed widget coordinates and geometries in the widget's
 * current window be updated as soon as possible.
 *
 * Safe to use even if the widget is not currently attached to a window.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WidgetUpdate(void *_Nonnull obj)
#else
void
ag_widget_update(void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

	wid->flags |= AG_WIDGET_UPDATE_WINDOW;

	AG_ObjectUnlock(wid);
}

/*
 * Push a rectangle onto the stack of clipping rectangles.
 * Must be invoked from GUI rendering context.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PushClipRect(void *_Nonnull obj, const AG_Rect *_Nonnull pr)
#else
void
ag_push_clip_rect(void *obj, const AG_Rect *_Nonnull pr)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);
	AG_Rect r;

	AG_OBJECT_ISA(wid, "AG_Widget:*");

	r.x = wid->rView.x1 + pr->x;
	r.y = wid->rView.y1 + pr->y;
	r.w = pr->w;
	r.h = pr->h;
	wid->drvOps->pushClipRect(wid->drv, &r);
}

/*
 * Pop the last rectangle off the stack of clipping rectangles.
 * Must be invoked from GUI rendering context.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PopClipRect(void *_Nonnull obj)
#else
void
ag_pop_clip_rect(void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");

	wid->drvOps->popClipRect(wid->drv);
}

/*
 * Push the given alpha blending mode onto the stack of blending modes.
 * Must be invoked from GUI rendering context.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PushBlendingMode(void *_Nonnull obj, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
#else
void
ag_push_blending_mode(void *obj, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");

	wid->drvOps->pushBlendingMode(wid->drv, fnSrc, fnDst);
}

/*
 * Pop the last alpha blending mode off of the stack of blending modes.
 * Must be invoked from GUI rendering context.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PopBlendingMode(void *_Nonnull obj)
#else
void
ag_pop_blending_mode(void *obj)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	
	wid->drvOps->popBlendingMode(wid->drv);
}

/*
 * Variant of AG_WidgetMapSurface() that sets the NODUP flag such that
 * the surface is not freed by the widget.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int
AG_WidgetMapSurfaceNODUP(void *_Nonnull obj, AG_Surface *_Nonnull S)
#else
int
ag_widget_map_surface_nodup(void *obj, AG_Surface *S)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);
	int name;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

	if ((name = AG_WidgetMapSurface(wid, S)) != -1)
		wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;

	AG_ObjectUnlock(wid);
	return (name);
}

/*
 * Variant of WidgetReplaceSurface() that sets the NODUP flag.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WidgetReplaceSurfaceNODUP(void *_Nonnull obj, int name, AG_Surface *_Nullable S)
#else
void
ag_widget_replace_surface_nodup(void *obj, int name, AG_Surface *S)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);

#ifdef AG_DEBUG
	if (name < 0 || name >= (int)wid->nSurfaces)
		AG_FatalError("Bad surface handle");
#endif
	AG_WidgetReplaceSurface(wid, name, S);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;

	AG_ObjectUnlock(wid);
}

/*
 * Draw an unmapped surface at given coordinates in the widget's coordinate
 * system. With hardware-accelerated drivers, this operation is slow compared
 * to drawing of mapped surfaces, since a software->hardware copy is done.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WidgetBlit(void *_Nonnull obj, AG_Surface *_Nonnull S, int x, int y)
#else
void
ag_widget_blit(void *obj, AG_Surface *S, int x, int y)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");

	wid->drvOps->blitSurface(wid->drv, wid, S,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y);
}

/*
 * Perform a hardware or software blit from a mapped surface to the display
 * at coordinates relative to the widget, using clipping.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WidgetBlitFrom(void *_Nonnull obj, int s, AG_Rect *_Nullable r, int x, int y)
#else
void
ag_widget_blit_from(void *obj, int s, AG_Rect *r, int x, int y)
#endif
{
	AG_Widget *wid = AGWIDGET(obj);

	AG_OBJECT_ISA(wid, "AG_Widget:*");

#ifdef AG_DEBUG
	if (s == -1 || wid->surfaces[s] == NULL)
		AG_FatalError("Bad surface handle");
#endif
	wid->drvOps->blitSurfaceFrom(wid->drv, wid, s, r,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y);
}

/*
 * Get a pointer to the keyboard state.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int *_Nonnull _Pure_Attribute
AG_GetKeyState(void *_Nonnull obj)
#else
int *
ag_get_key_state(void *obj)
#endif
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);

	return (kbd->keyState);
}

/*
 * Set the keyboard state.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SetKeyState(void *_Nonnull obj, int *_Nonnull ks)
#else
void
ag_set_key_state(void *obj, int *ks)
#endif
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);

	memcpy(kbd->keyState, ks, kbd->keyCount*sizeof(int));
}

/*
 * Return the number of entries in the keyboard state.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_GetKeyCount(void *_Nonnull obj)
#else
int
ag_get_key_count(void *obj)
#endif
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);

	return (kbd->keyCount);
}

/*
 * Return the current modifier key state.
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint _Pure_Attribute
AG_GetModState(void *_Nonnull obj)
#else
Uint
ag_get_mod_state(void *obj)
#endif
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);

	return (kbd->modState);
}

/*
 * Set the modifier key state.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SetModState(void *_Nonnull obj, Uint ms)
#else
void
ag_set_mod_state(void *obj, Uint ms)
#endif
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);

	kbd->modState = ms;
}
