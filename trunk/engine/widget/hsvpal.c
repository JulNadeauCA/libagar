/*	$Csoft: hsvpal.c,v 1.23 2005/09/27 00:25:22 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/rg/prim.h>

#include "hsvpal.h"

#include <engine/widget/primitive.h>
#include <engine/widget/window.h>
#include <engine/widget/fspinbutton.h>

const AG_WidgetOps agHSVPalOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_HSVPalDraw,
	AG_HSVPalScale
};

static float cH = 0.0, cS = 0.0, cV = 0.0, cA = 0.0;	/* Copy buffer */

static void render_palette(AG_HSVPal *);

AG_HSVPal *
AG_HSVPalNew(void *parent)
{
	AG_HSVPal *pal;

	pal = Malloc(sizeof(AG_HSVPal), M_OBJECT);
	AG_HSVPalInit(pal);
	AG_ObjectAttach(parent, pal);
	return (pal);
}

static __inline__ Uint8
get_alpha8(AG_HSVPal *pal)
{
	AG_WidgetBinding *bAlpha;
	void *pAlpha;
	Uint8 a = 255;

	bAlpha = AG_WidgetGetBinding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case AG_WIDGET_FLOAT:
		a = (Uint8)((*(float *)pAlpha)*255.0);
		break;
	case AG_WIDGET_DOUBLE:
		a = (Uint8)((*(double *)pAlpha)*255.0);
		break;
	case AG_WIDGET_INT:
		a = (int)((*(int *)pAlpha));
		break;
	case AG_WIDGET_UINT8:
		a = (int)((*(Uint8 *)pAlpha));
		break;
	}
	AG_WidgetUnlockBinding(bAlpha);
	return (a);
}
	
static __inline__ void
set_alpha8(AG_HSVPal *pal, Uint8 a)
{
	AG_WidgetBinding *bAlpha;
	void *pAlpha;

	bAlpha = AG_WidgetGetBinding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case AG_WIDGET_FLOAT:
		*(float *)pAlpha = (float)(((float)a)/255.0);
		break;
	case AG_WIDGET_DOUBLE:
		*(double *)pAlpha = (double)(((double)a)/255.0);
		break;
	case AG_WIDGET_INT:
		*(int *)pAlpha = (int)a;
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)pAlpha = (Uint8)a;
		break;
	}
	AG_WidgetUnlockBinding(bAlpha);
}

static __inline__ void
update_pixel_from_hsva(AG_HSVPal *pal)
{
	float h, s, v;
	Uint8 r, g, b, a;
	AG_WidgetBinding *bFormat;
	SDL_PixelFormat **pFormat;

	h = AG_WidgetFloat(pal, "hue");
	s = AG_WidgetFloat(pal, "saturation");
	v = AG_WidgetFloat(pal, "value");
	bFormat = AG_WidgetGetBinding(pal, "pixel-format", &pFormat);
	RG_HSV2RGB(h, s, v, &r, &g, &b);
	AG_WidgetSetUint32(pal, "pixel", SDL_MapRGBA(*pFormat, r, g, b,
	    get_alpha8(pal)));
	AG_WidgetUnlockBinding(bFormat);
}

static __inline__ void
update_hsv_from_pixel(AG_HSVPal *hsv, Uint32 pixel)
{
	Uint8 r, g, b, a;
	float h, s, v;
	AG_WidgetBinding *bFormat;
	SDL_PixelFormat **pFormat;
	
	bFormat = AG_WidgetGetBinding(hsv, "pixel-format", &pFormat);
	SDL_GetRGBA(pixel, *pFormat, &r, &g, &b, &a);
	RG_RGB2HSV(r, g, b, &h, &s, &v);
	AG_WidgetSetFloat(hsv, "hue", h);
	AG_WidgetSetFloat(hsv, "saturation", s);
	AG_WidgetSetFloat(hsv, "value", v);
	set_alpha8(hsv, a);
	AG_WidgetUnlockBinding(bFormat);
}

