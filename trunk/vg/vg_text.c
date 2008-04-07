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
#include "vg_view.h"
#include "icons.h"

#include <stdarg.h>
#include <string.h>

static void
Init(void *p)
{
	VG_Text *vt = p;

	vt->text[0] = '\0';
	vt->angle = 0.0f;
	vt->align = VG_ALIGN_MC;
	vt->nPtrs = 0;
	vt->fontSize = agDefaultFont->size;
	vt->fontFlags = agDefaultFont->flags;
	Strlcpy(vt->fontFace, OBJECT(agDefaultFont)->name,
	    sizeof(vt->fontFace));
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
	VG_Text *vt = p;

	vt->p = VG_ReadRef(ds, vt, "Point");
	vt->align = (enum vg_alignment)AG_ReadUint8(ds);
	vt->angle = AG_ReadFloat(ds);
	AG_CopyString(vt->fontFace, ds, sizeof(vt->fontFace));
	vt->fontSize = (int)AG_ReadUint8(ds);
	vt->fontFlags = (Uint)AG_ReadUint16(ds);
	AG_CopyString(vt->text, ds, sizeof(vt->text));
	vt->nPtrs = 0;
	return (0);
}

static void
Save(void *p, AG_DataSource *ds)
{
	VG_Text *vt = p;

	VG_WriteRef(ds, vt->p);
	AG_WriteUint8(ds, (Uint8)vt->align);
	AG_WriteFloat(ds, vt->angle);
	AG_WriteString(ds, vt->fontFace);
	AG_WriteUint8(ds, (Uint8)vt->fontSize);
	AG_WriteUint16(ds, (Uint16)vt->fontFlags);
	AG_WriteString(ds, vt->text);
}

/* Specify text with polled values. */
void
VG_TextPrintfP(VG_Text *vt, const char *fmt, ...)
{
	const char *p;
	va_list ap;
	
	if (fmt != NULL) {
		Strlcpy(vt->text, fmt, sizeof(vt->text));
	} else {
		vt->text[0] = '\0';
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
				if (vt->nPtrs >= VG_TEXT_MAX_PTRS) {
					break;
				}
				vt->ptrs[vt->nPtrs++] = va_arg(ap, void *);
				break;
			}
		}
	}
	va_end(ap);
}

/* Specify static text. */
void
VG_TextPrintf(VG_Text *vt, const char *fmt, ...)
{
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(vt->text, sizeof(vt->text), fmt, ap);
		va_end(ap);
	} else {
		vt->text[0] = '\0';
	}
}

static __inline__ void
AlignText(VG_Text *vt, int *x, int *y, int w, int h)
{
	switch (vt->align) {
	case VG_ALIGN_TL:
		break;
	case VG_ALIGN_TC:
		(*x) -= w/2;
		break;
	case VG_ALIGN_TR:
		(*x) -= w;
		break;
	case VG_ALIGN_ML:
		(*y) -= h/2;
		break;
	case VG_ALIGN_MC:
		(*x) -= w/2;
		(*y) -= h/2;
		break;
	case VG_ALIGN_MR:
		(*x) -= w;
		(*y) -= h/2;
		break;
	case VG_ALIGN_BL:
		(*y) -= h;
		break;
	case VG_ALIGN_BC:
		(*x) -= w/2;
		(*y) -= h;
		break;
	case VG_ALIGN_BR:
		(*x) -= w;
		(*y) -= h;
		break;
	default:
		break;
	}
}

#define TARG(_type) (*(_type *)vt->ptrs[ri])

static void
DrawPolled(VG_Text *vt, VG_View *vv)
{
	VG_Node *vn = VGNODE(vt);
	char s[VG_TEXT_MAX], s2[32];
	char *fmtp;
	int  ri = 0;
	int x, y, su;
	
	VG_GetViewCoords(vv, VG_PointPos(vt->p), &x, &y);
	s[0] = '\0';
	s2[0] = '\0';
	for (fmtp = &vt->text[0]; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
			case 'd':
			case 'i':
				Snprintf(s2, sizeof(s2), "%d", TARG(int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'o':
				Snprintf(s2, sizeof(s2), "%o", TARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'u':
				Snprintf(s2, sizeof(s2), "%u", TARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'x':
				Snprintf(s2, sizeof(s2), "%x", TARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'X':
				Snprintf(s2, sizeof(s2), "%X", TARG(Uint));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'c':
				s2[0] = TARG(char);
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 's':
				Strlcat(s, &TARG(char), sizeof(s));
				ri++;
				break;
			case 'f':
				Snprintf(s2, sizeof(s2), "%.2f", TARG(float));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'F':
				Snprintf(s2, sizeof(s2), "%.2f", TARG(double));
				Strlcat(s, s2, sizeof(s));
				ri++;
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
	AG_PushTextState();
	AG_TextFontLookup(vt->fontFace, vt->fontSize, vt->fontFlags),
	AG_TextColorVideo32(VG_MapColorRGB(vn->color));
	su = AG_TextCacheInsLookup(vv->tCache, vt->text);
	AlignText(vt, &x, &y, WSURFACE(vv,su)->w, WSURFACE(vv,su)->h);
	AG_WidgetBlitSurface(vv, su, x, y);
	AG_PopTextState();
}

#undef TARG

static void
Draw(void *p, VG_View *vv)
{
	VG_Text *vt = p;
	int x, y, su;
	
	if (vt->nPtrs > 0) {
		DrawPolled(vt, vv);
		return;
	}

	VG_GetViewCoords(vv, VG_PointPos(vt->p), &x, &y);

	AG_PushTextState();
	AG_TextFontLookup(vt->fontFace, vt->fontSize, vt->fontFlags),
	AG_TextColorVideo32(VG_MapColorRGB(VGNODE(vt)->color));

	su = AG_TextCacheInsLookup(vv->tCache, vt->text);
	AlignText(vt, &x, &y, WSURFACE(vv,su)->w, WSURFACE(vv,su)->h);
	AG_WidgetBlitSurface(vv, su, x,y);

	AG_PopTextState();
}

static void
Extent(void *p, VG_View *vv, VG_Rect *r)
{
	VG_Text *vt = p;
	int x, y, w, h;
	VG_Vector vRect;
	int su;
	
	su = AG_TextCacheInsLookup(vv->tCache, vt->text);
	VG_GetViewCoords(vv, VG_PointPos(vt->p), &x, &y);
	w = WSURFACE(vv,su)->w;
	h = WSURFACE(vv,su)->h;
	AlignText(vt, &x, &y, w, h);
	VG_GetVGCoords(vv, x,y, &vRect);
	r->x = vRect.x;
	r->y = vRect.y;
	r->w = (float)w/vv->scale;
	r->h = (float)h/vv->scale;
}

static float
PointProximity(void *p, VG_Vector *vPt)
{
	VG_Text *vt = p;
	VG_Vector vPos = VG_PointPos(vt->p);
	float d;

	d = VG_Distance(*vPt, vPos);
	vPt->x = vPos.x;
	vPt->y = vPos.y;
	return (d);
}

static void
Delete(void *p)
{
	VG_Text *vt = p;

	if (VG_DelRef(vt, vt->p) == 0)
		VG_Delete(vt->p);
}

const VG_NodeOps vgTextOps = {
	N_("Text label"),
	&vgIconText,
	sizeof(VG_Text),
	Init,
	NULL,			/* destroy */
	Load,
	Save,
	Draw,
	Extent,
	PointProximity,
	NULL,			/* lineProximity */
	Delete,
	NULL			/* moveNode */
};
