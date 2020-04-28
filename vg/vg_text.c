/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/textbox.h>
#include <agar/gui/font_selector.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/opengl.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

#include <stdarg.h>
#include <string.h>

VG_Text *
VG_TextNew(void *pNode, VG_Point *p1, VG_Point *p2)
{
	VG_Text *vt;

	vt = (VG_Text *)AG_Malloc(sizeof(VG_Text));
	VG_NodeInit(vt, &vgTextOps);
	vt->p1 = p1;
	vt->p2 = p2;
	VG_AddRef(vt, p1);
	VG_AddRef(vt, p2);
	VG_NodeAttach(pNode, vt);
	return (vt);
}

void
VG_TextAlignment(VG_Text *vt, enum vg_alignment align)
{
	vt->align = align;
}

void
VG_TextFontFace(VG_Text *vt, const char *face)
{
	VG *vg = VGNODE(vt)->vg;

	AG_ObjectLock(vg);
	AG_Strlcpy(vt->fontFace, face, sizeof(vt->fontFace));
	AG_ObjectUnlock(vg);
}

void
VG_TextFontSize(VG_Text *vt, float sizePts)
{
	vt->fontSize = sizePts;
}

void
VG_TextFontFlags(VG_Text *vt, Uint flags)
{
	vt->fontFlags = flags;
}

void
VG_TextSubstObject(VG_Text *vt, void *obj)
{
	VG *vg = VGNODE(vt)->vg;

	AG_ObjectLock(vg);
	vt->vsObj = obj;
	AG_ObjectUnlock(vg);
}

static void
Init(void *_Nonnull obj)
{
	VG_Text *vt = obj;

	vt->text[0] = '\0';
	vt->p1 = NULL;
	vt->p2 = NULL;
	vt->align = VG_ALIGN_MC;
	vt->fontFlags = vgGUI ? agDefaultFont->flags : 0;
	vt->fontSize = vgGUI ? agDefaultFont->spec.size : 12.0f;
	vt->fontFace[0] = '\0';
	vt->argsCount = 0;
	vt->args = NULL;
	vt->argSizes = NULL;
	vt->vsObj = NULL;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
    const AG_Version *_Nonnull ver)
{
	VG_Text *vt = obj;

	if ((vt->p1 = VG_ReadRef(ds, vt, "Point")) == NULL ||
	    (vt->p2 = VG_ReadRef(ds, vt, "Point")) == NULL)
		return (-1);

	vt->align = (enum vg_alignment)AG_ReadUint8(ds);
	AG_CopyString(vt->fontFace, ds, sizeof(vt->fontFace));
	vt->fontSize = AG_ReadFloat(ds);
	vt->fontFlags = (Uint)AG_ReadUint16(ds);
	AG_CopyString(vt->text, ds, sizeof(vt->text));
	return (0);
}

static void
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	VG_Text *vt = obj;

	VG_WriteRef(ds, vt->p1);
	VG_WriteRef(ds, vt->p2);
	AG_WriteUint8(ds, (Uint8)vt->align);
	AG_WriteString(ds, vt->fontFace);
	AG_WriteFloat(ds, vt->fontSize);
	AG_WriteUint16(ds, (Uint16)vt->fontFlags);
	AG_WriteString(ds, vt->text);
}

/* Specify static text (C string). */
void
VG_TextString(VG_Text *vt, const char *s)
{
	VG *vg = VGNODE(vt)->vg;

	AG_ObjectLock(vg);

	if (s != NULL) {
		Strlcpy(vt->text, s, sizeof(vt->text));
	} else {
		vt->text[0] = '\0';
	}

	AG_ObjectUnlock(vg);
}

/* Specify static text (format string). */
void
VG_TextPrintf(VG_Text *vt, const char *fmt, ...)
{
	VG *vg = VGNODE(vt)->vg;
	va_list ap;

	AG_ObjectLock(vg);

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(vt->text, sizeof(vt->text), fmt, ap);
		va_end(ap);
	} else {
		vt->text[0] = '\0';
	}

	AG_ObjectUnlock(vg);
}

static void
RenderText(VG_Text *_Nonnull vt, char *_Nonnull sIn, VG_View *_Nonnull vv)
{
	char sSubst[VG_TEXT_MAX], *s;
	VG_Vector v1, v2, vMid;
	AG_Color c;
	int x, y;
	int su;

	if (vt->vsObj != NULL) {
		AG_VariableSubst(vt->vsObj, sIn, sSubst, sizeof(sSubst));
		s = sSubst;
	} else {
		s = sIn;
	}

	AG_PushTextState();

	if (vt->fontFace[0] != '\0' &&
	   ((vgGUI && VG_Fabs(vt->fontSize - agDefaultFont->spec.size) < AG_FONT_PTS_EPSILON) ||
	    (vgGUI && vt->fontFlags != agDefaultFont->flags))) {
		AG_TextFontLookup(vt->fontFace, vt->fontSize, vt->fontFlags);
	}
	c = VG_MapColorRGB(VGNODE(vt)->color);
	AG_TextColor(&c);

	v1 = VG_Pos(vt->p1);
	v2 = VG_Pos(vt->p2);
	vMid.x = v1.x + (v2.x - v1.x)/2.0f;
	vMid.y = v1.y + (v2.y - v1.y)/2.0f;
	VG_GetViewCoords(vv, vMid, &x, &y);
	
	if ((su = AG_TextCacheGet(vv->tCache, s)) != -1) {
		VG_DrawSurface(vv, x, y,
		    VG_Degrees(VG_Atan2(v1.y-v2.y, v1.x-v2.x)),
		    su);
	}
	AG_PopTextState();
}

