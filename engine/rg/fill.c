/*	$Csoft: fill.c,v 1.17 2005/05/26 06:46:47 vedge Exp $	*/

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
#include <engine/view.h>

#include <engine/vg/vg.h>

#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/radio.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>

#include "tileset.h"
#include "tileview.h"
#include "fill.h"

const AG_Version rgFillVer = {
	"agar rg fill feature",
	0, 0
};

const RG_FeatureOps rgFillOps = {
	"fill",
	sizeof(struct rg_fill_feature),
	N_("Fill tile with solid color/pattern."),
	FEATURE_AUTOREDRAW,
	RG_FillInit,
	RG_FillLoad,
	RG_FillSave,
	NULL,		/* destroy */
	RG_FillApply,
	RG_FillMenu,
	RG_FillToolbar,
	RG_FillEdit
};

void
RG_FillInit(void *p, RG_Tileset *ts, int flags)
{
	struct rg_fill_feature *f = p;

	AG_FeatureInit(f, ts, flags, &rgFillOps);
	f->type = FILL_SOLID;
	f->alpha = 255;
	f->f_gradient.c1 = SDL_MapRGB(ts->fmt, 0, 0, 0);
	f->f_gradient.c2 = SDL_MapRGB(ts->fmt, 0, 0, 0);
}

int
RG_FillLoad(void *p, AG_Netbuf *buf)
{
	struct rg_fill_feature *f = p;
	RG_Tileset *ts = RG_FEATURE(f)->ts;

	if (AG_ReadVersion(buf, &rgFillVer, NULL) == -1)
		return (-1);

	f->type = (enum fill_type)AG_ReadUint8(buf);
	switch (f->type) {
	case FILL_SOLID:
		f->f_solid.c = AG_ReadColor(buf, ts->fmt);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		f->f_gradient.c1 = AG_ReadColor(buf, ts->fmt);
		f->f_gradient.c2 = AG_ReadColor(buf, ts->fmt);
		break;
	case FILL_PATTERN:
		f->f_pattern.texid = (int)AG_ReadUint32(buf);
		f->f_pattern.tex_xoffs = (int)AG_ReadUint32(buf);
		f->f_pattern.tex_yoffs = (int)AG_ReadUint32(buf);
		break;
	}
	return (0);
}

void
RG_FillSave(void *p, AG_Netbuf *buf)
{
	struct rg_fill_feature *f = p;
	RG_Tileset *ts = RG_FEATURE(f)->ts;

	AG_WriteVersion(buf, &rgFillVer);

	AG_WriteUint8(buf, (Uint8)f->type);
	switch (f->type) {
	case FILL_SOLID:
		AG_WriteColor(buf, ts->fmt, f->f_solid.c);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		AG_WriteColor(buf, ts->fmt, f->f_gradient.c1);
		AG_WriteColor(buf, ts->fmt, f->f_gradient.c2);
		break;
	case FILL_PATTERN:
		AG_WriteUint32(buf, (Uint32)f->f_pattern.texid);
		AG_WriteUint32(buf, (Uint32)f->f_pattern.tex_xoffs);
		AG_WriteUint32(buf, (Uint32)f->f_pattern.tex_yoffs);
		break;
	}
}

