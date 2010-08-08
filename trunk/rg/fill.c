/*
 * Copyright (c) 2005-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <gui/window.h>
#include <gui/label.h>
#include <gui/hsvpal.h>
#include <gui/radio.h>
#include <gui/box.h>
#include <gui/numerical.h>
#include <gui/notebook.h>
#include <gui/load_color.h>

#include <vg/vg.h>

#include "tileset.h"
#include "tileview.h"
#include "fill.h"
#include "icons.h"

const AG_Version rgFillVer = { 0, 0, };
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

	RG_FeatureInit(f, ts, flags, &rgFillOps);
	f->type = FILL_SOLID;
	f->alpha = 255;
	f->f_gradient.c1 = AG_ColorRGB(0,0,0);
	f->f_gradient.c2 = AG_ColorRGB(0,0,0);
}

int
RG_FillLoad(void *p, AG_DataSource *buf)
{
	struct rg_fill_feature *f = p;

	if (AG_ReadVersion(buf, "RG_Feature:RG_Fill", &rgFillVer, NULL) == -1)
		return (-1);

	f->type = (enum rg_fill_type)AG_ReadUint8(buf);
	switch (f->type) {
	case FILL_SOLID:
		f->f_solid.c = AG_ReadColor(buf);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		f->f_gradient.c1 = AG_ReadColor(buf);
		f->f_gradient.c2 = AG_ReadColor(buf);
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
RG_FillSave(void *p, AG_DataSource *buf)
{
	struct rg_fill_feature *f = p;

	AG_WriteVersion(buf, "RG_Feature:RG_Fill", &rgFillVer);

	AG_WriteUint8(buf, (Uint8)f->type);
	switch (f->type) {
	case FILL_SOLID:
		AG_WriteColor(buf, f->f_solid.c);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		AG_WriteColor(buf, f->f_gradient.c1);
		AG_WriteColor(buf, f->f_gradient.c2);
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
	AG_Box *box;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Fill/gradient"));

	{
		static const char *modes[] = {
			N_("Solid"),
			N_("Horizontal gradient"),
			N_("Vertical gradient"),
			N_("Circular gradient"),
			N_("Pixmap pattern"),
			NULL
		};
		AG_RadioNewUint(win, AG_RADIO_HFILL, modes, &f->type);
	}

	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL|AG_BOX_VFILL);
	{
		AG_HSVPal *hsv1, *hsv2;
		AG_Numerical *num;
		AG_Notebook *nb;
		AG_NotebookTab *ntab;

		nb = AG_NotebookNew(box, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
		ntab = AG_NotebookAddTab(nb, _("Color A"), AG_BOX_VERT);
		{
			hsv1 = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
			AG_BindUint8(hsv1, "RGBAv", (Uint8 *)&f->f_gradient.c1);
		}

		ntab = AG_NotebookAddTab(nb, _("Color B"), AG_BOX_VERT);
		{
			hsv2 = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
			AG_BindUint8(hsv2, "RGBAv", (Uint8 *)&f->f_gradient.c2);
		}
		
		num = AG_NumericalNewUint8R(box, 0, NULL,
		    _("Overall alpha: "), &f->alpha, 0, 255);
		AG_NumericalSetIncrement(num, 5);
	}
	return (win);
}

void
RG_FillApply(void *p, RG_Tile *t, int x, int y)
{
	struct rg_fill_feature *fi = p;
	AG_Surface *su = t->su;
	AG_Color C;
	Uint8 a;
	
	switch (fi->type) {
	case FILL_SOLID:
		AG_FillRect(su, NULL, fi->f_solid.c);
		break;
	case FILL_HGRADIENT:
		{
			int x, y;
			AG_Color c1 = fi->f_gradient.c1;
			AG_Color c2 = fi->f_gradient.c2;
	
			C.a = fi->alpha;
			for (y = 0; y < su->h; y++) {
				a = (Uint8)(su->h-y)*255/su->h;
				C.r = (((c1.r - c2.r)*a)>>8) + c2.r;
				C.g = (((c1.g - c2.g)*a)>>8) + c2.g;
				C.b = (((c1.b - c2.b)*a)>>8) + c2.b;

				if (fi->alpha == 255) {
					for (x = 0; x < su->w; x++) {
						RG_PutPixel(t->su, x, y,
						    AG_MapColorRGB(t->su->format, C));
					}
				} else {
					for (x = 0; x < su->w; x++) {
						RG_BlendRGB(t->su, x, y,
						    RG_PRIM_OVERLAY_ALPHA,
						    C);
					}
				}
			}
		}
		break;
	case FILL_VGRADIENT:
		{
			int x, y;
			AG_Color c1 = fi->f_gradient.c1;
			AG_Color c2 = fi->f_gradient.c2;
			
			C.a = fi->alpha;
			for (y = 0; y < su->h; y++) {
				for (x = 0; x < su->w; x++) {
					a = (su->h - x)*255/su->h;
					C.r = (((c1.r - c2.r)*a)>>8) + c2.r;
					C.g = (((c1.g - c2.g)*a)>>8) + c2.g;
					C.b = (((c1.b - c2.b)*a)>>8) + c2.b;

					RG_BlendRGB(t->su, x, y,
					    RG_PRIM_OVERLAY_ALPHA, C);
				}
			}
		}
		break;
	case FILL_CGRADIENT:
		{
			int i, r = MAX(su->w,su->h);
			int x = su->w/2;
			int y = su->h/2;
			AG_Color c1 = fi->f_gradient.c1;
			AG_Color c2 = fi->f_gradient.c2;
		
			C.a = fi->alpha;
			for (i = 0; i < r; i++) {
				a = (r - i)*255/r;
				C.r = (((c1.r - c2.r)*a)>>8) + c2.r;
				C.g = (((c1.g - c2.g)*a)>>8) + c2.g;
				C.b = (((c1.b - c2.b)*a)>>8) + c2.b;
				RG_ColorRGBA(t, C);
				RG_Circle2(t, x,y, i);
			}
		}
		break;
	default:
		break;
	}
}

static void
InvertColors(AG_Event *event)
{
	struct rg_fill_feature *fi = AG_PTR(1);
	AG_Color c;

	switch (fi->type) {
	case FILL_SOLID:
		c = fi->f_solid.c;
		c.r = 255 - c.r;
		c.g = 255 - c.g;
		c.b = 255 - c.b;
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		c = fi->f_gradient.c1;
		c.r = 255 - c.r;
		c.g = 255 - c.g;
		c.b = 255 - c.b;
		fi->f_gradient.c1 = c;

		c = fi->f_gradient.c2;
		c.r = 255 - c.r;
		c.g = 255 - c.g;
		c.b = 255 - c.b;
		fi->f_gradient.c2 = c;
		break;
	default:
		break;
	}
}

static void
SwapGradientColors(AG_Event *event)
{
	struct rg_fill_feature *fi = AG_PTR(1);
	AG_Color cSave;

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
set_type(AG_Event *event)
{
	struct rg_fill_feature *fi = AG_PTR(1);
	int type = AG_INT(2);

	fi->type = type;
}

void
RG_FillMenu(void *p, AG_MenuItem *mi)
{
	struct rg_fill_feature *fi = p;
	AG_MenuItem *mi_fill;

	mi_fill = AG_MenuAction(mi, _("Fill type"), rgIconFill.s, NULL, NULL);
	{
		AG_MenuAction(mi_fill, _("Solid fill"), rgIconFill.s,
		    set_type, "%p, %i", fi, FILL_SOLID);
		AG_MenuAction(mi_fill, _("Horizontal gradient"), rgIconHGrad.s,
		    set_type, "%p, %i", fi, FILL_HGRADIENT);
		AG_MenuAction(mi_fill, _("Vertical gradient"), rgIconVGrad.s,
		    set_type, "%p, %i", fi, FILL_VGRADIENT);
		AG_MenuAction(mi_fill, _("Circular gradient"), rgIconCGrad.s,
		    set_type, "%p, %i", fi, FILL_CGRADIENT);
		AG_MenuAction(mi_fill, _("Pattern"), rgIconTiling.s,
		    set_type, "%p, %i", fi, FILL_PATTERN);
	}
	
	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Swap gradient colors"), rgIconSwap.s,
	    SwapGradientColors, "%p", fi);
	AG_MenuAction(mi, _("Invert colors"), rgIconInvert.s,
	    InvertColors, "%p", fi);
}

AG_Toolbar *
RG_FillToolbar(void *p, RG_Tileview *tv)
{
	struct rg_fill_feature *fi = p;
	AG_Toolbar *tbar;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1, 0);
	AG_ToolbarButtonIcon(tbar, rgIconSwap.s, 0,
	    SwapGradientColors, "%p", fi);
	AG_ToolbarButtonIcon(tbar, rgIconInvert.s, 0,
	    InvertColors, "%p", fi);
	return (tbar);
}

