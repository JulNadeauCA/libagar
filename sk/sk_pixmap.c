/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Pixmap scaled and centered around a given point.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Pixmap *
SK_PixmapNew(void *pnode, SK_Point *pt)
{
	SK_Pixmap *px;

	px = Malloc(sizeof(SK_Pixmap));
	SK_PixmapInit(px, SK_GenNodeName(SKNODE(pnode)->sk, "Pixmap"));
	px->p = (pt != NULL) ? pt : SK_PointNew(pnode);
	SK_NodeAddReference(px, px->p);
	SK_NodeAttach(pnode, px);
	return (px);
}

void
SK_PixmapInit(void *p, Uint name)
{
	SK_Pixmap *px = p;

	SK_NodeInit(px, &skPixmapOps, name, 0);
	px->p = NULL;
	px->w = 0.0;
	px->h = 0.0;
	px->alpha = 1.0;
	px->s = -1;
	px->sSrc = NULL;
}

void
SK_PixmapDestroy(void *p)
{
#if 0
	SK_Pixmap *px = p;

	if (px->sSrc != NULL)
		AG_SurfaceFree(px->sSrc);
#endif
}

int
SK_PixmapLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Pixmap *px = p;

	px->p = SK_ReadRef(buf, sk, "Point");
	if (px->p == NULL) {
		AG_SetError("Missing center point (%s)", AG_GetError());
		return (-1);
	}
	px->w = M_ReadReal(buf);
	px->h = M_ReadReal(buf);
	px->alpha = M_ReadReal(buf);
	if ((px->sSrc = AG_ReadSurface(buf)) == NULL) {
		return (-1);
	}
	SK_NodeAddReference(px, px->p);
	return (0);
}

int
SK_PixmapSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Pixmap *px = p;

	SK_WriteRef(buf, px->p);
	M_WriteReal(buf, px->w);
	M_WriteReal(buf, px->h);
	M_WriteReal(buf, px->alpha);
	AG_WriteSurface(buf, px->sSrc);
	return (0);
}

void
SK_PixmapDraw(void *p, SK_View *skv)
{
	SK_Pixmap *px = p;
	M_Vector3 v = SK_Pos(px->p);

	SK_PointDraw(px->p, skv);

	if (px->w < skv->wPixel || px->h < skv->hPixel || px->sSrc == NULL) {
		return;
	}
	if (px->s == -1) {
		if ((px->s = AG_WidgetMapSurface(skv, px->sSrc)) == -1)
			return;
	}

	GL_PushMatrix();
	GL_Translatev(&v);

	AG_PushBlendingMode(skv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	AG_WidgetBlitSurfaceGL(skv, px->s, (float)px->w, (float)px->h);
	AG_PopBlendingMode(skv);

	GL_PopMatrix();
}

void
SK_PixmapEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Pixmap *px = p;
	const char *unit = skv->sk->uLen->abbr;

	M_NumericalNewReal(box, 0, unit, _("Width: "), &px->w);
	M_NumericalNewReal(box, 0, unit, _("Height: "), &px->h);
	M_NumericalNewReal(box, 0, unit, _("Alpha: "), &px->alpha);
}

M_Real
SK_PixmapProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	return (M_INFINITY);
}

int
SK_PixmapDelete(void *p)
{
	SK_Pixmap *px = p;
	SK *sk = SKNODE(px)->sk;
	int rv;

	SK_NodeDelReference(px, px->p);
	if (SKNODE(px->p)->nRefs == 0) {
		SK_NodeDel(px->p);
	}
	rv = SK_NodeDel(px);
	SK_Update(sk);
	return (rv);
}

int
SK_PixmapMove(void *p, const M_Vector3 *pos, const M_Vector3 *vel)
{
	SK_Pixmap *px = p;

	if (!(SKNODE(px->p)->flags & SK_NODE_MOVED)) {
		SK_Translatev(px->p, vel);
		SKNODE(px->p)->flags |= SK_NODE_MOVED;
	}
	return (1);
}

SK_Status
SK_PixmapConstrained(void *p)
{
	SK_Pixmap *px = p;

	if (SKNODE(px->p)->nEdges == 2) {
		if (SKNODE(px)->nEdges == 1) {
			return (SK_WELL_CONSTRAINED);
		} else if (SKNODE(px)->nEdges < 1) {
			return (SK_UNDER_CONSTRAINED);
		} else if (SKNODE(px)->nEdges > 1) {
			return (SK_OVER_CONSTRAINED);
		}
	} else if (SKNODE(px->p)->nEdges < 2) {
		return (SK_UNDER_CONSTRAINED);
	}
	return (SK_OVER_CONSTRAINED);
}

void
SK_PixmapDimensions(SK_Pixmap *px, M_Real w, M_Real h)
{
	SK_LockNode(px);
	px->w = w;
	px->h = h;
	SK_UnlockNode(px);
}

void
SK_PixmapAlpha(SK_Pixmap *px, M_Real a)
{
	SK_LockNode(px);
	px->alpha = a;
	SK_UnlockNode(px);
}

void
SK_PixmapSurface(SK_Pixmap *px, AG_Surface *s)
{
	SK_LockNode(px);
	px->sSrc = s;
	if (px->s != -1) {		/* XXX */
		px->s = -1;
	}
	SK_UnlockNode(px);
}

