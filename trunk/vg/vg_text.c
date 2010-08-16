/*
 * Copyright (c) 2004-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/primitive.h>
#include <gui/textbox.h>
#include <gui/font_selector.h>
#include <gui/checkbox.h>
#include <gui/opengl.h>

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
	vt->p1 = NULL;
	vt->p2 = NULL;
	vt->align = VG_ALIGN_MC;
	vt->fontSize = agGUI ? agDefaultFont->size : 12;
	vt->fontFlags = agGUI ? agDefaultFont->flags : 0;
	vt->fontFace[0] = '\0';
	vt->args = NULL;
	vt->argSizes = NULL;
	vt->vsObj = NULL;
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
	VG_Text *vt = p;

	if ((vt->p1 = VG_ReadRef(ds, vt, "Point")) == NULL ||
	    (vt->p2 = VG_ReadRef(ds, vt, "Point")) == NULL)
		return (-1);

	vt->align = (enum vg_alignment)AG_ReadUint8(ds);
	AG_CopyString(vt->fontFace, ds, sizeof(vt->fontFace));
	vt->fontSize = (int)AG_ReadUint8(ds);
	vt->fontFlags = (Uint)AG_ReadUint16(ds);
	AG_CopyString(vt->text, ds, sizeof(vt->text));
	return (0);
}

static void
Save(void *p, AG_DataSource *ds)
{
	VG_Text *vt = p;

	VG_WriteRef(ds, vt->p1);
	VG_WriteRef(ds, vt->p2);
	AG_WriteUint8(ds, (Uint8)vt->align);
	AG_WriteString(ds, vt->fontFace);
	AG_WriteUint8(ds, (Uint8)vt->fontSize);
	AG_WriteUint16(ds, (Uint16)vt->fontFlags);
	AG_WriteString(ds, vt->text);
}

/* Specify text with polled values. */
void
VG_TextPrintfPolled(VG_Text *vt, const char *fmt, ...)
{
	va_list ap;
	
	VG_Lock(VGNODE(vt)->vg);

	if (vt->args != NULL) {
		AG_ListDestroy(vt->args);
		vt->args = NULL;
		Free(vt->argSizes);
		vt->argSizes = NULL;
	}
	if (fmt != NULL) {
		Strlcpy(vt->text, fmt, sizeof(vt->text));
	} else {
		vt->text[0] = '\0';
		goto out;
	}

	va_start(ap, fmt);
	vt->args = AG_ListNew();
	AG_PARSE_VARIABLE_ARGS(ap, fmt, vt->args, vt->argSizes);
	va_end(ap);
out:
	VG_Unlock(VGNODE(vt)->vg);
}

/* Specify static text (C string). */
void
VG_TextString(VG_Text *vt, const char *s)
{
	VG_Lock(VGNODE(vt)->vg);
	if (s != NULL) {
		Strlcpy(vt->text, s, sizeof(vt->text));
	} else {
		vt->text[0] = '\0';
	}
	VG_Unlock(VGNODE(vt)->vg);
}

/* Specify static text (format string). */
void
VG_TextPrintf(VG_Text *vt, const char *fmt, ...)
{
	va_list ap;

	VG_Lock(VGNODE(vt)->vg);
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(vt->text, sizeof(vt->text), fmt, ap);
		va_end(ap);
	} else {
		vt->text[0] = '\0';
	}
	VG_Unlock(VGNODE(vt)->vg);
}