AG_Window *
RG_FillEdit(void *p, RG_Tileview *tv)
{
	struct rg_fill_feature *f = p;
	AG_Window *win;
	AG_Radio *rad;
	static const char *modes[] = {
		N_("Solid"),
		N_("Horizontal gradient"),
		N_("Vertical gradient"),
		N_("Circular gradient"),
		N_("Pixmap pattern"),
		NULL
	};
	AG_Box *box;

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Fill/gradient"));

	rad = AG_RadioNew(win, modes);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &f->type);

	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL|AG_BOX_HFILL);
	{
		AG_HSVPal *hsv1, *hsv2;
		AG_Spinbutton *sb;
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Box *hb;

		nb = AG_NotebookNew(box, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
		ntab = AG_NotebookAddTab(nb, _("Color A"), AG_BOX_VERT);
		{
			hsv1 = AG_HSVPalNew(ntab);
			AGWIDGET(hsv1)->flags |= AG_WIDGET_WFILL|
			                         AG_WIDGET_HFILL;
			AG_WidgetBind(hsv1, "pixel-format", AG_WIDGET_POINTER,
			    &tv->ts->fmt);
			AG_WidgetBind(hsv1, "pixel", AG_WIDGET_UINT32,
			    &f->f_gradient.c1);
		}

		ntab = AG_NotebookAddTab(nb, _("Color B"), AG_BOX_VERT);
		{
			hsv2 = AG_HSVPalNew(ntab);
			AGWIDGET(hsv2)->flags |= AG_WIDGET_WFILL|
			                         AG_WIDGET_HFILL;
			AG_WidgetBind(hsv1, "pixel-format", AG_WIDGET_POINTER,
			    &tv->ts->fmt);
			AG_WidgetBind(hsv2, "pixel", AG_WIDGET_UINT32,
			    &f->f_gradient.c2);
		}
		
		sb = AG_SpinbuttonNew(box, _("Overall alpha: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT8, &f->alpha);
		AG_SpinbuttonSetRange(sb, 0, 255);
		AG_SpinbuttonSetIncrement(sb, 5);
	}
	return (win);
}

void
RG_FillApply(void *p, RG_Tile *t, int x, int y)
{
	struct rg_fill_feature *fi = p;
	SDL_Surface *su = t->su;
	Uint8 r1, g1, b1;
	Uint8 r2, g2, b2;
	
	SDL_GetRGB(fi->f_gradient.c1, t->ts->fmt, &r1, &g1, &b1);
	SDL_GetRGB(fi->f_gradient.c2, t->ts->fmt, &r2, &g2, &b2);

	switch (fi->type) {
	case FILL_SOLID:
		SDL_GetRGB(fi->f_solid.c, t->ts->fmt, &r1, &g1, &b1);
		SDL_FillRect(su, NULL, SDL_MapRGBA(t->ts->fmt, r1, g1, b1,
		    fi->alpha));
		break;
	case FILL_HGRADIENT:
		{
			int x, y;
		
			SDL_LockSurface(su);
			for (y = 0; y < su->h; y++) {
				Uint32 c;
				Uint8 a = (Uint8)(su->h-y)*255/su->h;

				c = SDL_MapRGB(t->ts->fmt,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2);

				for (x = 0; x < su->w; x++) {
					if (fi->alpha == 255) {
						RG_PutPixel(t->su, x, y,
						    SDL_MapRGB(t->su->format,
						    (((r1 - r2)*a) >> 8) + r2,
						    (((g1 - g2)*a) >> 8) + g2,
						    (((b1 - b2)*a) >> 8) + b2));
					} else {
						RG_BlendRGB(t->su, x, y,
						    RG_PRIM_OVERLAY_ALPHA,
						    (((r1 - r2)*a) >> 8) + r2,
						    (((g1 - g2)*a) >> 8) + g2,
						    (((b1 - b2)*a) >> 8) + b2,
						    fi->alpha);
					}
				}
			}
			SDL_UnlockSurface(su);
		}
		break;
	case FILL_VGRADIENT:
		{
			int x, y;
		
			SDL_LockSurface(su);
			for (y = 0; y < su->h; y++) {
				for (x = 0; x < su->w; x++) {
					Uint8 a = (su->h-x)*255/su->h;
				
					RG_BlendRGB(t->su, x, y,
					    RG_PRIM_OVERLAY_ALPHA,
					    (((r1 - r2) * a) >> 8) + r2,
					    (((g1 - g2) * a) >> 8) + g2,
					    (((b1 - b2) * a) >> 8) + b2,
					    fi->alpha);
				}
			}
			SDL_UnlockSurface(su);
		}
		break;
	case FILL_CGRADIENT:
		{
			int i, r = MAX(su->w,su->h);
			int x = su->w/2;
			int y = su->h/2;
		
			SDL_LockSurface(su);
			for (i = 0; i < r; i++) {
				Uint8 a = (r-i)*255/r;
			
				RG_ColorRGBA(t,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2,
				    fi->alpha);
				RG_Circle2(t, x, y, i);
			}
			SDL_UnlockSurface(su);
		}
		break;
	default:
		break;
	}
}

static void
invert_colors(int argc, union evarg *argv)
{
	struct rg_fill_feature *fi = argv[1].p;
	RG_Tileset *ts = RG_FEATURE(fi)->ts;
	Uint8 r, g, b;

	switch (fi->type) {
	case FILL_SOLID:
		SDL_GetRGB(fi->f_solid.c, ts->fmt, &r, &g, &b);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_solid.c = SDL_MapRGB(ts->fmt, r, g, b);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		SDL_GetRGB(fi->f_gradient.c1, ts->fmt, &r, &g, &b);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_gradient.c1 = SDL_MapRGB(ts->fmt, r, g, b);
		SDL_GetRGB(fi->f_gradient.c2, ts->fmt, &r, &g, &b);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_gradient.c2 = SDL_MapRGB(ts->fmt, r, g, b);
		break;
	default:
		break;
	}
}

static void
swap_gradient(int argc, union evarg *argv)
{
	struct rg_fill_feature *fi = argv[1].p;
	Uint32 cSave;

	switch (fi->type) {
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		cSave = fi->f_gradient.c2;
		fi->f_gradient.c2 = fi->f_gradient.c1;
		fi->f_gradient.c1 = cSave;
		break;
	default:
		break;
	}
}

static void
set_type(int argc, union evarg *argv)
{
	struct rg_fill_feature *fi = argv[1].p;
	int type = argv[2].i;

	fi->type = type;
}

void
RG_FillMenu(void *p, AG_MenuItem *mi)
{
	struct rg_fill_feature *fi = p;
	AG_MenuItem *mi_fill;

	mi_fill = AG_MenuAction(mi, _("Fill type"), RG_FILL_ICON, NULL, NULL);
	{
		AG_MenuAction(mi_fill, _("Solid fill"), RG_FILL_ICON,
		    set_type, "%p, %i", fi, FILL_SOLID);
		AG_MenuAction(mi_fill, _("Horizontal gradient"),
		    RG_HGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_HGRADIENT);
		AG_MenuAction(mi_fill, _("Vertical gradient"),
		    RG_VGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_VGRADIENT);
		AG_MenuAction(mi_fill, _("Circular gradient"),
		    RG_CGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_CGRADIENT);
		AG_MenuAction(mi_fill, _("Pattern"), RG_TILING_ICON,
		    set_type, "%p, %i", fi, FILL_PATTERN);
	}
	
	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Swap gradient colors"), RG_SWAP_ICON,
	    swap_gradient, "%p", fi);
	AG_MenuAction(mi, _("Invert colors"), RG_INVERT_ICON,
	    invert_colors, "%p", fi);
}

AG_Toolbar *
RG_FillToolbar(void *p, RG_Tileview *tv)
{
	struct rg_fill_feature *fi = p;
	AG_Toolbar *tbar;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1, 0);
	AG_ToolbarAddButton(tbar, 0, AGICON(RG_SWAP_ICON), 0, 0,
	    swap_gradient, "%p", fi);
	AG_ToolbarAddButton(tbar, 0, AGICON(RG_INVERT_ICON), 0, 0,
	    invert_colors, "%p", fi);
	return (tbar);
}

