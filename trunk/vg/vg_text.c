/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Text entity.
 */

#include <core/limits.h>
#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_text.h"
#include "vg_view.h"
#include "vg_math.h"
#include "icons.h"

#include <stdarg.h>
#include <string.h>

static void
Init(VG *vg, VG_Element *vge)
{
	vge->vg_text.su = NULL;
	vge->vg_text.text[0] = '\0';
	vge->vg_text.angle = 0.0f;
	vge->vg_text.align = VG_ALIGN_MC;
	vge->vg_text.nptrs = 0;
}

static void
Destroy(VG *vg, VG_Element *vge)
{
	if (vge->vg_text.su != NULL)
		SDL_FreeSurface(vge->vg_text.su);
}

/* Specify the text alignment around the central vertex. */
void
VG_TextAlignment(VG *vg, enum vg_alignment align)
{
	vg->cur_vge->vg_text.align = align;
}

/* Specify the angle relative to the central vertex. */
void
VG_TextAngle(VG *vg, float angle)
{
	vg->cur_vge->vg_text.angle = angle;
}

/* Specify text with polled values. */
void
VG_PrintfP(VG *vg, const char *fmt, ...)
{
	VG_Element *vge = vg->cur_vge;
	const char *p;
	va_list ap;
	
	if (fmt != NULL) {
		Strlcpy(vge->vg_text.text, fmt, sizeof(vge->vg_text.text));
	} else {
		vge->vg_text.text[0] = '\0';
		return;
	}

	va_start(ap, fmt);
	for (p = &fmt[0]; *p != '\0'; p++) {
		if (*p == '%' && *(p+1) != '\0') {
			switch (*(p+1)) {
			case ' ':
			case '(':
			case ')':
			case '%':
				break;
			default:
				if (vge->vg_text.nptrs >= VG_TEXT_MAX_PTRS) {
					break;
				}
				vge->vg_text.ptrs[vge->vg_text.nptrs++] =
				    va_arg(ap, void *);
				break;
			}
		}
	}
	va_end(ap);
	
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
		vge->vg_text.su = NULL;
	}
}

/* Specify static text. */
void
VG_Printf(VG *vg, const char *fmt, ...)
{
	VG_Element *vge = vg->cur_vge;
	va_list args;

	if (fmt != NULL) {
		va_start(args, fmt);
		Vsnprintf(vge->vg_text.text, sizeof(vge->vg_text.text), fmt,
		    args);
		va_end(args);
	} else {
		vge->vg_text.text[0] = '\0';
	}
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
		vge->vg_text.su = NULL;
	}
}

#define TEXT_ARG(_type) (*(_type *)vge->vg_text.ptrs[ri])

static void
omega(char *s, size_t len, int n)
{
	Strlcat(s, "omega", len);
}

static const struct {
	char	 *fmt;
	size_t	  fmt_len;
	void	(*func)(char *, size_t, int);
} fmts[] = {
	{ "omega", sizeof("omega"), omega },
};
static const int nfmts = sizeof(fmts) / sizeof(fmts[0]);

static void
AlignText(VG_View *vv, VG_Element *vge, Sint16 *x, Sint16 *y)
{
	SDL_Surface *su = vge->vg_text.su;
	int vx, vy;
	
	VG_GetViewCoords(vv, vge->vtx[0].x, vge->vtx[0].y, &vx, &vy);

	switch (vge->vg_text.align) {
	case VG_ALIGN_TL:
		*x = (Sint16)vx;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_TC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_TR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy;
		break;
	case VG_ALIGN_ML:
		*x = (Sint16)vx;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_MC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_MR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy - su->h/2;
		break;
	case VG_ALIGN_BL:
		*x = (Sint16)vx;
		*y = (Sint16)vy - su->h;
		break;
	case VG_ALIGN_BC:
		*x = (Sint16)vx - su->w/2;
		*y = (Sint16)vy - su->h;
		break;
	case VG_ALIGN_BR:
		*x = (Sint16)vx - su->w;
		*y = (Sint16)vy - su->h;
		break;
	default:
		break;
	}
}

