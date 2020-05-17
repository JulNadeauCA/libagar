/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Color Picker widget. It can connect to an AG_Color(3), a packed pixel,
 * RGB component values, or floating-point HSV (Hue/Saturation/Value) and
 * alpha.  It uses floating-point HSV representation internally.
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

static void UpdatePixelFromHSVA(AG_HSVPal *_Nonnull);

AG_HSVPal *
AG_HSVPalNew(void *parent, Uint flags)
{
	AG_HSVPal *pal;

	pal = Malloc(sizeof(AG_HSVPal));
	AG_ObjectInit(pal, &agHSVPalClass);

	if (flags & AG_HSVPAL_HFILL) { WIDGET(pal)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_HSVPAL_VFILL) { WIDGET(pal)->flags |= AG_WIDGET_VFILL; }
	pal->flags |= flags;

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

static void
UpdateHSVFromPixel32(AG_HSVPal *_Nonnull pal, Uint32 px)
{
	Uint8 r, g, b, a;
	float h, s, v;
	AG_Variable *bFormat;
	AG_PixelFormat **pFormat;
	
	bFormat = AG_GetVariable(pal, "pixel-format", (void *)&pFormat);
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

#if AG_MODEL == AG_LARGE
static void
UpdateHSVFromPixel64(AG_HSVPal *_Nonnull pal, Uint64 px)
{
	Uint8 r, g, b, a;
	float h, s, v;
	AG_Variable *bFormat;
	AG_PixelFormat **pFormat;
	
	bFormat = AG_GetVariable(pal, "pixel-format", (void *)&pFormat);
	if (*pFormat != NULL) {
		AG_GetColor64_RGBA8(px, *pFormat, &r,&g,&b,&a);
		AG_RGB2HSV(r, g, b, &h,&s,&v);
		AG_SetFloat(pal, "hue", h);
		AG_SetFloat(pal, "saturation", s);
		AG_SetFloat(pal, "value", v);
		SetAlpha8(pal, a);
	}
	AG_UnlockVariable(bFormat);
	AG_Redraw(pal);
}
#endif /* AG_LARGE */

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

	AG_OBJECT_ISA(pal, "AG_Widget:AG_HSVPal:*");

	h = Atan2((float)y, (float)x);
	if (h < 0) {
		h += 2.0f*(float)AG_PI;
	}
	AG_SetFloat(pal, "hue", h/(2.0f*AG_PI)*360.0f);

	UpdatePixelFromHSVA(pal);
	AG_PostEvent(pal, "h-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
}

void
AG_HSVPal_UpdateSV(AG_HSVPal *_Nonnull pal, int ax, int ay)
{
	float s,v, hTri;
	int x = ax - pal->triangle.x;
	int y = ay - pal->triangle.y;
	int y_2;

	AG_OBJECT_ISA(pal, "AG_Widget:AG_HSVPal:*");

	if (y >= pal->triangle.h) { y = pal->triangle.h-1; }
	y_2 = y >> 1;
	if (x < -y_2) { x = -y_2; }
	if (x > y_2)  { x =  y_2; }

	hTri = (float)pal->triangle.h;
	s = 1.0f - (float)(y) / hTri;
	v = 1.0f - (float)(x + y_2) / hTri;

	if (s < AG_SATURATION_EPSILON) { s = 0.00001f; }
	else if (s > 1.0f)             { s = 1.0f; }

	if (v < AG_VALUE_EPSILON)      { v = 0.0001f; }
	else if (v > 1.0f)             { v = 1.0f; }

	AG_SetFloat(pal, "saturation", s);
	AG_SetFloat(pal, "value", v);

	UpdatePixelFromHSVA(pal);
	AG_PostEvent(pal, "sv-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
UpdatePixelFromHSVA(AG_HSVPal *_Nonnull pal)
{
	Uint8 r,g,b,a;
	AG_Variable *bColor, *bv;
	AG_Color *pColor;
	void *v;

	AG_MapHSVf_RGB8(AG_GetFloat(pal, "hue"),
	                AG_GetFloat(pal, "saturation"),
	                AG_GetFloat(pal, "value"),
	                &r, &g, &b);

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
		
		if ((bFormat = AG_GetVariable(pal, "pixel-format", (void *)&pFormat))) {
			AG_SetUint32(pal, "pixel",
			    AG_MapPixel32_RGBA8(*pFormat, r,g,b,a));
#if AG_MODEL == AG_LARGE
			AG_SetUint64(pal, "pixel64",
			    AG_MapPixel64_RGBA8(*pFormat, r,g,b,a));
#endif
		}
		AG_UnlockVariable(bFormat);
	}

	bColor = AG_GetVariable(pal, "agcolor", (void *)&pColor);
	pColor->r = AG_8toH(r);
	pColor->g = AG_8toH(g);
	pColor->b = AG_8toH(b);
	pColor->a = AG_8toH(a);
	AG_UnlockVariable(bColor);

	AG_Redraw(pal);
}

static void
UpdateAlpha(AG_HSVPal *_Nonnull pal, int x)
{
	AG_Variable *bAlpha;
	void *pAlpha;

	x -= WIDGET(pal)->paddingLeft;

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
	const int btn = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if ((WIDGET(pal)->flags & AG_WIDGET_FOCUSABLE) &&
	    !AG_WidgetIsFocused(pal))
		AG_WidgetFocus(pal);

	switch (btn) {
	case AG_MOUSE_LEFT:
		if ((pal->flags & AG_HSVPAL_NOALPHA) == 0 &&
		    y > pal->rPrev.y) {
			UpdateAlpha(pal, x);
			pal->state = AG_HSVPAL_SEL_A;
		} else {
			const float r = Hypot((float)(x - pal->circle.x),
			                      (float)(y - pal->circle.y));

			if (r > (float)pal->circle.rIn) {
				AG_HSVPal_UpdateHue(pal,
				    x - pal->circle.x,
				    y - pal->circle.y);
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
	AG_Redraw(pal);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	const int x = AG_INT(1);
	const int y = AG_INT(2);

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

static Uint32
KeyMoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);
	const int doHue = (keymod & AG_KEYMOD_SHIFT) ||
	                  (keymod & AG_KEYMOD_CTRL) ||
	                  (keymod & AG_KEYMOD_ALT);
	Uint32 rv = 10;
	float sat, x;

	switch (keysym) {
	case AG_KEY_UP:
		if (doHue) {
	 		x = AG_GetFloat(pal,"hue");
			if ((x -= 2.0f) < 0.0f) {
				x = 360.0f;
			}
			AG_SetFloat(pal, "hue", x);
			goto hue_changed;
		} else {
	 		sat = AG_GetFloat(pal,"saturation");
			if ((sat += 0.015f) > 1.0f) {
				sat = 1.0f;
				rv = 0;
			}
			AG_SetFloat(pal, "saturation", sat);
			goto sv_changed;
		}
		break;
	case AG_KEY_DOWN:
		if (doHue) {
	 		x = AG_GetFloat(pal,"hue");
			if ((x += 2.0f) > 360.0f) {
				x = 0.0f;
			}
			AG_SetFloat(pal, "hue", x);
			goto hue_changed;
		} else {
	 		sat = AG_GetFloat(pal,"saturation");
			if ((sat -= 0.015f) < 0.0f) {
				sat = 0.0f;
				rv = 0;
			}
			AG_SetFloat(pal, "saturation", sat);
			goto sv_changed;
		}
		break;
	case AG_KEY_LEFT:
		if (doHue) {
	 		x = AG_GetFloat(pal,"hue");
			if ((x -= 2.0f) < 0.0f) {
				x = 360.0f;
			}
			AG_SetFloat(pal, "hue", x);
			goto hue_changed;
		} else {
	 		x = AG_GetFloat(pal,"value");
			if ((x += 0.01f) > 1.0f) {
				x = 1.0f;
				rv = 0;
			}
			AG_SetFloat(pal, "value", x);
			goto sv_changed;
		}
		break;
	case AG_KEY_RIGHT:
		if (doHue) {
	 		x = AG_GetFloat(pal,"hue");
			if ((x += 2.0f) > 360.0f) {
				x = 0.0f;
			}
			AG_SetFloat(pal, "hue", x);
			goto hue_changed;
		} else {
	 		x = AG_GetFloat(pal,"value");
			if ((x -= 0.01f) < 0.0f) {
				x = 0.0f;
				rv = 0;
			}
			AG_SetFloat(pal, "value", x);
			goto sv_changed;
		}
		break;
	}
	return (rv);
hue_changed:
	UpdatePixelFromHSVA(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_PostEvent(pal, "h-changed", NULL);
	return (rv);
sv_changed:
	UpdatePixelFromHSVA(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
	AG_PostEvent(pal, "sv-changed", NULL);
	return (rv);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
	case AG_KEY_LEFT:
		AG_AddTimer(pal, &pal->toMove[keysym - AG_KEY_UP], 250,
		    KeyMoveTimeout, "%i,%i", keysym, keymod);
		AG_ExecTimer(&pal->toMove[keysym - AG_KEY_UP]);
		break;
	}

}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_LEFT:
	case AG_KEY_RIGHT:
		AG_DelTimer(pal, &pal->toMove[keysym-AG_KEY_UP]);
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
		UpdateHSVFromPixel32(pal, *(Uint32 *)V->data.p);
#if AG_MODEL == AG_LARGE
	} else if (AG_VARIABLE_TYPE(V) == AG_VARIABLE_UINT64 &&
	    strcmp(V->name, "pixel64") == 0) {
		pal->flags |= AG_HSVPAL_PIXEL;
		UpdateHSVFromPixel64(pal, *(Uint64 *)V->data.p);
#endif
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

	WIDGET(pal)->flags |= AG_WIDGET_FOCUSABLE |
	                      AG_WIDGET_USE_TEXT;

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
	AG_ColorRGB_8(&pal->cTile[0], 140,140,140);
	AG_ColorRGB_8(&pal->cTile[1], 70,70,70);

	AG_InitTimer(&pal->toMove[0], "moveUp", 0);
	AG_InitTimer(&pal->toMove[1], "moveDown", 0);
	AG_InitTimer(&pal->toMove[2], "moveLeft", 0);
	AG_InitTimer(&pal->toMove[3], "moveRight", 0);

	AG_SetEvent(pal, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(pal, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(pal, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(pal, "key-down", KeyDown, NULL);
	AG_SetEvent(pal, "key-up", KeyUp, NULL);
	
	AG_BindFloat(pal, "hue", &pal->h);
	AG_BindFloat(pal, "saturation", &pal->s);
	AG_BindFloat(pal, "value", &pal->v);
	AG_BindFloat(pal, "alpha", &pal->a);
	AG_BindUint32(pal, "pixel", &pal->pixel);
#if AG_MODEL == AG_LARGE
	AG_BindUint64(pal, "pixel64", &pal->pixel64);
#endif
	AG_BindPointer(pal, "agcolor", (void *)&pal->color);
	AG_BindPointer(pal, "pixel-format", NULL);
	
	AG_SetEvent(pal, "bound", Bound, NULL);
	OBJECT(pal)->flags |= AG_OBJECT_BOUND_EVENTS;
	AG_AddEvent(pal, "attached", OnAttach, NULL);
}

/* Render static components (for a given Hue) to a surface. */
static void
RenderStatic(AG_HSVPal *_Nonnull pal)
{
	AG_Surface *S = pal->surface;
	float hue,sat, val, h;
	AG_Color c;
	AG_Pixel px;
	int x, y, i;

	AG_FillRect(S, NULL, &WCOLOR(pal,BG_COLOR));

	hue = AG_GetFloat(pal, "hue");
	sat = AG_GetFloat(pal, "saturation");
	val = AG_GetFloat(pal, "value");

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
			AG_MapHSVf_RGB(hue, sat,
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
#if 0
		AG_Rect rd;
		for (y = 16; y < pal->rPrev.h+16; y+=8) {
			for (x = 0; x < pal->rPrev.w; x+=16) {
				rd.w = 8;
				rd.h = 8;
				rd.x = pal->rPrev.x+x;
				rd.y = pal->rPrev.y+y;
				AG_FillRect(S, &rd, &pal->cTile[0]);
			}
			y += 8;
			for (x = 8; x < pal->rPrev.w; x+=16) {
				rd.w = 8;
				rd.h = 8;
				rd.x = pal->rPrev.x+x;
				rd.y = pal->rPrev.y+y;
				AG_FillRect(S, &rd, &pal->cTile[1]);
			}
		}
#endif
		AG_MapHSVf_RGB(hue, sat, val, &c.r, &c.g, &c.b);

		for (y = pal->rPrev.y;
		     y < pal->rPrev.y + pal->rPrev.h;
		     y++) {
			for (x = pal->rPrev.x, i=0, c.a=0;
			     x < pal->rPrev.x + pal->rPrev.w;
			     x++, i++) {
				AG_SurfaceBlendRGB(S, x,y,
				    c.r, c.g, c.b, c.a,
				    AG_ALPHA_SRC);

				c.a = i*AG_OPAQUE/pal->rPrev.w;
			}
		}
	}
	AG_WidgetUpdateSurface(pal, pal->surfaceId);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_HSVPal *pal = obj;

	r->w = WIDGET(pal)->paddingLeft + 128 + WIDGET(pal)->paddingRight;
	r->h = WIDGET(pal)->paddingTop + 128 + WIDGET(pal)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_HSVPal *pal = obj;
	const int paddingLeft   = WIDGET(pal)->paddingLeft;
	const int paddingRight  = WIDGET(pal)->paddingRight;
	const int paddingTop    = WIDGET(pal)->paddingTop;
	const int paddingBottom = WIDGET(pal)->paddingBottom;
	const int wAvail = a->w - paddingLeft - paddingRight;
	int hPreview = MAX(0, 32 - paddingBottom);

	if (a->w < 32 || a->h < 32) {
		return (-1);
	} else if (a->w < 50 || a->h < 50) {
		pal->circle.width = 4;
		pal->selcircle_r = 3;
		hPreview = 16;
	} else if (a->w < 100 || a->h < 100) {
		pal->circle.width = 6;
		pal->selcircle_r = 2;
	} else if (a->w < 200 || a->h < 200) {
		pal->circle.width = 12;
		pal->selcircle_r = 2;
	} else if (a->w < 300 || a->h < 300) {
		pal->circle.width = 20;
		pal->selcircle_r = 4;
	} else {
		pal->circle.width = 30;
		pal->selcircle_r = 6;
	}

	pal->rPrev.x = paddingLeft;
	pal->rPrev.y = a->h - hPreview - paddingBottom;
	if ((pal->flags & AG_HSVPAL_NOPREVIEW) == 0) {
		pal->rPrev.w = wAvail;
		pal->rPrev.h = hPreview;
	} else {
		pal->rPrev.w = 0;
		pal->rPrev.h = 0;
	}

	pal->circle.rOut = MIN(wAvail, a->h - paddingTop - paddingBottom);
	pal->circle.rOut = MIN(pal->circle.rOut,
	                       (a->h - pal->rPrev.h - paddingTop - paddingBottom));
	pal->circle.rOut >>= 1;
	if (pal->circle.rOut < 16)
		return (-1);

	pal->circle.rIn = pal->circle.rOut - pal->circle.width;

	pal->circle.dh = (float)(1.0/(pal->circle.rOut*AG_PI));

	pal->circle.x = paddingLeft + (wAvail >> 1);
	pal->circle.y = (a->h - pal->rPrev.h) >> 1;

	pal->triangle.x = pal->circle.x;
	pal->triangle.y = pal->circle.y + pal->circle.width - pal->circle.rOut;
	pal->triangle.h = pal->circle.rIn*Sin((37.0f  / 360.0f)*(2.0f * AG_PI))
			 -pal->circle.rIn*Sin((270.0f / 360.0f)*(2.0f * AG_PI));

	pal->flags |= AG_HSVPAL_DIRTY;
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_HSVPal *pal = obj;
	AG_Color c, cInv, cBlack;
	float hueDeg, hueRad, sat, val;
	Uint8 r, g, b, a;
	const int w = WIDTH(pal);
	const int h = HEIGHT(pal);
	int x, y, rad;
	int xSel, ySel;

	if (pal->surface == NULL) {
		pal->surface = AG_SurfaceStdRGBA(w,h);
		pal->surfaceId = AG_WidgetMapSurface(pal, pal->surface);
		RenderStatic(pal);
	} else if (pal->surface->w != w ||                       /* Resized */
	           pal->surface->h != h) {
		pal->surface = AG_SurfaceStdRGBA(w,h);
		RenderStatic(pal);
		AG_WidgetReplaceSurface(pal, pal->surfaceId, pal->surface);
		pal->flags |= AG_HSVPAL_DIRTY;
	} else if (pal->flags & AG_HSVPAL_DIRTY) {
		RenderStatic(pal);
		AG_WidgetUpdateSurface(pal, pal->surfaceId);
	}
	pal->flags &= ~(AG_HSVPAL_DIRTY);
	AG_WidgetBlitFrom(pal, pal->surfaceId, NULL, 0,0);

	hueDeg = AG_GetFloat(pal, "hue");
	hueRad = (hueDeg / 360.0f) * 2.0f*AG_PI;
	sat = AG_GetFloat(pal, "saturation");
	val = AG_GetFloat(pal, "value");
	a = (Uint8)(AG_GetFloat(pal, "alpha")*255.0f);
	AG_MapHSVf_RGB8(hueDeg, sat, val, &r,&g,&b);

	c.r = AG_8toH(r);
	c.g = AG_8toH(g);
	c.b = AG_8toH(b);
	c.a = AG_OPAQUE;

	cInv.r = AG_OPAQUE - c.r;
	cInv.g = AG_OPAQUE - c.g;
	cInv.b = AG_OPAQUE - c.b;
	cInv.a = AG_OPAQUE;

	AG_ColorBlack(&cBlack);

	/* Hue */
	rad = pal->circle.rIn + (pal->circle.width >> 1);
	xSel = pal->circle.x + rad*Cos(hueRad);
	ySel = pal->circle.y + rad*Sin(hueRad);
	AG_DrawCircleFilled(pal, xSel, ySel,
	    pal->selcircle_r + ((pal->state == AG_HSVPAL_SEL_H) ? 2 : 0),
	    &c);
	AG_DrawCircle(pal, xSel, ySel,
	    pal->selcircle_r + 1 + ((pal->state == AG_HSVPAL_SEL_H) ? 2 : 0),
	    &cInv);
	AG_DrawCircle(pal, xSel, ySel,
	    pal->selcircle_r + 2 + ((pal->state == AG_HSVPAL_SEL_H) ? 2 : 0),
	    &cBlack);
	
	/* Saturation and Value */
	/* The rendering routine uses (v = 1 - x/h), so (x = -v*h + h). */
	y = (int)((1.0 - sat) * (float)pal->triangle.h);
	x = (int)(-(val*(float)pal->triangle.h - (float)pal->triangle.h));
	if (x < 0) { x = 0; }
	if (x > y) { x = y; }
	xSel = pal->triangle.x + x - (y >> 1);
	ySel = pal->triangle.y + y;
	AG_DrawCircle(pal, xSel, ySel,
	    pal->selcircle_r + ((pal->state == AG_HSVPAL_SEL_SV) ? 2 : 0),
	    &c);
	AG_DrawCircle(pal, xSel, ySel,
	    pal->selcircle_r + 1 + ((pal->state == AG_HSVPAL_SEL_SV) ? 2 : 0),
	    &cInv);
	AG_DrawCircle(pal, xSel, ySel,
	    pal->selcircle_r + 2 + ((pal->state == AG_HSVPAL_SEL_SV) ? 2 : 0),
	    &cBlack);

	if (!(pal->flags & AG_HSVPAL_NOALPHA)) {    /* Transparency preview */
		int lx  = 1 + pal->rPrev.x + ((a*pal->rPrev.w) >> 8);
		int ly1 = pal->rPrev.y;
		int ly2 = pal->rPrev.y + pal->rPrev.h;

		if (lx > w-2) { lx = w-2; }
		AG_DrawLineV(pal, lx,   ly1, ly2, &WCOLOR(pal, LINE_COLOR));
		AG_DrawLineV(pal, lx+2, ly1, ly2, &WCOLOR(pal, LINE_COLOR));

		AG_DrawRectOutline(pal, &pal->rPrev, &WCOLOR(pal,LINE_COLOR));
	} else {
		if (!(pal->flags & AG_HSVPAL_NOPREVIEW))
			AG_DrawRectFilled(pal, &pal->rPrev, &c);
	}

	/* Display RGB/HSV values */
	if (pal->flags & AG_HSVPAL_SHOW_RGB_HSV) {
		AG_Rect rClip;
		AG_Surface *S;
	
		/* XXX TODO cache rendered text */
		AG_TextBGColor(&c);

		if (w < 80) {
			AG_TextFontLookup("league-gothic-condensed",
			                  WFONT(pal)->spec.size+3.0f, 0);
		} else {
			AG_TextFontLookup("league-gothic",
			                  WFONT(pal)->spec.size+3.0f, 0);
		}
	
		if (sat > 0.66f) {
			if (val < 0.66f || ((hueDeg > 25.0f && hueDeg < 200.0f))) {
				AG_TextColor(&cBlack);
			} else {
				AG_TextColorRGB(255,255,255);
			}
		} else {
			if (val < 0.66f) {
				AG_TextColorRGB(255,255,255);
			} else {
				AG_TextColor(&cBlack);
			}
		}

		if ((pal->flags & AG_HSVPAL_SHOW_RGB) &&
		    (pal->flags & AG_HSVPAL_SHOW_HSV)) {
			S = AG_TextRenderF(
			    " %u  %u  %u \n"
			    " %.1f  %.2f  %.2f ",
			    r,g,b, hueDeg, sat, val);
		} else if (pal->flags & AG_HSVPAL_SHOW_RGB) {
			S = AG_TextRenderF(" %u  %u  %u ", r, g, b);
		} else {
			S = AG_TextRenderF(" %.1f  %.2f  %.2f ",
			                   hueDeg, sat, val);
		}

		if (S->w > w-2) {
			rClip.x = 0;
			rClip.y = 0;
			rClip.w = w-2;
			rClip.h = h-2;
			AG_PushClipRect(pal, &rClip);
		}
		
		AG_WidgetBlit(pal, S,         (w >> 1) - (S->w >> 1),
		    pal->rPrev.y + (pal->rPrev.h >> 1) - (S->h >> 1));
		
		if (S->w > w-2)
			AG_PopClipRect(pal);

		AG_SurfaceFree(S);
	}

	if (AG_WidgetIsFocused(pal))
		AG_DrawRectOutline(pal, &WIDGET(pal)->r,
		    &WCOLOR_FOCUSED(pal,LINE_COLOR));
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