static void
RenderText(VG_Text *vt, char *sIn, VG_View *vv)
{
	char sSubst[VG_TEXT_MAX], *s;
	VG_Vector v1, v2, vMid;
	int x, y;

	if (vt->vsObj != NULL) {
		AG_VariableSubst(vt->vsObj, sIn, sSubst, sizeof(sSubst));
		s = sSubst;
	} else {
		s = sIn;
	}

	AG_PushTextState();

	if (vt->fontFace[0] != '\0' &&
	   ((agGUI && vt->fontSize != agDefaultFont->size) ||
	    (agGUI && vt->fontFlags != agDefaultFont->flags))) {
		AG_TextFontLookup(vt->fontFace, vt->fontSize, vt->fontFlags);
	}
	AG_TextColor(VG_MapColorRGB(VGNODE(vt)->color));

	v1 = VG_Pos(vt->p1);
	v2 = VG_Pos(vt->p2);
	vMid.x = v1.x + (v2.x - v1.x)/2.0f;
	vMid.y = v1.y + (v2.y - v1.y)/2.0f;
	VG_GetViewCoords(vv, vMid, &x, &y);

	VG_DrawText(vv, x, y,
	    VG_Degrees(VG_Atan2(v1.y-v2.y, v1.x-v2.x)),
	    s);

	AG_PopTextState();
}

static void
RenderTextPolled(VG_Text *vt, VG_View *vv)
{
	char val[64], s[VG_TEXT_MAX], *c;
	int argIdx = 0;

	s[0] = '\0';
	for (c = &vt->text[0]; *c != '\0'; ) {
		if (c[0] != '%') {
			val[0] = *c;
			val[1] = '\0';
			Strlcat(s, val, sizeof(s));
			c++;
			continue;
		}
		if (c[1] == '\0' || c[1] == '%') {
			val[0] = '%';
			val[1] = '\0';
			Strlcat(s, val, sizeof(s));
			c+=2;
			continue;
		}
		if ((argIdx+1) >= vt->args->n) {
			AG_FatalError("Argument inconsistency");
		}
		AG_PrintVariable(val, sizeof(val), &vt->args->v[argIdx]);
		Strlcat(s, val, sizeof(s));
		c += vt->argSizes[argIdx++];
	}
	RenderText(vt, s, vv);
}

static void
Draw(void *p, VG_View *vv)
{
	VG_Text *vt = p;

	if (vt->args != NULL) {
		RenderTextPolled(vt, vv);
	} else {
		RenderText(vt, vt->text, vv);
	}
}

static void
Extent(void *p, VG_View *vv, VG_Vector *a, VG_Vector *b)
{
	VG_Text *vt = p;
	float wText, hText;
	VG_Vector v1, v2;
	int su;

	su = AG_TextCacheGet(vv->tCache, vt->text);
	wText = (float)WSURFACE(vv,su)->w/vv->scale;
	hText = (float)WSURFACE(vv,su)->h/vv->scale;
	v1 = VG_Pos(vt->p1);
	v2 = VG_Pos(vt->p2);
	
	a->x = MIN(v1.x,v2.x) - wText/2.0f;
	a->y = MIN(v1.y,v2.y) - hText/2.0f;
	b->x = MAX(v1.x,v2.x) + hText/2.0f;
	b->y = MAX(v1.y,v2.y) + hText/2.0f;
}

static float
PointProximity(void *p, VG_View *vv, VG_Vector *vPt)
{
	VG_Text *vt = p;
	VG_Vector v1 = VG_Pos(vt->p1);
	VG_Vector v2 = VG_Pos(vt->p2);

	/* XXX TODO */
	return VG_PointLineDistance(v1, v2, vPt);
}

static void
Delete(void *p)
{
	VG_Text *vt = p;

	if (VG_DelRef(vt, vt->p1) == 0)
		VG_Delete(vt->p1);
	if (VG_DelRef(vt, vt->p2) == 0)
		VG_Delete(vt->p2);
}

static void
SetAlign(AG_Event *event)
{
	VG_Text *vt = AG_PTR(1);
	enum vg_alignment align = (enum vg_alignment)AG_INT(2);

	vt->align = align;
}