static void
DrawLabel(VG_View *vv, VG_Element *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	char s[VG_TEXT_MAX];
	char s2[32];
	char *fmtp;
	int i, ri = 0;

	if (vge->vg_text.nptrs == 0) {
		if (vge->vg_text.su != NULL) {
			SDL_FreeSurface(vge->vg_text.su);
		}
		AG_PushTextState();
		AG_TextFontLookup(vge->text_st.face, vge->text_st.size, 0),
		AG_TextColorVideo32(c32);
		vge->vg_text.su = AG_TextRender(vge->vg_text.text);
		AG_PopTextState();
		return;
	}

	s[0] = '\0';
	s2[0] = '\0';

	for (fmtp = vge->vg_text.text; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
			case 'd':
			case 'i':
				Snprintf(s2, sizeof(s2), "%d", TEXT_ARG(int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'o':
				Snprintf(s2, sizeof(s2), "%o", TEXT_ARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'u':
				Snprintf(s2, sizeof(s2), "%u", TEXT_ARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'x':
				Snprintf(s2, sizeof(s2), "%x", TEXT_ARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'X':
				Snprintf(s2, sizeof(s2), "%X", TEXT_ARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'c':
				s2[0] = TEXT_ARG(char);
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 's':
				Strlcat(s, &TEXT_ARG(char), sizeof(s));
				ri++;
				break;
			case 'p':
				Snprintf(s2, sizeof(s2), "%p",
				    TEXT_ARG(void *));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'f':
				Snprintf(s2, sizeof(s2), "%.2f",
				    TEXT_ARG(float));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case '[':
				for (i = 0; i < nfmts; i++) {
					if (strncmp(fmts[i].fmt, fmtp+2,
					    fmts[i].fmt_len-1) != 0) {
						continue;
					}
					fmtp += fmts[i].fmt_len;
					fmts[i].func(s2, sizeof(s2), ri);
					Strlcat(s, s2, sizeof(s));
					ri++;
					break;
				}
				break;
			case '%':
				s2[0] = '%';
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				break;
			}
			fmtp++;
		} else {
			s2[0] = *fmtp;
			s2[1] = '\0';
			Strlcat(s, s2, sizeof(s));
		}
	}
	if (vge->vg_text.su != NULL) {
		SDL_FreeSurface(vge->vg_text.su);
	}
	AG_PushTextState();
	AG_TextFontLookup(vge->text_st.face, vge->text_st.size, 0),
	AG_TextColorVideo32(c32);
	vge->vg_text.su = AG_TextRender(s);
	AG_PopTextState();
}

static void
Draw(VG_View *vv, VG_Element *vge)
{
	SDL_Rect rd;

	DrawLabel(vv, vge);
	AlignText(vv, vge, &rd.x, &rd.y);
	AG_WidgetBlit(vv, vge->vg_text.su, rd.x, rd.y);
}

static void
Extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
//	Sint16 rx, ry;

	r->x = 0;
	r->y = 0;
	r->w = 0;
	r->h = 0;
#if 0
	/* XXX XXX XXX */
	DrawLabel(vv, vge);
	AlignText(vv, vge, &rx, &ry);
	r->x = VG_VcoordX(vv,rx);
	r->y = VG_VcoordY(vv,ry);
	r->w = vge->vg_text.su->w/vv->scale;
	r->h = vge->vg_text.su->h/vv->scale;
#endif
}

static float
Intersect(VG *vg, VG_Element *vge, float *x, float *y)
{
	float d;

	if (vge->nvtx < 1 || vge->vg_text.su == NULL)
		return (AG_FLT_MAX);

	d = Distance2(*x, *y, vge->vtx[0].x, vge->vtx[0].y);
	*x = vge->vtx[0].x;
	*y = vge->vtx[0].y;
	return (d);
}

const VG_ElementOps vgTextOps = {
	N_("Text string"),
	&vgIconText,
	Init,
	Destroy,
	Draw,
	Extent,
	Intersect	
};
