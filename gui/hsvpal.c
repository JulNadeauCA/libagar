/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#if defined(AG_WIDGETS)

#include <agar/gui/gui.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/primitive.h>
#include <agar/gui/numerical.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/icons.h>

#include <string.h>

#ifdef AG_THREADS
static AG_Mutex CopyLock = AG_MUTEX_INITIALIZER;	/* Copy buffer lock */
#endif
static float cH = 0.0, cS = 0.0, cV = 0.0, cA = 0.0;	/* Copy buffer */

AG_HSVPal *
AG_HSVPalNew(void *parent, Uint flags)
{
	AG_HSVPal *pal;

	pal = Malloc(sizeof(AG_HSVPal));
	AG_ObjectInit(pal, &agHSVPalClass);
	pal->flags |= flags;

	if (flags & AG_HSVPAL_HFILL) { AG_ExpandHoriz(pal); }
	if (flags & AG_HSVPAL_VFILL) { AG_ExpandVert(pal); }

	AG_ObjectAttach(parent, pal);
	return (pal);
}

/* Return the 8-bit representation of the current alpha value. */
static __inline__ Uint8
GetAlpha8(AG_HSVPal *_Nonnull pal)
{
	AG_Variable *bAlpha;
	Uint8 a = 255;
	void *p;

	bAlpha = AG_GetVariable(pal, "alpha", &p);
	switch (AG_VARIABLE_TYPE(bAlpha)) {
	case AG_VARIABLE_FLOAT:
		a = (Uint8)((*(float *)p)*255.0f);
		break;
	case AG_VARIABLE_DOUBLE:
		a = (Uint8)((*(double *)p)*255.0);
		break;
	case AG_VARIABLE_INT:
		a = (int)((*(int *)p));
		break;
	case AG_VARIABLE_UINT8:
		a = (int)((*(Uint8 *)p));
		break;
	default:
		break;
	}
	AG_UnlockVariable(bAlpha);
	return (a);
}
	
static __inline__ void
SetAlpha8(AG_HSVPal *_Nonnull pal, Uint8 a)
{
	AG_Variable *bAlpha;
	void *pAlpha;

	bAlpha = AG_GetVariable(pal, "alpha", &pAlpha);
	switch (AG_VARIABLE_TYPE(bAlpha)) {
	case AG_VARIABLE_FLOAT:
		*(float *)pAlpha = (float)a/255.0f;
		break;
	case AG_VARIABLE_DOUBLE:
		*(double *)pAlpha = (double)a/255.0;
		break;
	case AG_VARIABLE_INT:
		*(int *)pAlpha = (int)a;
		break;
	case AG_VARIABLE_UINT8:
		*(Uint8 *)pAlpha = (Uint8)a;
		break;
	default:
		break;
	}
	AG_UnlockVariable(bAlpha);
}

