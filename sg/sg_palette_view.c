/*
 * Copyright (c) 2013-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Visualization widget for SG_Palette(3) objects.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/* Create a new SG_PaletteView widget. */
SG_PaletteView	*
SG_PaletteViewNew(void *parent, SG_Palette *pal, Uint flags)
{
	SG_PaletteView *pv;

	pv = Malloc(sizeof(SG_PaletteView));
	AG_ObjectInit(pv, &sgPaletteViewClass);
	pv->flags |= flags;
	pv->pal = pal;

	if (flags & SG_PALETTE_VIEW_HFILL) { WIDGET(pv)->flags |= AG_WIDGET_HFILL; }
	if (flags & SG_PALETTE_VIEW_VFILL) { WIDGET(pv)->flags |= AG_WIDGET_VFILL; }

	AG_ObjectAttach(parent, pv);
	return (pv);
}

static void
OnOverlay(AG_Event *_Nonnull event)
{
	SG_PaletteView *pv = AG_SELF();
	AG_Surface *su;
	char text[128];
	
	AG_Snprintf(text, sizeof(text), "%s", OBJECT(pv)->name);
	AG_TextColor(&WCOLOR(pv, TEXT_COLOR));
	if ((su = AG_TextRender(text)) != NULL) {
		AG_WidgetBlit(pv, su, 0, HEIGHT(pv) - su->h);
		AG_SurfaceFree(su);
	}
}

#if 0
static void
ViewKeyDown(AG_Event *_Nonnull event)
{
	SG_PaletteView *pv = AG_PTR(1);
	const int keysym = AG_INT(2);
	const int kmod = AG_INT(3);
	
	switch (keysym) {
	case AG_KEY_LEFT:
		break;
	case AG_KEY_RIGHT:
		break;
	case AG_KEY_UP:
		break;
	case AG_KEY_DOWN:
		break;
	}
	pv->lastKeyDown = keysym;
}

static void
ViewKeyUp(AG_Event *_Nonnull event)
{
	SG_PaletteView *pv = AG_PTR(1);
	const int keysym = AG_INT(2);

	switch (keysym) {
	case AG_KEY_LEFT:
	case AG_KEY_RIGHT:
	case AG_KEY_UP:
	case AG_KEY_DOWN:
		if (keysym == pv->lastKeyDown) {
			AG_DelTimeout(pv, &pv->toRot);
		}
		break;
	}
}

static void
ViewButtonDown(AG_Event *_Nonnull event)
{
	SG_PaletteView *pv = AG_PTR(1);
	const int button = AG_INT(2);
	const int x = AG_INT(3);
	const int y = AG_INT(4);

	if (!AG_WidgetIsFocused(pv)) {
		AG_WidgetFocus(pv);
	}
	switch (button) {
	case AG_MOUSE_LEFT:
		if ((AG_GetModState(pv) & AG_KEYMOD_CTRL) == 0) {
			SelectByMouse(pv, x,y);
		}
		break;
	}
}

static void
ViewButtonUp(AG_Event *_Nonnull event)
{
	SG_PaletteView *pv = AG_PTR(1);
	const int button = AG_INT(2);

	switch (button) {
	case AG_MOUSE_LEFT:
		break;
	default:
		break;
	}
}
#endif /* 0 */

static void
OnShow(AG_Event *_Nonnull event)
{
	SG_View *sv = SG_VIEW_SELF();
	SG_PaletteView *pv = AG_SELF();

	SG_ViewTransition(sv, pv->sg, NULL, 0);
}

static void
Init(void *_Nonnull obj)
{
	SG_PaletteView *pv = obj;
	
	WIDGET(pv)->flags |= AG_WIDGET_USE_OPENGL;

	pv->flags = 0;
	pv->pal = NULL;
	if ((pv->sg = SG_New(NULL, NULL, 0)) == NULL)
		AG_FatalError(NULL);
	
	AG_AddEvent(pv, "widget-shown", OnShow, NULL);
	AG_SetEvent(pv, "widget-overlay", OnOverlay, NULL);
}

static void
Destroy(void *_Nonnull obj)
{
	SG_PaletteView *pv = obj;

	AG_ObjectDestroy(pv->sg);
}

AG_WidgetClass sgPaletteViewClass = {
	{
		"AG_Widget:SG_View:SG_PaletteView",
		sizeof(SG_PaletteView),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	NULL,			/* sizeReq */
	NULL			/* sizeAlloc */
};