static __inline__ void
update_h(AG_HSVPal *pal, int x, int y)
{
	float h;

	h = atan2((float)y, (float)x);
	if (h < 0) {
		h += 2*M_PI;
	}
	AG_WidgetSetFloat(pal, "hue", h/(2*M_PI)*360.0);

	update_pixel_from_hsva(pal);
	AG_PostEvent(NULL, pal, "h-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
update_sv(AG_HSVPal *pal, int ax, int ay)
{
	float s, v;
	int x = ax - pal->triangle.x;
	int y = ay - pal->triangle.y;

	if (x < -y/2) { x = -y/2; }
	if (x > y/2) { x = y/2; }
	if (y > pal->triangle.h-1) { y = pal->triangle.h-1; }

	s = 1.0 - (float)y/(float)pal->triangle.h;
	v = 1.0 - (float)(x + y/2)/(float)pal->triangle.h;

	if (s < 0.0) { s = 0.00001; }
	else if (s > 1.0) { s = 1.0; }

	if (v < 0.0) { v = 0.0001; }
	else if (v > 1.0) { v = 1.0; }

	AG_WidgetSetFloat(pal, "saturation", s);
	AG_WidgetSetFloat(pal, "value", v);

	update_pixel_from_hsva(pal);
	AG_PostEvent(NULL, pal, "sv-changed", NULL);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
update_a(AG_HSVPal *pal, int x)
{
	AG_WidgetBinding *bAlpha;
	void *pAlpha;

	bAlpha = AG_WidgetGetBinding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case AG_WIDGET_FLOAT:
		*(float *)pAlpha = ((float)x)/((float)pal->rAlpha.w);
		if (*(float *)pAlpha > 1.0) {
			*(float *)pAlpha = 1.0;
		} else if (*(float *)pAlpha < 0.0) {
			*(float *)pAlpha = 0.0;
		}
		break;
	case AG_WIDGET_DOUBLE:
		*(double *)pAlpha = ((double)x)/((double)pal->rAlpha.w);
		if (*(double *)pAlpha > 1.0) {
			*(double *)pAlpha = 1.0;
		} else if (*(double *)pAlpha < 0.0) {
			*(double *)pAlpha = 0.0;
		}
		break;
	case AG_WIDGET_INT:
		*(int *)pAlpha = x/pal->rAlpha.w;
		if (*(int *)pAlpha > 255) {
			*(int *)pAlpha = 255;
		} else if (*(int *)pAlpha < 0) {
			*(int *)pAlpha = 0;
		}
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)pAlpha = (Uint8)(x/pal->rAlpha.w);
		break;
	}
	AG_WidgetUnlockBinding(bAlpha);

	update_pixel_from_hsva(pal);
	AG_PostEvent(NULL, pal, "a-changed", NULL);
}

static void
close_menu(AG_HSVPal *pal)
{
	AG_MenuCollapse(pal->menu, pal->menu_item);
	AG_ObjectDestroy(pal->menu);
	Free(pal->menu, M_OBJECT);

	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
}

static void
show_rgb(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;
	Uint8 r, g, b;
	float h, s, v;
	
	h = AG_WidgetFloat(pal, "hue");
	s = AG_WidgetFloat(pal, "saturation");
	v = AG_WidgetFloat(pal, "value");

	RG_HSV2RGB(h, s, v, &r, &g, &b);

	AG_TextMsg(AG_MSG_INFO, "%.2f,%.2f,%.2f -> %u,%u,%u", h, s, v, r, g, b);
}

#if 0
static void
edit_values(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;
	AG_Window *pwin;
	AG_Window *win;
	AG_FSpinbutton *fsb;
	AG_WidgetBinding *b1, *b2;
	float v;

	if ((pwin = AG_WidgetParentWindow(pal)) == NULL)
		return;

	if ((win = AG_WindowNew(AG_WINDOW_NO_MAXIMIZE|AG_WINDOW_DETACH,
	    "hsvpal-%p-numedit", pal)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Color values"));
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_LEFT, 0);
	{
		AG_WidgetBinding *bAlpha;
		void *pAlpha;

		fsb = AG_FSpinbuttonNew(win, NULL, _("Hue: "));
		AG_FSpinbuttonPrescale(fsb, "000");
		AG_WidgetCopyBinding(fsb, "value", pal, "hue");
		AG_FSpinbuttonSetRange(fsb, 0.0, 359.0);
		AG_FSpinbuttonSetIncrement(fsb, 1);
		FSpinbuttonSetPrecision(fsb, "f", 0);
		
		fsb = AG_FSpinbuttonNew(win, NULL, _("Saturation: "));
		AG_FSpinbuttonPrescale(fsb, "00.00");
		AG_WidgetCopyBinding(fsb, "value", pal, "saturation");
		AG_FSpinbuttonSetRange(fsb, 0.0, 1.0);
		AG_FSpinbuttonSetIncrement(fsb, 0.01);
		FSpinbuttonSetPrecision(fsb, "f", 2);

		fsb = AG_FSpinbuttonNew(win, NULL, _("Value: "));
		AG_FSpinbuttonPrescale(fsb, "00.00");
		AG_WidgetCopyBinding(fsb, "value", pal, "value");
		AG_FSpinbuttonSetRange(fsb, 0.0, 1.0);
		AG_FSpinbuttonSetIncrement(fsb, 0.01);
		FSpinbuttonSetPrecision(fsb, "f", 2);

		fsb = AG_FSpinbuttonNew(win, NULL, _("Alpha: "));
		AG_FSpinbuttonPrescale(fsb, "0.000");
		AG_WidgetCopyBinding(fsb, "value", pal, "alpha");
		bAlpha = AG_WidgetGetBinding(pal, "alpha", &pAlpha);
		switch (bAlpha->vtype) {
		case AG_WIDGET_FLOAT:
		case AG_WIDGET_DOUBLE:
			AG_FSpinbuttonSetRange(fsb, 0.0, 1.0);
			AG_FSpinbuttonSetIncrement(fsb, 0.005);
			FSpinbuttonSetPrecision(fsb, "f", 3);
			break;
		case AG_WIDGET_INT:
		case AG_WIDGET_UINT:
		case AG_WIDGET_UINT8:
			AG_FSpinbuttonSetRange(fsb, 0.0, 255.0);
			AG_FSpinbuttonSetIncrement(fsb, 1.0);
			FSpinbuttonSetPrecision(fsb, "f", 0);
			break;
		}
		AG_WidgetUnlockBinding(bAlpha);
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}
#endif

static void
complementary(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;
	float hue = AG_WidgetFloat(pal, "hue");

	AG_WidgetSetFloat(pal, "hue", ((int)hue+180) % 359);
	update_pixel_from_hsva(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
copy_color(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;
	
	cH = AG_WidgetFloat(pal, "hue");
	cS = AG_WidgetFloat(pal, "saturation");
	cV = AG_WidgetFloat(pal, "value");
	cA = AG_WidgetFloat(pal, "alpha");
}

static void
paste_color(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;

	AG_WidgetSetFloat(pal, "hue", cH);
	AG_WidgetSetFloat(pal, "saturation", cS);
	AG_WidgetSetFloat(pal, "value", cV);
	AG_WidgetSetFloat(pal, "alpha", cA);
	update_pixel_from_hsva(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
invert_saturation(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;

	AG_WidgetSetFloat(pal, "saturation",
	    1.0 - AG_WidgetFloat(pal, "saturation"));
	update_pixel_from_hsva(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
invert_value(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[1].p;

	AG_WidgetSetFloat(pal, "value",
	    1.0 - AG_WidgetFloat(pal, "value"));
	update_pixel_from_hsva(pal);
	pal->flags |= AG_HSVPAL_DIRTY;
}

static void
open_menu(AG_HSVPal *pal)
{
	int x, y;

	if (pal->menu != NULL)
		close_menu(pal);

	pal->menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(pal->menu);

	pal->menu_item = AG_MenuAddItem(pal->menu, NULL);
	{
#if 0
		AG_MenuAction(pal->menu_item, _("Edit numerically"), -1,
		    edit_values, "%p", pal);

#endif
		AG_MenuAction(pal->menu_item, _("Copy"), -1,
		    copy_color, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Paste"), -1,
		    paste_color, "%p", pal);
		AG_MenuSeparator(pal->menu_item);
		AG_MenuAction(pal->menu_item, _("Show RGB value"), -1,
		    show_rgb, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Complementary color"), -1,
		    complementary, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Invert saturation"), -1,
		    invert_saturation, "%p", pal);
		AG_MenuAction(pal->menu_item, _("Invert value"), -1,
		    invert_value, "%p", pal);
	}
	pal->menu->sel_item = pal->menu_item;
	
	SDL_GetMouseState(&x, &y);
	pal->menu_win = AG_MenuExpand(pal->menu, pal->menu_item, x, y);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[0].p;
	int btn = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	float r;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (y > pal->rAlpha.y) {
			update_a(pal, x);
			pal->state = AG_HSVPAL_SEL_A;
		} else {
			x -= pal->circle.x;
			y -= pal->circle.y;
			r = hypot((float)x, (float)y);

			if (r > (float)pal->circle.rin) {
				update_h(pal, x, y);
				pal->state = AG_HSVPAL_SEL_H;
			} else {
				update_sv(pal, argv[2].i, argv[3].i);
				pal->state = AG_HSVPAL_SEL_SV;
			}
		}
		AG_WidgetFocus(pal);
		break;
	case SDL_BUTTON_MIDDLE:
	case SDL_BUTTON_RIGHT:
		open_menu(pal);
		break;
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[0].p;

	pal->state = AG_HSVPAL_SEL_NONE;
}

static void
mousemotion(int argc, union evarg *argv)
{
	AG_HSVPal *pal = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;

	switch (pal->state) {
	case AG_HSVPAL_SEL_NONE:
		break;
	case AG_HSVPAL_SEL_H:
		update_h(pal,
		    x - pal->circle.x,
		    y - pal->circle.y);
		break;
	case AG_HSVPAL_SEL_SV:
		update_sv(pal, x, y);
		break;
	case AG_HSVPAL_SEL_A:
		update_a(pal, x);
		break;
	}
}

static void
binding_changed(int argc, union evarg *argv)
{
	AG_HSVPal *hsv = argv[0].p;
	AG_WidgetBinding *bind = argv[1].p;

	if (bind->type == AG_WIDGET_UINT32 &&
	    strcmp(bind->name, "pixel") == 0) {
#if 0
		hsv->flags |= AG_HSVPAL_PIXEL;
#endif
		update_hsv_from_pixel(hsv, *(Uint32 *)bind->p1);
	}
	
}

void
AG_HSVPalInit(AG_HSVPal *pal)
{
	int i;

	AG_WidgetInit(pal, "hsvpal", &agHSVPalOps, AG_WIDGET_FOCUSABLE);
	AG_WidgetBind(pal, "hue", AG_WIDGET_FLOAT, &pal->h);
	AG_WidgetBind(pal, "saturation", AG_WIDGET_FLOAT, &pal->s);
	AG_WidgetBind(pal, "value", AG_WIDGET_FLOAT, &pal->v);
	AG_WidgetBind(pal, "alpha", AG_WIDGET_FLOAT, &pal->a);
	AG_WidgetBind(pal, "pixel", AG_WIDGET_UINT32, &pal->pixel);
	AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER, &agVideoFmt);

	pal->flags = 0;
	pal->h = 0.0;
	pal->s = 0.0;
	pal->v = 0.0;
	pal->a = 1.0;
	pal->pixel = SDL_MapRGBA(agVideoFmt, 0, 0, 0, 255);
	pal->circle.spacing = 10;
	pal->circle.width = 20;
	pal->state = AG_HSVPAL_SEL_NONE;
	pal->surface = NULL;
	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
	AG_WidgetMapSurface(pal, NULL);

	AG_SetEvent(pal, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(pal, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(pal, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(pal, "widget-bound", binding_changed, NULL);
}

static void
render_palette(AG_HSVPal *pal)
{
	float h, cur_h, cur_s, cur_v;
	Uint32 pc;
	Uint8 r, g, b, a, da;
	Uint8 *pDst;
	int x, y, i;
	SDL_Rect rd;

	cur_h = (AG_WidgetFloat(pal, "hue")/360) * 2*M_PI;
	cur_s = AG_WidgetFloat(pal, "saturation");
	cur_v = AG_WidgetFloat(pal, "value");

	SDL_LockSurface(pal->surface);

	/* Render the circle of hues. */
	for (h = 0.0; h < 2*M_PI; h += pal->circle.dh) {
		RG_HSV2RGB((h/(2*M_PI)*360.0), 1.0, 1.0, &r, &g, &b);
		pc = SDL_MapRGB(agVideoFmt, r, g, b);

		for (i = 0; i < pal->circle.width; i++) {
			x = (pal->circle.rout - i)*cos(h);
			y = (pal->circle.rout - i)*sin(h);

			AG_PUT_PIXEL2(pal->surface,
			    pal->circle.x+x,
			    pal->circle.y+y,
			    pc);
		}
	}

	/* Render the triangle of saturation and value. */
	for (y = 0; y < pal->triangle.h; y += 2) {
		float sat = (float)(pal->triangle.h - y) /
		            (float)(pal->triangle.h);

		for (x = 0; x < y; x++) {
			RG_HSV2RGB((cur_h/(2*M_PI))*360.0, sat,
			    1.0 - ((float)x/(float)pal->triangle.h),
			    &r, &g, &b);
			pc = SDL_MapRGB(agVideoFmt, r, g, b);
			AG_PUT_PIXEL2(pal->surface,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y,
			    pc);
			AG_PUT_PIXEL2(pal->surface,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y + 1,
			    pc);
		}
	}

	/* Render the alpha selector. */
	/* XXX overblending */
	for (y = 8; y < pal->rAlpha.h+16; y+=8) {
		for (x = 0; x < pal->rAlpha.w; x+=16) {
			rd.w = 8;
			rd.h = 8;
			rd.x = pal->rAlpha.x+x;
			rd.y = pal->rAlpha.y+y;
			SDL_FillRect(pal->surface, &rd, pal->cTile);
		}
		y += 8;
		for (x = 8; x < pal->rAlpha.w; x+=16) {
			rd.w = 8;
			rd.h = 8;
			rd.x = pal->rAlpha.x+x;
			rd.y = pal->rAlpha.y+y;
			SDL_FillRect(pal->surface, &rd, pal->cTile);
		}
	}
	RG_HSV2RGB((cur_h/(2*M_PI))*360.0, cur_s, cur_v, &r, &g, &b);
	da = MIN(1, pal->surface->w/255);
	for (y = pal->rAlpha.y+8; y < pal->surface->h; y++) {
		for (x = 0, a = 0; x < pal->surface->w; x++) {
			AG_BLEND_RGBA2(pal->surface, x, y,
			    r, g, b, a, AG_ALPHA_SRC);
			a = x*255/pal->surface->w;
		}
	}
	SDL_UnlockSurface(pal->surface);
}

void
AG_HSVPalScale(void *p, int w, int h)
{
	AG_HSVPal *pal = p;
	int i, y = 0;

	if (w == -1 && h == -1) {
		AGWIDGET(pal)->w = agView->w/5;
		AGWIDGET(pal)->h = agView->h/3;
	}
	
	pal->rAlpha.x = 0;
	pal->rAlpha.h = 32;
	pal->rAlpha.y = AGWIDGET(pal)->h - 32;
	pal->rAlpha.w = AGWIDGET(pal)->w;
	
	pal->circle.rout = MIN(AGWIDGET(pal)->w,
	    AGWIDGET(pal)->h - pal->rAlpha.h)/2;
	pal->circle.rin = pal->circle.rout - pal->circle.width;
	pal->circle.dh = (float)(1.0/(pal->circle.rout*M_PI));
	pal->circle.x = AGWIDGET(pal)->w/2;
	pal->circle.y = (AGWIDGET(pal)->h - pal->rAlpha.h)/2;

	pal->triangle.x = AGWIDGET(pal)->w/2;
	pal->triangle.y = pal->circle.y+pal->circle.width-pal->circle.rout;
	pal->triangle.h = pal->circle.rin*sin((37.0/360.0)*(2*M_PI)) -
			  pal->circle.rin*sin((270.0/360.0)*(2*M_PI));
	
	pal->selcircle_r = pal->circle.width/2 - 4;

	pal->flags |= AG_HSVPAL_DIRTY;
}

void
AG_HSVPalDraw(void *p)
{
	AG_HSVPal *pal = p;
	float cur_h, cur_s, cur_v;
	Uint8 r, g, b, a;
	int x, y;
	int i;
	
	if (pal->flags & AG_HSVPAL_DIRTY) {
		pal->flags &= ~(AG_HSVPAL_DIRTY);
		pal->surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		    AGWIDGET(pal)->w, AGWIDGET(pal)->h, 32,
		    agVideoFmt->Rmask, agVideoFmt->Gmask, agVideoFmt->Bmask, 0);
		if (pal->surface == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		pal->cTile = SDL_MapRGB(pal->surface->format, 140, 140, 140);
		render_palette(pal);
		AG_WidgetReplaceSurface(pal, 0, pal->surface);
	}

	cur_h = (AG_WidgetFloat(pal, "hue") / 360.0) * 2*M_PI;
	cur_s = AG_WidgetFloat(pal, "saturation");
	cur_v = AG_WidgetFloat(pal, "value");
	a = (Uint8)(AG_WidgetFloat(pal, "alpha")*255);

	AG_WidgetBlitFrom(pal, pal, 0, NULL, 0, 0);

	/* Indicate the current selection. */
	agPrim.circle(pal,
	    pal->circle.x + (pal->circle.rin + pal->circle.width/2)*cos(cur_h),
	    pal->circle.y + (pal->circle.rin + pal->circle.width/2)*sin(cur_h),
	    pal->selcircle_r,
	    AG_COLOR(HSVPAL_CIRCLE_COLOR));
	
	/* The rendering routine uses (v = 1 - x/h), so (x = -v*h + h). */
	y = (int)((1.0 - cur_s) * (float)pal->triangle.h);
	x = (int)(-(cur_v*(float)pal->triangle.h - (float)pal->triangle.h));
	if (x < 0) { x = 0; }
	if (x > y) { x = y; }
	agPrim.circle(pal,
	    pal->triangle.x + x - y/2,
	    pal->triangle.y + y,
	    pal->selcircle_r,
	    AG_COLOR(HSVPAL_CIRCLE_COLOR));

	x = a*pal->rAlpha.w/255;
	if (x > pal->rAlpha.w-3) { x = pal->rAlpha.w-3; }

	/* Draw the color preview. */
	RG_HSV2RGB((cur_h*360.0)/(2*M_PI), cur_s, cur_v, &r, &g, &b);
	agPrim.rect_filled(pal,
	    pal->rAlpha.x, pal->rAlpha.y,
	    pal->rAlpha.w, 8,
	    SDL_MapRGB(agVideoFmt, r, g, b));

	/* Draw the alpha bar. */
	agPrim.vline(pal,
	    pal->rAlpha.x + x,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    AG_COLOR(HSVPAL_BAR1_COLOR));
	agPrim.vline(pal,
	    pal->rAlpha.x + x + 1,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    AG_COLOR(HSVPAL_BAR2_COLOR));
	agPrim.vline(pal,
	    pal->rAlpha.x + x + 2,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    AG_COLOR(HSVPAL_BAR1_COLOR));
}