static __inline__ void
UpdatePixelFromHSVA(AG_HSVPal *_Nonnull pal)
{
	Uint8 r, g, b, a;
	AG_Variable *bColor, *bv;
	AG_Color *pColor;
	void *v;

	AG_HSV2RGB(AG_GetFloat(pal, "hue"),
	           AG_GetFloat(pal, "saturation"),
		   AG_GetFloat(pal, "value"), &r, &g, &b);
	a = GetAlpha8(pal);
	
	if ((bv = AG_AccessVariable(pal, "RGBv")) != NULL) {
		v = bv->data.p;
		switch (AG_VARIABLE_TYPE(bv)) {
		case AG_VARIABLE_FLOAT:
			((float *)v)[0] = (float)r/255.0f;
			((float *)v)[1] = (float)g/255.0f;
			((float *)v)[2] = (float)b/255.0f;
			break;
		case AG_VARIABLE_DOUBLE:
			((double *)v)[0] = (double)r/255.0;
			((double *)v)[1] = (double)g/255.0;
			((double *)v)[2] = (double)b/255.0;
			break;
		case AG_VARIABLE_INT:
			((int *)v)[0] = (int)r;
			((int *)v)[1] = (int)g;
			((int *)v)[2] = (int)b;
			break;
		case AG_VARIABLE_UINT8:
			((Uint8 *)v)[0] = r;
			((Uint8 *)v)[1] = g;
			((Uint8 *)v)[2] = b;
			break;
		default:
			break;
		}
		AG_UnlockVariable(bv);
	}
	if ((bv = AG_AccessVariable(pal, "RGBAv")) != NULL) {
		v = bv->data.p;
		switch (AG_VARIABLE_TYPE(bv)) {
		case AG_VARIABLE_FLOAT:
			((float *)v)[0] = (float)r/255.0;
			((float *)v)[1] = (float)g/255.0;
			((float *)v)[2] = (float)b/255.0;
			((float *)v)[3] = (float)a/255.0;
			break;
		case AG_VARIABLE_DOUBLE:
			((double *)v)[0] = (double)r/255.0;
			((double *)v)[1] = (double)g/255.0;
			((double *)v)[2] = (double)b/255.0;
			((double *)v)[3] = (double)a/255.0;
			break;
		case AG_VARIABLE_INT:
			((int *)v)[0] = (int)r;
			((int *)v)[1] = (int)g;
			((int *)v)[2] = (int)b;
			((int *)v)[3] = (int)a;
			break;
		case AG_VARIABLE_UINT8:
			((Uint8 *)v)[0] = r;
			((Uint8 *)v)[1] = g;
			((Uint8 *)v)[2] = b;
			((Uint8 *)v)[3] = a;
			break;
		default:
			break;
		}
		AG_UnlockVariable(bv);
	}

	if (pal->flags & AG_HSVPAL_PIXEL) {
		AG_PixelFormat **pFormat;
		AG_Variable *bFormat;
		
		if ((bFormat = AG_GetVariable(pal, "pixel-format", &pFormat))) {
			AG_SetUint32(pal, "pixel",
			    AG_MapPixel32_RGBA8(*pFormat, r,g,b,a));
		}
		AG_UnlockVariable(bFormat);
	}

	bColor = AG_GetVariable(pal, "color", &pColor);
	pColor->r = AG_8toH(r);
	pColor->g = AG_8toH(g);
	pColor->b = AG_8toH(b);
	pColor->a = AG_8toH(a);
	AG_UnlockVariable(bColor);

	AG_Redraw(pal);
}

static void
UpdateHSVFromPixel(AG_HSVPal *_Nonnull pal, Uint32 px)
{
	Uint8 r, g, b, a;
	float h, s, v;
	AG_Variable *bFormat;
	AG_PixelFormat **pFormat;
	
	bFormat = AG_GetVariable(pal, "pixel-format", &pFormat);
	if (*pFormat != NULL) {
		AG_GetColor32_RGBA8(px, *pFormat, &r,&g,&b,&a);
		AG_RGB2HSV(r, g, b, &h,&s,&v);
		AG_SetFloat(pal, "hue", h);
		AG_SetFloat(pal, "saturation", s);
		AG_SetFloat(pal, "value", v);
		SetAlpha8(pal, a);
	}
	AG_UnlockVariable(bFormat);
	AG_Redraw(pal);
}

static void
UpdateHSVFromRGBAv(AG_HSVPal *_Nonnull pal)
{
	AG_Variable *bRGBAv;
	void *RGBAv;
	Uint8 r, g, b, a;
	float h, s, v;

	if ((bRGBAv = AG_GetVariable(pal, "RGBAv", &RGBAv)) == NULL) {
		return;
	}
	switch (AG_VARIABLE_TYPE(bRGBAv)) {
	case AG_VARIABLE_FLOAT:
		r = (Uint8)(((float *)RGBAv)[0] * 255.0f);
		g = (Uint8)(((float *)RGBAv)[1] * 255.0f);
		b = (Uint8)(((float *)RGBAv)[2] * 255.0f);
		a = (Uint8)(((float *)RGBAv)[3] * 255.0f);
		break;
	case AG_VARIABLE_DOUBLE:
		r = (Uint8)(((double *)RGBAv)[0] * 255.0);
		g = (Uint8)(((double *)RGBAv)[1] * 255.0);
		b = (Uint8)(((double *)RGBAv)[2] * 255.0);
		a = (Uint8)(((double *)RGBAv)[3] * 255.0);
		break;
	case AG_VARIABLE_INT:
		r = (Uint8)(((int *)RGBAv)[0]);
		g = (Uint8)(((int *)RGBAv)[1]);
		b = (Uint8)(((int *)RGBAv)[2]);
		a = (Uint8)(((int *)RGBAv)[3]);
		break;
	case AG_VARIABLE_UINT8:
		r = ((Uint8 *)RGBAv)[0];
		g = ((Uint8 *)RGBAv)[1];
		b = ((Uint8 *)RGBAv)[2];
		a = ((Uint8 *)RGBAv)[3];
		break;
	default:
		r = 0;
		g = 0;
		b = 0;
		a = 0;
		break;
	}

	AG_RGB2HSV(r,g,b, &h,&s,&v);
	AG_SetFloat(pal, "hue", h);
	AG_SetFloat(pal, "saturation", s);
	AG_SetFloat(pal, "value", v);
	SetAlpha8(pal, a);
	AG_UnlockVariable(bRGBAv);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_Redraw(pal);
}