SK_NodeOps skPixmapOps = {
	"Pixmap",
	sizeof(SK_Pixmap),
	0,
	SK_PixmapInit,
	SK_PixmapDestroy,
	SK_PixmapLoad,
	SK_PixmapSave,
	SK_PixmapDraw,
	NULL,		/* redraw */
	SK_PixmapEdit,
	SK_PixmapProximity,
	SK_PixmapDelete,
	SK_PixmapMove,
	SK_PixmapConstrained
};

struct sk_pixmap_tool {
	SK_Tool tool;
	SK_Pixmap *curPx;
};

static void
ToolInit(void *_Nonnull p)
{
	struct sk_pixmap_tool *t = p;

	t->curPx = NULL;
}

static SK_Point *_Nullable
OverPoint(SK_View *_Nonnull skv, const M_Vector3 *_Nonnull pos,
    M_Vector3 *_Nonnull vC, void *_Nullable ignore)
{
	SK *sk = skv->sk;
	SK_Node *node;
	
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
	}
	if ((node = SK_ProximitySearch(sk, "Point", pos, vC, ignore)) != NULL &&
	    M_VecDistance3p(pos, vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		return ((SK_Point *)node);
	}
	return (NULL);
}

static SK_Pixmap *_Nullable
InsertPixmap(struct sk_pixmap_tool *_Nonnull t, SK_View *_Nonnull skv,
    const M_Vector3 *_Nonnull pos)
{
	SK *sk = skv->sk;
	SK_Pixmap *px;
	SK_Point *pOver;
	M_Vector3 vC;

	if (t->curPx != NULL) {
		t->curPx = NULL;
		return (NULL);
	}
	pOver = OverPoint(skv, pos, &vC, NULL);
	px = SK_PixmapNew(sk->root, pOver);
	if (pOver == NULL) {
		SK_Translatev(px->p, pos);
	}
	t->curPx = px;
	SK_Update(sk);
	return (px);
}

static void
LoadFromBMP(AG_Event *_Nonnull event)
{
	struct sk_pixmap_tool *t = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	M_Vector3 *pos = AG_PTR(3);
	char *path = AG_STRING(4);
	AG_Surface *s;
	SK_Pixmap *px;

	AG_ObjectLock(skv->sk);
	if ((s = AG_SurfaceFromBMP(path)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	if ((px = InsertPixmap(t, skv, pos)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	SK_PixmapSurface(px, s);
	SK_PixmapDimensions(px, s->w*skv->wPixel, s->h*skv->hPixel);
out:
	Free(pos);
	AG_ObjectUnlock(skv->sk);
}

static void
LoadFromPNG(AG_Event *_Nonnull event)
{
	struct sk_pixmap_tool *t = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	M_Vector3 *pos = AG_PTR(3);
	char *path = AG_STRING(4);
	AG_Surface *s;
	SK_Pixmap *px;

	AG_ObjectLock(skv->sk);
	if ((s = AG_SurfaceFromPNG(path)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	if ((px = InsertPixmap(t, skv, pos)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	SK_PixmapSurface(px, s);
	SK_PixmapDimensions(px, s->w*skv->wPixel, s->h*skv->hPixel);
out:
	Free(pos);
	AG_ObjectUnlock(skv->sk);
}

static void
LoadFromJPEG(AG_Event *_Nonnull event)
{
	struct sk_pixmap_tool *t = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	M_Vector3 *pos = AG_PTR(3);
	char *path = AG_STRING(4);
	AG_Surface *s;
	SK_Pixmap *px;

	AG_ObjectLock(skv->sk);
	if ((s = AG_SurfaceFromJPEG(path)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	if ((px = InsertPixmap(t, skv, pos)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	SK_PixmapSurface(px, s);
	SK_PixmapDimensions(px, s->w*skv->wPixel, s->h*skv->hPixel);
out:
	Free(pos);
	AG_ObjectUnlock(skv->sk);
}

static int
ToolMouseButtonDown(void *_Nonnull p, M_Vector3 pos, int btn)
{
	struct sk_pixmap_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	M_Vector3 *posDup;
	AG_Window *win;
	AG_FileDlg *fd;

	if (btn != AG_MOUSE_LEFT) {
		return (0);
	}
	if ((win = AG_WindowNew(0)) == NULL)
		return (0);

	fd = AG_FileDlgNewMRU(win, "sk-pixmaps",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND |
	    AG_FILEDLG_MASK_EXT);

	posDup = M_VectorDup3(&pos);
	AG_FileDlgAddType(fd, _("PC Bitmap"), "*.bmp",
	    LoadFromBMP, "%p,%p,%p", t, skv, posDup);
	AG_FileDlgAddType(fd, _("PNG image"), "*.png",
	    LoadFromPNG, "%p,%p,%p", t, skv, posDup);
	AG_FileDlgAddType(fd, _("JPEG image"), "*.jpg,*.jpeg",
	    LoadFromJPEG, "%p,%p,%p", t, skv, posDup);
	AG_WindowShow(win);
	return (1);
}

SK_ToolOps skPixmapToolOps = {
	N_("Pixmap"),
	N_("Insert a pixmap into the sketch"),
	NULL,
	sizeof(struct sk_pixmap_tool),
	0,
	ToolInit,
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* mousemotion */
	ToolMouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
