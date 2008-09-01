/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <gui/widget.h>
#include <gui/button.h>
#include <gui/fspinbutton.h>
#include <gui/spinbutton.h>
#include <gui/primitive.h>
#include <gui/text_cache.h>

#include "m.h"
#include "m_matview.h"
#include "m_gui.h"

M_Matview *
M_MatviewNew(void *parent, M_Matrix *M, Uint flags)
{
	M_Matview *mv;

	mv = Malloc(sizeof(M_Matview));
	AG_ObjectInit(mv, &mMatviewClass);
	mv->flags |= flags;
	if (M != NULL) {
		M_MatviewSetMatrix(mv, M);
	}
	AG_ObjectAttach(parent, mv);
	return (mv);
}

void
M_MatviewSetMatrix(M_Matview *mv, M_Matrix *M)
{
	M_MatrixFPU *MFPU = (void *)M;

	if (strcmp(M->ops->name, "scalar") != 0) {
		AG_FatalError("Cannot display %s matrices",
		    M->ops->name);
	}
	AG_ObjectLock(mv);
	mv->matrix = MMATRIX(MFPU);
	mv->mPre = MROWS(MFPU);
	mv->nPre = MCOLS(MFPU);
	AG_WidgetBind(mv->hBar, "max", AG_WIDGET_UINT, &MMATRIX(mv->matrix)->m);
	AG_WidgetBind(mv->vBar, "max", AG_WIDGET_UINT, &MMATRIX(mv->matrix)->n);
	AG_ObjectUnlock(mv);
}

static void
KeyDown(AG_Event *event)
{
	M_Matview *mv = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_g:
		M_MatviewSetDisplayMode(mv, M_MATVIEW_GREYSCALE);
		break;
	case SDLK_n:
		M_MatviewSetDisplayMode(mv, M_MATVIEW_NUMERICAL);
		break;
	case SDLK_EQUALS:
		mv->scale++;
		break;
	case SDLK_MINUS:
		if (mv->scale-1 >= 0) {
			mv->scale--;
		}
		break;
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();

	AG_WidgetFocus(bu);
}

static void
Init(void *obj)
{
	M_Matview *mv = obj;

	WIDGET(mv)->flags |= AG_WIDGET_EXPAND|AG_WIDGET_CLIPPING|
	                     AG_WIDGET_FOCUSABLE;

	mv->mode = M_MATVIEW_NUMERICAL;
	mv->matrix = NULL;
	mv->flags = 0;
	mv->hSpacing = 2;
	mv->vSpacing = 2;
	mv->hBar = AG_ScrollbarNew(mv, AG_SCROLLBAR_HORIZ, 0);
	mv->vBar = AG_ScrollbarNew(mv, AG_SCROLLBAR_VERT, 0);
	mv->xOffs = 0;
	mv->yOffs = 0;
	mv->scale = 8;
	mv->mPre = 0;
	mv->nPre = 0;
	mv->numFmt = "%g";
	mv->tCache = agTextCache ? AG_TextCacheNew(mv, 64, 16) : NULL;
	
	AG_WidgetBind(mv->hBar, "value", AG_WIDGET_UINT, &mv->xOffs);
	AG_WidgetBind(mv->vBar, "value", AG_WIDGET_UINT, &mv->yOffs);
	AG_WidgetSetInt(mv->hBar, "min", 0);
	AG_WidgetSetInt(mv->vBar, "min", 0);

	AG_TextSize("-00", &mv->wEnt, &mv->hEnt);

	AG_SetEvent(mv, "window-keydown", KeyDown, NULL);
	AG_SetEvent(mv, "window-mousebuttondown", MouseButtonDown, NULL);
}

static void
Destroy(void *obj)
{
	M_Matview *mv = obj;

	if (mv->tCache != NULL)
		AG_TextCacheDestroy(mv->tCache);
}

void
M_MatviewSetDisplayMode(M_Matview *mv, enum m_matview_mode mode)
{
	AG_ObjectLock(mv);
	mv->mode = mode;
	AG_ObjectUnlock(mv);
}

void
M_MatviewSetNumericalFmt(M_Matview *mv, const char *fmt)
{
	AG_ObjectLock(mv);
	mv->numFmt = fmt;
	AG_ObjectUnlock(mv);
}