#if 0
static void
RenderTextPolled(VG_Text *_Nonnull vt, VG_View *_Nonnull vv)
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
		if ((argIdx+1) >= vt->argsCount) {
			AG_FatalError("Argument inconsistency");
		}
		AG_PrintVariable(val, sizeof(val), &vt->args[argIdx]);
		Strlcat(s, val, sizeof(s));
		c += vt->argSizes[argIdx++];
	}
	RenderText(vt, s, vv);
}
#endif

static void
Draw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Text *vt = obj;

#if 0
	if (vt->args != NULL) {
		RenderTextPolled(vt, vv);
	} else
#endif
	{
		RenderText(vt, vt->text, vv);
	}
}

static void
Extent(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull a,
    VG_Vector *_Nonnull b)
{
	VG_Text *vt = obj;
	float wText, hText;
	VG_Vector v1, v2;
	int su;

	if ((su = AG_TextCacheGet(vv->tCache, vt->text)) == -1) {
		a->x = 0.0f;
		a->y = 0.0f;
		return;
	}
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
PointProximity(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull vPt)
{
	VG_Text *vt = obj;
	VG_Vector v1 = VG_Pos(vt->p1);
	VG_Vector v2 = VG_Pos(vt->p2);

	/* XXX TODO */
	return VG_PointLineDistance(v1, v2, vPt);
}

static void
Delete(void *_Nonnull obj)
{
	VG_Text *vt = obj;

	if (VG_DelRef(vt, vt->p1) == 0)
		VG_Delete(vt->p1);
	if (VG_DelRef(vt, vt->p2) == 0)
		VG_Delete(vt->p2);
}

static void
SetAlign(AG_Event *_Nonnull event)
{
	VG_Text *vt = AG_PTR(1);
	enum vg_alignment align = (enum vg_alignment)AG_INT(2);

	vt->align = align;
}

static void
SelectFont(AG_Event *_Nonnull event)
{
	VG_Text *vt = AG_PTR(1);
	AG_Window *win = AG_WINDOW_PTR(2);
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(3);

	Strlcpy(vt->fontFace, fs->curFace, sizeof(vt->fontFace));
	vt->fontSize = fs->curSize;
	vt->fontFlags = 0;
	if (fs->curStyle & AG_FONT_BOLD) { vt->fontFlags |= VG_TEXT_BOLD; }
	if (fs->curStyle & AG_FONT_ITALIC) { vt->fontFlags |= VG_TEXT_ITALIC; }

	AG_ObjectDetach(win);
}

static void
SelectFontDlg(AG_Event *_Nonnull event)
{
	VG_Text *vt = AG_PTR(1);
	VG_View *vv = VG_VIEW_PTR(2);
	AG_Window *win, *winParent;
	AG_FontSelector *fs;
	AG_Box *hBox;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Font selection"));

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(hBox, 0, _("OK"), SelectFont, "%p,%p,%p", vt, win, fs);
	AG_ButtonNewFn(hBox, 0, _("Close"), AGWINCLOSE(win));

	AG_WindowShow(win);
	if ((winParent = AG_ParentWindow(vv)) != NULL)
		AG_WindowAttach(winParent, win);
}

static void *
Edit(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Text *vt = obj;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_Pane *vPane;
	AG_Textbox *tb;
	AG_Box *bAl, *bAlv;

	vPane = AG_PaneNewVert(box, AG_PANE_EXPAND);

	AG_LabelNew(vPane->div[0], 0, _("Text: "));
	tb = AG_TextboxNewS(vPane->div[0],
	    AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND,
	    NULL);
#ifdef AG_UNICODE
	AG_TextboxBindUTF8(tb, vt->text, sizeof(vt->text));
#else
	AG_TextboxBindASCII(tb, vt->text, sizeof(vt->text));
#endif

	bAlv = AG_BoxNewVert(vPane->div[1], AG_BOX_HFILL | AG_BOX_NO_SPACING);
	AG_LabelNew(bAlv, 0, _("Alignment: "));
	bAl = AG_BoxNewHoriz(bAlv, AG_BOX_HFILL | AG_BOX_HOMOGENOUS |
	                           AG_BOX_NO_SPACING);
	AG_ButtonNewFn(bAl, 0, _("TL"), SetAlign, "%p,%i", vt, VG_ALIGN_TL);
	AG_ButtonNewFn(bAl, 0, _("TC"), SetAlign, "%p,%i", vt, VG_ALIGN_TC);
	AG_ButtonNewFn(bAl, 0, _("TR"), SetAlign, "%p,%i", vt, VG_ALIGN_TR);

	bAl = AG_BoxNewHoriz(bAlv, AG_BOX_HFILL | AG_BOX_HOMOGENOUS |
	                           AG_BOX_NO_SPACING);
	AG_ButtonNewFn(bAl, 0, _("ML"), SetAlign, "%p,%i", vt, VG_ALIGN_ML);
	AG_ButtonNewFn(bAl, 0, _("MC"), SetAlign, "%p,%i", vt, VG_ALIGN_MC);
	AG_ButtonNewFn(bAl, 0, _("MR"), SetAlign, "%p,%i", vt, VG_ALIGN_MR);

	bAl = AG_BoxNewHoriz(bAlv, AG_BOX_HFILL | AG_BOX_HOMOGENOUS |
	                           AG_BOX_NO_SPACING);
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