static void
SelectFont(AG_Event *event)
{
	VG_Text *vt = AG_PTR(1);
	AG_Window *win = AG_PTR(2);
	AG_FontSelector *fs = AG_PTR(3);

	Strlcpy(vt->fontFace, fs->curFace, sizeof(vt->fontFace));
	vt->fontSize = fs->curSize;
	vt->fontFlags = 0;
	if (fs->curStyle & AG_FONT_BOLD) { vt->fontFlags |= VG_TEXT_BOLD; }
	if (fs->curStyle & AG_FONT_ITALIC) { vt->fontFlags |= VG_TEXT_ITALIC; }

	AG_ObjectDetach(win);
}

static void
SelectFontDlg(AG_Event *event)
{
	VG_Text *vt = AG_PTR(1);
	VG_View *vv = AG_PTR(2);
	AG_Window *win, *winParent;
	AG_FontSelector *fs;
	AG_Box *hBox;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Font selection"));

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(hBox, 0, _("OK"), SelectFont, "%p,%p,%p", vt, win, fs);
	AG_ButtonNewFn(hBox, 0, _("Close"), AG_WindowCloseGenEv, "%p", win);

	AG_WindowShow(win);
	if ((winParent = AG_ParentWindow(vv)) != NULL)
		AG_WindowAttach(winParent, win);
}

static void *
Edit(void *p, VG_View *vv)
{
	VG_Text *vt = p;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_Pane *vPane;
	AG_Textbox *tb;
	AG_Box *bAl, *bAlv;

	vPane = AG_PaneNewVert(box, AG_PANE_EXPAND);

	AG_LabelNew(vPane->div[0], 0, _("Text: "));
	tb = AG_TextboxNewS(vPane->div[0],
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXPAND,
	    NULL);
	AG_TextboxBindUTF8(tb, vt->text, sizeof(vt->text));

	bAlv = AG_BoxNewVertNS(vPane->div[1], AG_BOX_HFILL|AG_BOX_FRAME);
	AG_LabelNew(bAlv, 0, _("Alignment: "));
	bAl = AG_BoxNewHorizNS(bAlv, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(bAl, 0, _("TL"), SetAlign, "%p,%i", vt, VG_ALIGN_TL);
	AG_ButtonNewFn(bAl, 0, _("TC"), SetAlign, "%p,%i", vt, VG_ALIGN_TC);
	AG_ButtonNewFn(bAl, 0, _("TR"), SetAlign, "%p,%i", vt, VG_ALIGN_TR);
	bAl = AG_BoxNewHorizNS(bAlv, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(bAl, 0, _("ML"), SetAlign, "%p,%i", vt, VG_ALIGN_ML);
	AG_ButtonNewFn(bAl, 0, _("MC"), SetAlign, "%p,%i", vt, VG_ALIGN_MC);
	AG_ButtonNewFn(bAl, 0, _("MR"), SetAlign, "%p,%i", vt, VG_ALIGN_MR);
	bAl = AG_BoxNewHorizNS(bAlv, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(bAl, 0, _("BL"), SetAlign, "%p,%i", vt, VG_ALIGN_BL);
	AG_ButtonNewFn(bAl, 0, _("BC"), SetAlign, "%p,%i", vt, VG_ALIGN_BC);
	AG_ButtonNewFn(bAl, 0, _("BR"), SetAlign, "%p,%i", vt, VG_ALIGN_BR);

	AG_ButtonNewFn(vPane->div[1], AG_BUTTON_HFILL, _("Select font"),
	    SelectFontDlg, "%p,%p", vt, vv);
	AG_CheckboxNewFlag(vPane->div[1], 0, _("Underline"),
	    &vt->fontFlags, VG_TEXT_UNDERLINE);
	AG_CheckboxNewFlag(vPane->div[1], 0, _("Scale to view"),
	    &vt->fontFlags, VG_TEXT_SCALED);

	return (box);
}

VG_NodeOps vgTextOps = {
	N_("Text"),
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
	NULL,			/* moveNode */
	Edit
};