void
M_MatviewSizeHint(M_Matview *mv, const char *text, Uint m, Uint n)
{
	AG_ObjectLock(mv);
	mv->mPre = m;
	mv->nPre = n;
	AG_TextSize(text, &mv->wEnt, &mv->hEnt);
	AG_ObjectUnlock(mv);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	M_Matview *mv = p;

	r->w = mv->nPre*(mv->wEnt + mv->hSpacing) + mv->hSpacing*2;
	r->h = mv->mPre*(mv->hEnt + mv->vSpacing) + mv->vSpacing*2;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	M_Matview *mv = p;
	AG_SizeAlloc aChld;

	aChld.x = 0;
	aChld.y = a->h - mv->hBar->wButton;
	aChld.w = a->w;
	aChld.h = mv->hBar->wButton+1;
	AG_WidgetSizeAlloc(mv->hBar, &aChld);

	aChld.x = a->w - mv->vBar->wButton;
	aChld.y = mv->vBar->wButton;
	aChld.w = mv->vBar->wButton;
	aChld.h = a->h - mv->hBar->wButton+1;
	AG_WidgetSizeAlloc(mv->vBar, &aChld);
	return (0);
}

static void
DrawNumerical(void *p)
{
	char text[8];
	M_Matview *mv = p;
	M_Matrix *M = mv->matrix;
	int m, n;
	int x, y;
	int xMin = 5, xMax = 0;
	int xOffs = -mv->xOffs*mv->wEnt + 8;
	int yOffs = -mv->yOffs*mv->hEnt + 8;

	AG_DrawBox(mv,
	    AG_RECT(0, 0, WIDTH(mv), HEIGHT(mv)), -1,
	    AG_COLOR(BG_COLOR));

	AG_PushTextState();
	AG_TextColor(TEXT_COLOR);

	for (m = 0, y = yOffs;
	     m < MROWS(M) && y < HEIGHT(mv);
	     m++, y += (mv->hEnt + mv->vSpacing)) {
		for (n = 0, x = xOffs;
		     n < MCOLS(M) && x < WIDTH(mv);
		     n++, x += (mv->wEnt + mv->hSpacing)) {
			Snprintf(text, sizeof(text), mv->numFmt, M_Get(M, m, n));
			if (agTextCache) {
				int su = AG_TextCacheInsLookup(mv->tCache,text);
				AG_WidgetBlitSurface(mv, su, x, y);
				xMax = MAX(xMax, x+WSURFACE(mv,su)->w);
			} else {
				AG_Surface *suTmp;
				suTmp = AG_TextRender(text);
				AG_WidgetBlit(mv, suTmp, x, y);
				xMax = MAX(xMax, x+suTmp->w);
				AG_SurfaceFree(suTmp);
			}
			xMin = MIN(xMin, x);
		}
	}
	
	AG_DrawLineV(mv, xMin-2, 2, y, AG_COLOR(TEXT_COLOR));
	AG_DrawLineV(mv, xMax+4, 2, y, AG_COLOR(TEXT_COLOR));
	AG_PopTextState();
}

static void
DrawGreyscale(void *p)
{
	M_Matview *mv = p;
	M_Matrix *A = mv->matrix;
	Uint m, n;
	int x, y;
	M_Real big = 0.0, small = 0.0;
	int xOffs = -mv->xOffs*mv->scale;
	int yOffs = -mv->yOffs*mv->scale;

	AG_DrawBox(mv,
	    AG_RECT(0, 0, WIDTH(mv), HEIGHT(mv)), -1,
	    AG_COLOR(BG_COLOR));

	for (m = 0; m < MROWS(A); m++) {
		for (n = 0; n < MCOLS(A); n++) {
			if (M_Get(A,m,n) == 0.0) { continue; }
			if (M_Get(A,m,n) > big) { big = M_Get(A,m,n); }
			if (M_Get(A,m,n) < small) { small = M_Get(A,m,n); }
		}
	}
	big -= small;

	for (m = 0, y = yOffs;
	     m < MROWS(A) && y < HEIGHT(mv);
	     m++, y += mv->scale) {
		for (n = 0, x = xOffs;
		     n < MCOLS(A) && x < WIDTH(mv);
		     n++, x += mv->scale) {
		     	M_Real dv = M_Get(A,m,n);
			Uint32 c;
			Uint8 v;

			if (dv == 0.0) {
				continue;
			}
			if (dv == HUGE_VAL) {
				c = AG_MapRGB(agVideoFmt, 200,0,0);
			} else {
				if (dv >= 0.0) {
					v = 127 + (Uint8)(dv*127.0/big);
					c = AG_MapRGB(agVideoFmt, v,0,0);
				} else {
					v = 127 + (Uint8)(Fabs(dv)*127.0/big);
					c = AG_MapRGB(agVideoFmt, 0,0,v);
				}
			}
			AG_DrawRectFilled(mv,
			    AG_RECT(x,y,mv->scale,mv->scale),
			    c);
		}
	}
}

static void
Draw(void *obj)
{
	M_Matview *mv = obj;

	if (mv->mode == M_MATVIEW_NUMERICAL) {
		DrawNumerical(mv);
	} else {
		DrawGreyscale(mv);
	}
}

AG_WidgetClass mMatviewClass = {
	{
		"AG_Widget:M_Matview",
		sizeof(M_Matview),
		{ 0,0 },
		Init,
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