static void
UpdateHSVFromRGBv(AG_HSVPal *_Nonnull pal)
{
	AG_Variable *bRGBv;
	void *RGBv;
	Uint8 r, g, b;
	float h, s, v;

	if ((bRGBv = AG_GetVariable(pal, "RGBv", &RGBv)) == NULL) {
		return;
	}
	switch (AG_VARIABLE_TYPE(bRGBv)) {
	case AG_VARIABLE_FLOAT:
		r = (Uint8)(((float *)RGBv)[0] * 255.0f);
		g = (Uint8)(((float *)RGBv)[1] * 255.0f);
		b = (Uint8)(((float *)RGBv)[2] * 255.0f);
		break;
	case AG_VARIABLE_DOUBLE:
		r = (Uint8)(((double *)RGBv)[0] * 255.0);
		g = (Uint8)(((double *)RGBv)[1] * 255.0);
		b = (Uint8)(((double *)RGBv)[2] * 255.0);
		break;
	case AG_VARIABLE_INT:
		r = (Uint8)(((int *)RGBv)[0]);
		g = (Uint8)(((int *)RGBv)[1]);
		b = (Uint8)(((int *)RGBv)[2]);
		break;
	case AG_VARIABLE_UINT8:
		r = ((Uint8 *)RGBv)[0];
		g = ((Uint8 *)RGBv)[1];
		b = ((Uint8 *)RGBv)[2];
		break;
	default:
		r = 0;
		g = 0;
		b = 0;
		break;
	}

	AG_RGB2HSV(r,g,b, &h,&s,&v);
	AG_SetFloat(pal, "hue", h);
	AG_SetFloat(pal, "saturation", s);
	AG_SetFloat(pal, "value", v);
	AG_UnlockVariable(bRGBv);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_Redraw(pal);
}

void
AG_HSVPal_UpdateHue(AG_HSVPal *_Nonnull pal, int x, int y)
{
	float h;

	h = Atan2((float)y, (float)x);
	if (h < 0) {
		h += (2.0f*AG_PI);
	}
	AG_SetFloat(pal, "hue", h/(2.0f*AG_PI)*360.0f);

	UpdatePixelFromHSVA(pal);
	AG_PostEvent(pal, "h-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_Redraw(pal);
}

void
AG_HSVPal_UpdateSV(AG_HSVPal *_Nonnull pal, int ax, int ay)
{
	float s, v, hTri;
	int x = ax - pal->triangle.x;
	int y = ay - pal->triangle.y;
	int y_2 = y>>1;

	if (x < -y_2) { x = -y_2; }
	if (x > y_2)  { x =  y_2; }
	if (y > pal->triangle.h-1) { y = pal->triangle.h-1; }

	hTri = (float)pal->triangle.h;
	s = 1.0f - (float)(y) / hTri;
	v = 1.0f - (float)(x + y_2) / hTri;

	if (s < 0.0f)      { s = 0.00001f; }
	else if (s > 1.0f) { s = 1.0f; }
	if (v < 0.0f)      { v = 0.0001f; }
	else if (v > 1.0f) { v = 1.0f; }

	AG_SetFloat(pal, "saturation", s);
	AG_SetFloat(pal, "value", v);

	UpdatePixelFromHSVA(pal);
	AG_PostEvent(pal, "sv-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_Redraw(pal);
}

static void
UpdateAlpha(AG_HSVPal *_Nonnull pal, int x)
{
	AG_Variable *bAlpha;
	void *pAlpha;

	bAlpha = AG_GetVariable(pal, "alpha", &pAlpha);
	switch (AG_VARIABLE_TYPE(bAlpha)) {
	case AG_VARIABLE_FLOAT:
		*(float *)pAlpha = ((float)x)/((float)pal->rPrev.w);
		if (*(float *)pAlpha > 1.0f)      { *(float *)pAlpha = 1.0f; }
		else if (*(float *)pAlpha < 0.0f) { *(float *)pAlpha = 0.0f; }
		break;
	case AG_VARIABLE_DOUBLE:
		*(double *)pAlpha = ((double)x)/((double)pal->rPrev.w);
		if (*(double *)pAlpha > 1.0)      { *(double *)pAlpha = 1.0; }
		else if (*(double *)pAlpha < 0.0) { *(double *)pAlpha = 0.0; }
		break;
	case AG_VARIABLE_INT:
		*(int *)pAlpha = x/pal->rPrev.w;
		if (*(int *)pAlpha > 255)    { *(int *)pAlpha = 255; }
		else if (*(int *)pAlpha < 0) { *(int *)pAlpha = 0; }
		break;
	case AG_VARIABLE_UINT8:
		*(Uint8 *)pAlpha = (Uint8)(x/pal->rPrev.w);
		break;
	default:
		break;
	}
	AG_UnlockVariable(bAlpha);

	UpdatePixelFromHSVA(pal);
	AG_PostEvent(pal, "a-changed", NULL);
	AG_Redraw(pal);
}

static void
CloseMenu(AG_HSVPal *_Nonnull pal)
{
	AG_MenuCollapse(pal->menu_item);

	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
}

#if 0
static void
EditNumValues(AG_Event *event)
{
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);
	AG_Window *pwin;
	AG_Window *win;
	AG_Numerical *num;
	AG_Variable *b1, *b2;
	float v;

	if ((pwin = AG_ParentWindow(pal)) == NULL)
		return;

	if ((win = AG_WindowNewNamed(AG_WINDOW_NOMAXIMIZE, "hsvpal-%p-numedit",
	    pal)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Color values"));
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_LEFT, 0);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	{
		AG_Variable *bAlpha;
		void *pAlpha;

		num = AG_NumericalNew(win, 0, NULL, _("Hue: "));
		AG_NumericalSizeHint(num, "000");
		AG_WidgetCopyBinding(num, "value", pal, "hue");
		AG_NumericalSetRange(num, 0.0, 359.0);
		AG_NumericalSetIncrement(num, 1);
		AG_NumericalSetPrecision(num, "f", 0);
		
		num = AG_NumericalNew(win, 0, NULL, _("Saturation: "));
		AG_NumericalSizeHint(num, "00.00");
		AG_WidgetCopyBinding(num, "value", pal, "saturation");
		AG_NumericalSetRange(num, 0.0, 1.0);
		AG_NumericalSetIncrement(num, 0.01);
		AG_NumericalSetPrecision(num, "f", 2);

		num = AG_NumericalNew(win, 0, NULL, _("Value: "));
		AG_NumericalSizeHint(num, "00.00");
		AG_WidgetCopyBinding(num, "value", pal, "value");
		AG_NumericalSetRange(num, 0.0, 1.0);
		AG_NumericalSetIncrement(num, 0.01);
		AG_NumericalSetPrecision(num, "f", 2);

		num = AG_NumericalNew(win, 0, NULL, _("Alpha: "));
		AG_NumericalSizeHint(num, "0.000");
		AG_WidgetCopyBinding(num, "value", pal, "alpha");
		bAlpha = AG_GetVariable(pal, "alpha", &pAlpha);
		switch (AG_VARIABLE_TYPE(bAlpha)) {
		case AG_VARIABLE_FLOAT:
		case AG_VARIABLE_DOUBLE:
			AG_NumericalSetRange(num, 0.0, 1.0);
			AG_NumericalSetIncrement(num, 0.005);
			AG_NumericalSetPrecision(num, "f", 3);
			break;
		case AG_VARIABLE_INT:
		case AG_VARIABLE_UINT:
		case AG_VARIABLE_UINT8:
			AG_NumericalSetRange(num, 0.0, 255.0);
			AG_NumericalSetIncrement(num, 1.0);
			AG_NumericalSetPrecision(num, "f", 0);
			break;
		default:
			break;
		}
		AG_UnlockVariable(bAlpha);
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}
#endif

static void
SetComplementaryColor(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);
	float hue;

	AG_ObjectLock(pal);
	hue = AG_GetFloat(pal, "hue");
	AG_SetFloat(pal, "hue", ((int)hue+180) % 359);
	UpdatePixelFromHSVA(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_PostEvent(pal, "h-changed", NULL);
	AG_PostEvent(pal, "sv-changed", NULL);
	AG_ObjectUnlock(pal);
	AG_Redraw(pal);
}

static void
CopyColor(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);
	
	AG_ObjectLock(pal);
	AG_MutexLock(&CopyLock);
	cH = AG_GetFloat(pal, "hue");
	cS = AG_GetFloat(pal, "saturation");
	cV = AG_GetFloat(pal, "value");
	cA = AG_GetFloat(pal, "alpha");
	AG_MutexUnlock(&CopyLock);
	AG_ObjectUnlock(pal);
	AG_Redraw(pal);
}

static void
PasteColor(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);

	AG_ObjectLock(pal);

	AG_MutexLock(&CopyLock);
	AG_SetFloat(pal, "hue", cH);
	AG_SetFloat(pal, "saturation", cS);
	AG_SetFloat(pal, "value", cV);
	AG_SetFloat(pal, "alpha", cA);
	AG_MutexUnlock(&CopyLock);
	
	UpdatePixelFromHSVA(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_PostEvent(pal, "h-changed", NULL);
	AG_PostEvent(pal, "sv-changed", NULL);

	AG_ObjectUnlock(pal);
	AG_Redraw(pal);
}

static void
OpenMenu(AG_HSVPal *_Nonnull pal, int x, int y)
{
	if (pal->menu != NULL)
		CloseMenu(pal);

	pal->menu = AG_MenuNew(NULL, 0);
	pal->menu_item = AG_MenuNode(pal->menu->root, NULL, NULL);
	{
#if 0
		AG_MenuAction(pal->menu_item, _("Edit numerically"), NULL,
		    EditNumValues, "%p", pal);
#endif
		AG_MenuAction(pal->menu_item, _("Copy color"), agIconSave.s,
		    CopyColor, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Paste color"), agIconLoad.s,
		    PasteColor, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Complementary color"), NULL,
		    SetComplementaryColor, "%p", pal);
		AG_MenuSeparator(pal->menu_item);
		AG_MenuFlags(pal->menu_item, _("Show RGB value"), agIconMagnifier.s,
		    &pal->flags, AG_HSVPAL_SHOW_RGB, 0);
		AG_MenuFlags(pal->menu_item, _("Show HSV value"), agIconMagnifier.s,
		    &pal->flags, AG_HSVPAL_SHOW_HSV, 0);
	}
	pal->menu->itemSel = pal->menu_item;
	pal->menu_win = AG_MenuExpand(pal, pal->menu_item, x, y);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	int btn = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	float r;

	if (!AG_WidgetIsFocused(pal))
		AG_WidgetFocus(pal);

	switch (btn) {
	case AG_MOUSE_LEFT:
		if (y > pal->rPrev.y) {
			UpdateAlpha(pal, x);
			pal->state = AG_HSVPAL_SEL_A;
		} else {
			x -= pal->circle.x;
			y -= pal->circle.y;
			r = Hypot((float)x, (float)y);

			if (r > (float)pal->circle.rIn) {
				AG_HSVPal_UpdateHue(pal, x,y);
				pal->state = AG_HSVPAL_SEL_H;
			} else {
				AG_HSVPal_UpdateSV(pal, x,y);
				pal->state = AG_HSVPAL_SEL_SV;
			}
		}
		break;
	case AG_MOUSE_MIDDLE:
	case AG_MOUSE_RIGHT:
		OpenMenu(pal, x,y);
		break;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();

	pal->state = AG_HSVPAL_SEL_NONE;
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	switch (pal->state) {
	case AG_HSVPAL_SEL_NONE:
		break;
	case AG_HSVPAL_SEL_H:
		AG_HSVPal_UpdateHue(pal,
		    x - pal->circle.x,
		    y - pal->circle.y);
		break;
	case AG_HSVPAL_SEL_SV:
		AG_HSVPal_UpdateSV(pal, x, y);
		break;
	case AG_HSVPAL_SEL_A:
		UpdateAlpha(pal, x);
		break;
	default:
		break;
	}
}

static void
Bound(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	const AG_Variable *V = AG_PTR(1);

	if (AG_VARIABLE_TYPE(V) == AG_VARIABLE_UINT32 &&
	    strcmp(V->name, "pixel") == 0) {
		pal->flags |= AG_HSVPAL_PIXEL;
		UpdateHSVFromPixel(pal, *(Uint32 *)V->data.p);
	} else if (strcmp(V->name, "RGBAv") == 0) {
		UpdateHSVFromRGBAv(pal);
	} else if (strcmp(V->name, "RGBv") == 0) {
		UpdateHSVFromRGBv(pal);
	} else if (strcmp(V->name, "pixel-format") == 0) {
		if (!(pal->flags & AG_HSVPAL_FORCE_NOALPHA)) {
			AG_PixelFormat **pFormat = (AG_PixelFormat **)V->data.p;
			if ((*pFormat)->Amask != 0) {
				pal->flags &= ~(AG_HSVPAL_NOALPHA);
			} else {
				pal->flags |= AG_HSVPAL_NOALPHA;
			}
		}
	}
	AG_Redraw(pal);
}

static void
OnAttach(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	
	pal->pixel = AG_MapPixel32_RGBA8(agSurfaceFmt, 0,0,0,255);
	AG_ColorRGBA_8(&pal->color, 0,0,0, 255);
	AG_BindPointer(pal, "pixel-format", (void *)&agSurfaceFmt);
}

static void
Init(void *_Nonnull obj)
{
	AG_HSVPal *pal = obj;

	WIDGET(pal)->flags |= AG_WIDGET_FOCUSABLE;

	pal->flags = 0;
	pal->h = 0.0f;
	pal->s = 0.0f;
	pal->v = 0.0f;
	pal->a = 1.0f;
	pal->pixel = 0;
	AG_ColorBlack(&pal->color);
	pal->circle.spacing = 10;
	pal->state = AG_HSVPAL_SEL_NONE;
	pal->surface = NULL;
	pal->surfaceId = -1;
	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
	AG_ColorRGB_8(&pal->cTile, 140,140,140);

	AG_SetEvent(pal, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(pal, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(pal, "mouse-motion", MouseMotion, NULL);
	
	AG_BindFloat(pal, "hue", &pal->h);
	AG_BindFloat(pal, "saturation", &pal->s);
	AG_BindFloat(pal, "value", &pal->v);
	AG_BindFloat(pal, "alpha", &pal->a);
	AG_BindUint32(pal, "pixel", &pal->pixel);
	AG_BindPointer(pal, "color", (void *)&pal->color);
	AG_BindPointer(pal, "pixel-format", NULL);
	
	AG_SetEvent(pal, "bound", Bound, NULL);
	OBJECT(pal)->flags |= AG_OBJECT_BOUND_EVENTS;
	AG_AddEvent(pal, "attached", OnAttach, NULL);
}

static void
RenderPalette(AG_HSVPal *_Nonnull pal)
{
	AG_Surface *S = pal->surface;
	float h, hue, sat, val;
	AG_Color c;
	AG_Pixel px;
	int x, y, i;

	AG_ColorBlack(&c);
	AG_FillRect(S, NULL, &c);
	/* XXX overdraw */

	hue = (AG_GetFloat(pal, "hue")/360.0f) * 2.0f*AG_PI;
	sat =  AG_GetFloat(pal, "saturation");
	val =  AG_GetFloat(pal, "value");

	/* Render the circle of hues. */
	for (h = 0.0f; h < 2.0f*AG_PI; h += pal->circle.dh) {

		AG_MapHSVf_RGB((h / (2.0f*AG_PI)*360.0f), 1.0f, 1.0f,
		               &c.r, &c.g, &c.b);
		c.a = AG_OPAQUE;

		px = AG_MapPixel(&S->format, &c);

		for (i = 0; i < pal->circle.width; i++) {
			x = (pal->circle.rOut - i)*Cos(h);
			y = (pal->circle.rOut - i)*Sin(h);

			AG_SurfacePut(S,
			    pal->circle.x + x,
			    pal->circle.y + y,
			    px);
		}
	}

	/* Render the triangle of saturation and value. */
	for (y = 0; y < pal->triangle.h; y += 2) {
		float sat = (float)(pal->triangle.h - y) /
		            (float)(pal->triangle.h);

		for (x = 0; x < y; x++) {
			AG_MapHSVf_RGB(
			    (hue/(2.0f*AG_PI))*360.0f,
			    sat,
			    1.0f - ((float)x/(float)pal->triangle.h),
			    &c.r, &c.g, &c.b);

			px = AG_MapPixel(&S->format, &c);
			AG_SurfacePut(S,
			    pal->triangle.x + x - (y >> 1),
			    pal->triangle.y + y,
			    px);
			AG_SurfacePut(S,
			    pal->triangle.x + x - (y >> 1),
			    pal->triangle.y + y + 1,
			    px);
		}
	}

	if (!(pal->flags & AG_HSVPAL_NOALPHA)) {
		AG_Rect rd;

		/* Render the color preview. */
		/* XXX overblending */
		for (y = 16; y < pal->rPrev.h+16; y+=8) {
			for (x = 0; x < pal->rPrev.w; x+=16) {
				rd.w = 8;
				rd.h = 8;
				rd.x = pal->rPrev.x+x;
				rd.y = pal->rPrev.y+y;
				AG_FillRect(S, &rd, &pal->cTile);
			}
			y += 8;
			for (x = 8; x < pal->rPrev.w; x+=16) {
				rd.w = 8;
				rd.h = 8;
				rd.x = pal->rPrev.x+x;
				rd.y = pal->rPrev.y+y;
				AG_FillRect(S, &rd, &pal->cTile);
			}
		}

		AG_MapHSVf_RGB((hue/(2.0f*AG_PI))*360.0f, sat, val,
		    &c.r, &c.g, &c.b);

		for (y = pal->rPrev.y+8; y < S->h; y++) {
			for (x = 0, c.a = 0; x < S->w; x++) {
				AG_SurfaceBlendRGB(S, x,y,
				    c.r, c.g, c.b, c.a,
				    AG_ALPHA_SRC);

				c.a = x*AG_OPAQUE/S->w;
			}
		}
	}
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	r->w = 128;
	r->h = 128;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	const int padding = 5;
	AG_HSVPal *pal = obj;

	if (a->w < 32 || a->h < 32) {
		return (-1);
	} else if (a->w < 50 || a->h < 50) {
		pal->circle.width = 2;
		pal->selcircle_r = 2;
	} else if (a->w < 100 || a->h < 100) {
		pal->circle.width = 4;
		pal->selcircle_r = 2;
	} else if (a->w < 200 || a->h < 200) {
		pal->circle.width = 10;
		pal->selcircle_r = 4;
	} else if (a->w < 300 || a->h < 300) {
		pal->circle.width = 20;
		pal->selcircle_r = 4;
	} else {
		pal->circle.width = 30;
		pal->selcircle_r = 6;
	}

	pal->rPrev.x = 0;
	pal->rPrev.y = a->h - 32;
	pal->rPrev.w = a->w;
	pal->rPrev.h = 32;

	pal->circle.rOut = MIN(a->w - padding, a->h - padding);
	pal->circle.rOut = MIN(pal->circle.rOut, (a->h - pal->rPrev.h));
	pal->circle.rOut >>= 1;
	pal->circle.rIn = pal->circle.rOut - pal->circle.width;
	pal->circle.dh = (float)(1.0/(pal->circle.rOut*AG_PI));
	pal->circle.x = a->w >> 1;
	pal->circle.y = (a->h - pal->rPrev.h) >> 1;

	pal->triangle.x = a->w >> 1;
	pal->triangle.y = pal->circle.y+pal->circle.width-pal->circle.rOut;
	pal->triangle.h = pal->circle.rIn*Sin((37.0f  / 360.0f)*(2.0f * AG_PI))
			 -pal->circle.rIn*Sin((270.0f / 360.0f)*(2.0f * AG_PI));
	
	if (pal->surface != NULL) {
		AG_SurfaceResize(pal->surface, a->w, a->h);
	}
	pal->flags |= AG_HSVPAL_DIRTY;
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_HSVPal *pal = obj;
	AG_Color c;
	float hue, sat, val;
	Uint8 r, g, b, a;
	int w = WIDTH(pal);
	int x, y, rad;

	if (pal->surface == NULL) {
#ifdef HAVE_OPENGL
		pal->surface = AG_SurfaceStdGL(w, HEIGHT(pal));
#else
		pal->surface = AG_SurfaceRGB(w, HEIGHT(pal),
		                             agSurfaceFmt->BitsPerPixel, 0,
		                             agSurfaceFmt->Rmask,
		                             agSurfaceFmt->Gmask,
		                             agSurfaceFmt->Bmask);
#endif
		pal->surfaceId = AG_WidgetMapSurface(pal, pal->surface);
	}
	if (pal->flags & AG_HSVPAL_DIRTY) {
		pal->flags &= ~(AG_HSVPAL_DIRTY);
		RenderPalette(pal);
		AG_WidgetUpdateSurface(pal, pal->surfaceId);
	}

	hue = (AG_GetFloat(pal, "hue") / 360.0f) * 2.0f*AG_PI;
	sat =  AG_GetFloat(pal, "saturation");
	val =  AG_GetFloat(pal, "value");
	a = (Uint8)(AG_GetFloat(pal, "alpha")*255.0f);

	AG_WidgetBlitFrom(pal, pal->surfaceId, NULL, 0, 0);

	/* Indicate the current selection. */
	/* TODO xor */
	rad = pal->circle.rIn + (pal->circle.width >> 1);
	AG_ColorBlack(&c);
	AG_DrawCircle(pal,
	    pal->circle.x + rad*Cos(hue),
	    pal->circle.y + rad*Sin(hue),
	    pal->selcircle_r,
	    &c);
	
	/* The rendering routine uses (v = 1 - x/h), so (x = -v*h + h). */
	y = (int)((1.0 - sat) * (float)pal->triangle.h);
	x = (int)(-(val*(float)pal->triangle.h - (float)pal->triangle.h));
	if (x < 0) { x = 0; }
	if (x > y) { x = y; }
	AG_DrawCircle(pal,
	    pal->triangle.x + x - y/2,
	    pal->triangle.y + y,
	    pal->selcircle_r,
	    &c);

	AG_MapHSVf_RGB8((hue * 360.0f)/(2.0f*AG_PI), sat, val, &r,&g,&b);

	/* Draw the color preview rectangle. */
	if (!(pal->flags & AG_HSVPAL_NOPREVIEW)) {
		AG_Rect rPrev = pal->rPrev;

		rPrev.h >>= 1;
		AG_ColorRGB_8(&c, r,g,b);
		AG_DrawRectFilled(pal, &rPrev, &c);
	}

	/* Overlay the slider over the trasparency preview. */
	if (!(pal->flags & AG_HSVPAL_NOALPHA)) {
		int lx  = 1 + pal->rPrev.x + ((a*pal->rPrev.w) >> 8);
		int ly1 = pal->rPrev.y;
		int ly2 = pal->rPrev.y + pal->rPrev.h;

		if (lx > w-2) { lx = w-2; }
		AG_ColorBlack(&c);
		AG_DrawLineV(pal, lx,   ly1, ly2, &c);
		AG_DrawLineV(pal, lx+2, ly1, ly2, &c);
	}

	/* Display RGB/HSV values */
	if (pal->flags & (AG_HSVPAL_SHOW_RGB|AG_HSVPAL_SHOW_HSV)) {
		AG_Surface *s;

		/* XXX inefficient */
		AG_PushTextState();
		AG_TextBGColorRGB(0,0,0);
		AG_TextColorRGB(255,255,255);
		if ((pal->flags & AG_HSVPAL_SHOW_RGB) &&
		    (pal->flags & AG_HSVPAL_SHOW_HSV)) {
			s = AG_TextRenderF(
			    "RGB: %u,%u,%u\n"
			    "HSV: %.02f,%.02f,%.02f",
			    r,g,b, (hue*360.0f) / (2.0f*AG_PI), sat, val);
		} else if (pal->flags & AG_HSVPAL_SHOW_RGB) {
			s = AG_TextRenderF("RGB: %u,%u,%u", r, g, b);
		} else {
			s = AG_TextRenderF("HSV: %.01f,%.02f,%.02f",
			    (hue*360.0f) / (2.0f*AG_PI), sat, val);
		}
		AG_WidgetBlit(pal, s,
		    (w >> 1) - (s->w >> 1),
		    pal->rPrev.y + (pal->rPrev.h >> 1) - (s->h >> 1));
		AG_SurfaceFree(s);
		AG_PopTextState();
	}
}

AG_WidgetClass agHSVPalClass = {
	{
		"Agar(Widget:HSVPal)",
		sizeof(AG_HSVPal),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
