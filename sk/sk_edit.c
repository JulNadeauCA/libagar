/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Graphical interface for edition of SK(3) objects.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

#include <string.h>
#include <math.h>

extern SK_ToolOps skSelectToolOps;
extern SK_ToolOps skPointToolOps;
extern SK_ToolOps skLineToolOps;
extern SK_ToolOps skPolygonToolOps;
extern SK_ToolOps skCircleToolOps;
extern SK_ToolOps skDimensionToolOps;
extern SK_ToolOps skPixmapToolOps;
extern SK_ToolOps skMeasureToolOps;

SK_ToolOps *skToolkit[] = {
	&skSelectToolOps,
	&skPointToolOps,
	&skLineToolOps,
	&skPolygonToolOps,
	&skCircleToolOps,
	&skDimensionToolOps,
	&skPixmapToolOps,
	&skMeasureToolOps,
};
Uint skToolkitCount = sizeof(skToolkit) / sizeof(skToolkit[0]);

#if 0
static int
CompareClass(const char *_Nonnull pat, const char *_Nonnull cname)
{
	const char *c;

	if (pat[0] == '*' && pat[1] == '\0') {
		return (0);
	}
	for (c = &pat[0]; *c != '\0'; c++) {
		if (c[0] == ':' && c[1] == '*' && c[2] == '\0') {
			if (c == &pat[0])
				return (0);
			if (strncmp(cname, pat, c - &pat[0] + 1) == 0)
				return (0);
		}
	}
	return (1);
}

static void
ListLibraryItems(AG_Tlist *_Nonnull tl, const char *_Nonnull cname, int depth)
{
	char subname[SK_TYPE_NAME_MAX];
	const char *c;
	AG_TlistItem *it;
	int i, j;

	for (i = 0; i < skElementsCnt; i++) {
		SK_NodeOps *ops = skElements[i];
		
		if (CompareClass(cname, ops->name) != 0) {
			continue;
		}
		for (c = ops->name, j = 0; *c != '\0'; c++) {
			if (*c == ':')
				j++;
		}
		if (j == depth) {
			it = AG_TlistAddS(tl, NULL, ops->name);
			it->p1 = ops;
			it->depth = depth;
			Strlcpy(subname, ops->name, sizeof(subname));
			Strlcat(subname, ":*", sizeof(subname));
			ListLibraryItems(tl, subname, depth+1);
		}
	}
}
#endif
static void
ImportSketchDlg(AG_Event *_Nonnull event)
{
	AG_Window *win;
/*	AG_FileDlg *dlg; */

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	(void)AG_FileDlgNewMRU(win, "sg-sketches",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);
#if 0
	AG_FileDlgAddType(dlg, _("SVG Format"), "*.svg",
	    ImportFromSVG, "%p", sk);
	AG_FileDlgAddType(dlg, _("DXF Format"), "*.dxf",
	    ImportFromDXF, "%p", sk);
	AG_FileDlgAddType(dlg, _("PDF Format"), "*.pdf",
	    ImportFromPDF, "%p", sk);
#endif
	AG_WindowShow(win);
}

static void
NodeDelete(AG_Event *_Nonnull event)
{
	SK_Node *node = AG_PTR(1);
	SK_View *skv = AG_PTR(2);

	SK_ViewClearEditPane(skv);
	if (node->ops->del != NULL) {
		if (node->ops->del(node) == -1)
			AG_TextMsgFromError();
	} else {
		AG_TextMsg(AG_MSG_ERROR, _("Entity cannot be deleted"));
	}
}

static void
NodeMoveUp(AG_Event *_Nonnull event)
{
	SK_Node *node = AG_PTR(1);

	SK_NodeMoveUp(node);
}

static void
NodeMoveDown(AG_Event *_Nonnull event)
{
	SK_Node *node = AG_PTR(1);

	SK_NodeMoveDown(node);
}

/*
 * Generate a plot of the proximity to a given node from every visible
 * pixel. This is useful for debugging proximity tests.
 */
static void
PlotProximityMap(AG_Event *_Nonnull event)
{
	SK_Node *node = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	SK *sk = skv->sk;
	SK_Pixmap *px;
	AG_Surface *s;
	int x, y;
	Uint8 *p, color;
	Uint32 c;
	M_Vector3 v, vC;
	M_Real prox, proxMax = 0.0;
	M_Matrix44 Tinv;
	M_Real *map;

	s = AG_SurfaceNew(agSurfaceFmt, AGWIDGET(skv)->w, AGWIDGET(skv)->h, 0);
	if (s == NULL) {
		AG_FatalError(NULL);
	}
	p = s->pixels;
	map = Malloc(s->w*s->h*sizeof(M_Real));
	Tinv = M_MatInvert44(skv->mView);
	v.z = 0.0;
	for (y = 0; y < s->h; y++) {
		for (x = 0; x < s->w; x++) {
			v.y = SK_VIEW_Y(skv, y);
			v.x = SK_VIEW_X(skv, x);
			v = M_VecFromProj3(M_MatMultVector44(Tinv,
			                   M_VecToProj3(v,1)));
			prox = node->ops->proximity(node, &v, &vC);
			if (prox != M_INFINITY && prox > proxMax) {
				proxMax = prox;
			}
			map[y*s->w + x] = node->ops->proximity(node, &v, &vC);
		}
	}
	for (y = 0; y < s->h; y++) {
		for (x = 0; x < s->w; x++) {
			M_Real m = map[y*s->w + (s->w - x)];

			if (m < 2.0) {
				color = (Uint8)(m*255.0/2.0);
				c = AG_MapPixel32_RGB8(&s->format,
				    255-color, 0, 0);
			} else {
				color = (Uint8)(m*255.0/proxMax);
				c = AG_MapPixel32_RGB8(&s->format,
				    color, color, color);
			}
			AG_SurfacePut32_At(s, p, c);
			p += s->format.BytesPerPixel;
		}
	}
	px = SK_PixmapNew(sk->root, NULL);
	SK_PixmapDimensions(px, s->w*skv->wPixel, s->h*skv->hPixel);
	SK_PixmapSurface(px, s);
	SK_NodeMoveHead(px);
	Free(map);
}

static void
PlotCluster(SK *_Nonnull sk, AG_Graph *_Nonnull gf, SK_Cluster *_Nonnull cl)
{
	SK_Constraint *ct;
	AG_GraphVertex *v1, *v2;
	AG_GraphEdge *edge;

	AG_GraphFreeVertices(gf);

	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if ((v1 = AG_GraphVertexFind(gf, ct->n1)) == NULL) {
			v1 = AG_GraphVertexNew(gf, ct->n1);
			AG_GraphVertexLabelS(v1, ct->n1->name);
		}
		if ((v2 = AG_GraphVertexFind(gf, ct->n2)) == NULL) {
			v2 = AG_GraphVertexNew(gf, ct->n2);
			AG_GraphVertexLabelS(v2, ct->n2->name);
		}
		edge = AG_GraphEdgeNew(gf, v1, v2, ct);
		AG_GraphEdgeLabelS(edge,
		    (ct->type == SK_DISTANCE && ct->ct_distance == 0.0) ?
		    "Incidence" : skConstraintNames[ct->type]);
	}
	AG_GraphAutoPlace(gf, 800, 800);
}

static void
SelectCluster(AG_Event *_Nonnull event)
{
	SK *sk = AG_PTR(1);
	AG_Graph *gf = AG_PTR(2);
	AG_TlistItem *ti = AG_PTR(3);
	SK_Cluster *cluster = ti->p1;

	PlotCluster(sk, gf, cluster);
}

static void
PollClusters(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Cluster *cl;
	AG_TlistItem *ti;

	AG_TlistBegin(tl);
	AG_TlistAddPtr(tl, NULL, "Original", &sk->ctGraph);
	TAILQ_FOREACH(cl, &sk->clusters, clusters) {
		ti = AG_TlistAdd(tl, NULL, "Cluster%d", cl->name);
		ti->p1 = cl;
	}
	AG_TlistEnd(tl);
}

static void
ViewConstraintGraphs(AG_Event *_Nonnull event)
{
	AG_Window *winParent = AG_PTR(1);
	SK *sk = AG_PTR(2);
	AG_Graph *gf;
	AG_Window *win;
	AG_Pane *pane;
	AG_Tlist *tl;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Constraint graph of <%s>"),
	    OBJECT(sk)->name);
	
	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	gf = AG_GraphNew(pane->div[1], AG_GRAPH_EXPAND);
	tl = AG_TlistNewPolled(pane->div[0], AG_TLIST_EXPAND,
	    PollClusters, "%p", sk);
	AG_TlistSizeHint(tl, "<Original>", 6);
	AG_TlistSetDblClickFn(tl, SelectCluster, "%p,%p", sk, gf);

	PlotCluster(sk, gf, &sk->ctGraph);

	AG_WindowShow(win);
	AG_WindowAttach(winParent, win);
}

static void
CreateNewView(AG_Event *_Nonnull event)
{
	AG_Window *winParent = AG_PTR(1);
	SK *sk = AG_PTR(2);
	SK_View *skv;
	AG_Window *win;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, OBJECT(sk)->name);

	AG_SetStyle(win, "spacing", "0");
	AG_SetStyleF(win, "padding", "0 %d %d %d",
	    WIDGET(win)->paddingRight,
	    WIDGET(win)->paddingBottom,
	    WIDGET(win)->paddingLeft);

	skv = SK_ViewNew(win, sk, SK_VIEW_EXPAND);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_TR, 40, 30);
	AG_WidgetFocus(skv);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
NodeMenu(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	SK_Node *node = AG_TlistSelectedItemPtr(tl);
	SK_View *skv = AG_PTR(1);
	AG_PopupMenu *pm;

	pm = AG_PopupNew(skv);
	AG_MenuAction(pm->root, _("Delete entity"), agIconTrash.s,
	    NodeDelete, "%p,%p", node, skv);
	AG_MenuAction(pm->root, _("Move up"), NULL,
	    NodeMoveUp, "%p", node);
	AG_MenuAction(pm->root, _("Move down"), NULL,
	    NodeMoveDown, "%p", node);
	AG_MenuAction(pm->root, _("Plot proximity map"), NULL,
	    PlotProximityMap, "%p,%p", node, skv);
	AG_PopupShowAt(pm,
	    WIDGET(tl)->drv->mouse->x,
	    WIDGET(tl)->drv->mouse->y);
}

static void
FindNodes(AG_Tlist *_Nonnull tl, SK_Node *_Nonnull node, int depth)
{
	AG_TlistItem *it;
	SK_Node *cnode;

	it = AG_TlistAddS(tl, NULL, node->name);
	it->depth = depth;
	it->p1 = node;
	it->selected = (node->flags & SK_NODE_SELECTED);

	if (!TAILQ_EMPTY(&node->cnodes)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(cnode, &node->cnodes, sknodes)
			FindNodes(tl, cnode, depth+1);
	}
}

static void
PollNodes(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	SK *sk = AG_PTR(1);

	AG_TlistBegin(tl);
	FindNodes(tl, (SK_Node *)sk->root, 0);
	AG_TlistEnd(tl);
}

static void
NodeSelect(AG_Event *_Nonnull event)
{
	AG_TlistItem *it = AG_PTR(1);
	int state = AG_INT(2);
	SK_Node *node = it->p1;
	
	if (state) {
		node->flags |= SK_NODE_SELECTED;
	} else {
		node->flags &= ~(SK_NODE_SELECTED);
	}
}

static void
SelectTool(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	SK_Tool *tool = it->p1;

	SK_ViewSelectTool(skv, tool, NULL);
}

static void
NodeSetName(AG_Event *_Nonnull event)
{
	SK_Node *node = AG_PTR(1);

	AG_Snprintf(node->name, sizeof(node->name), "%s%u", node->ops->name,
	   (Uint)node->handle);
}

static void
PollConstraints(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Node *node = AG_PTR(2);
	SK_Constraint *ct;
	AG_TlistItem *it;

	AG_TlistBegin(tl);
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		char ctArgs[64];
		
		if (node != NULL) {
			if (node != ct->n1 && node != ct->n2)
				continue;
		}

		switch (ct->uType) {
		case SK_DISTANCE:
			AG_Snprintf(ctArgs, sizeof(ctArgs), "(%.03f %s)",
			    ct->ct_distance, sk->uLen->abbr);
			break;
		case SK_ANGLE:
			AG_Snprintf(ctArgs, sizeof(ctArgs), "(%.02f\xc2\xb0)",
			    Degrees(ct->ct_angle));
			break;
		default:
			ctArgs[0] = '\0';
			break;
		}
		it = AG_TlistAdd(tl, NULL, "%s%s: %s,%s",
		    skConstraintNames[ct->uType], ctArgs,
		    ct->n1->name, ct->n2->name);
		it->p1 = ct;
	}
	AG_TlistEnd(tl);
}

static void
NodeEditGeneric(SK_Node *_Nonnull node, AG_Widget *_Nonnull box,
    SK_View *_Nonnull skv)
{
	AG_Tlist *tl;
	AG_Numerical *num;

	AG_SeparatorNewHoriz(box);

	num = AG_NumericalNew(box, 0, NULL, _("Name: "));
	AG_BindUint(num, "value", &node->handle);
	AG_SetEvent(num, "numerical-changed", NodeSetName, "%p", node);

//	lbl = AG_LabelNewPolledMT(box, AG_LABEL_HFILL, &skv->sk->lock,
//	    "Flags: <%[flags]>", &node->flags);
//	AG_LabelFlag(lbl, 0, "SELECTED", SK_NODE_SELECTED);

	AG_LabelNewS(box, 0, _("Geometric constraints: "));
	tl = AG_TlistNewPolled(box, AG_TLIST_POLL | AG_TLIST_EXPAND,
	    PollConstraints, "%p,%p", SKNODE(node)->sk, node);
	AG_WidgetSetFocusable(tl, 0);
}

static void
NodeEdit(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);
	SK_Node *node = ti->p1;

	SK_ViewClearEditPane(skv);
	if (node->ops->edit == NULL) {
		return;
	}
	skv->editBox = (AG_Widget *)AG_BoxNewVert(skv->editPane->div[1],
	                                          AG_BOX_EXPAND);
	node->ops->edit(node, skv->editBox, skv);
	NodeEditGeneric(node, skv->editBox, skv);
	SK_ViewResizePanes(skv);
}

static void
UpdateConstraint(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_PTR(1);

	SK_Update(skv->sk);
}

static void
ConstraintEdit(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	SK_Constraint *ct = it->p1;
	AG_Numerical *num;
	
	SK_ViewClearEditPane(skv);
	skv->editBox = (AG_Widget *)
	    AG_BoxNew(skv->editPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
	AG_LabelNew(skv->editBox, AG_LABEL_HFILL, _("Type: %s"),
	    skConstraintNames[ct->uType]);

	switch (ct->uType) {
	case SK_DISTANCE:
		num = AG_NumericalNew(skv->editBox, AG_NUMERICAL_HFILL, "m",
		    _("Distance: "));
		M_BindReal(num, "value", &ct->ct_distance);
		M_SetReal(num, "min", 0.0);
		M_SetReal(num, "inc", 0.1);
		AG_WidgetSetFocusable(num, 0);
		AG_SetEvent(num, "numerical-changed",
		    UpdateConstraint, "%p,%p", skv, ct);
		break;
	case SK_ANGLE:
		num = AG_NumericalNew(skv->editBox, AG_NUMERICAL_HFILL,
		    "deg", _("Angle: "));
		M_BindReal(num, "value", &ct->ct_angle);
		M_SetReal(num, "inc", 1.0);
		AG_WidgetSetFocusable(num, 0);
		AG_SetEvent(num, "numerical-changed",
		    UpdateConstraint, "%p,%p", skv, ct);
		break;
	default:
		break;
	}
	SK_ViewResizePanes(skv);
}

static void
ConstraintDelete(AG_Event *_Nonnull event)
{
	SK_Constraint *ct = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	SK *sk = skv->sk;

	SK_ViewClearEditPane(skv);

	SK_NodeDelConstraint(ct->n1, ct);
	SK_NodeDelConstraint(ct->n2, ct);
	SK_DelConstraint(&sk->ctGraph, ct);
	SK_Update(sk);
}

static void
ConstraintMenu(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	SK_Constraint *ct = AG_TlistSelectedItemPtr(tl);
	SK_View *skv = AG_PTR(2);
	AG_PopupMenu *pm;

	pm = AG_PopupNew(skv);
	AG_MenuAction(pm->root, _("Delete constraint"), agIconTrash.s,
	    ConstraintDelete, "%p,%p", ct, skv);
	AG_PopupShowAt(pm,
	    WIDGET(tl)->drv->mouse->x,
	    WIDGET(tl)->drv->mouse->y);
}

static void
PollInsns(AG_Event *_Nonnull event)
{
	AG_Table *tbl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Insn *si;

	AG_TableBegin(tbl);
	TAILQ_FOREACH(si, &sk->insns, insns) {
		switch (si->type) {
		case SK_COMPOSE_PAIR:
			AG_TableAddRow(tbl, "%s:%s:%s:%s", "COMPOSE_PAIR",
			    si->n[0]->name, si->n[1]->name, "");
			break;
		case SK_COMPOSE_RING:
			AG_TableAddRow(tbl, "%s:%s:%s:%s", "COMPOSE_RING",
			    si->n[0]->name, si->n[1]->name, si->n[2]->name);
			break;
		}
	}
	AG_TableEnd(tbl);
}

static void
ExecSelectedInsns(AG_Event *_Nonnull event)
{
	SK *sk = AG_PTR(1);
	AG_Table *tbl = AG_PTR(2);
	SK_Insn *si;
	Uint m = 0;
	
	SK_ClearProgramState(sk);
	TAILQ_FOREACH(si, &sk->insns, insns) {
		if (!AG_TableRowSelected(tbl, m++)) {
			continue;
		}
		if (SK_ExecInsn(sk, si) == -1) {
			AG_TextMsgFromError();
		}
		break;
	}
}

static void
ExecInsns(AG_Event *_Nonnull event)
{
	SK *sk = AG_PTR(1);
	AG_Table *tbl = AG_PTR(2);
	SK_Insn *si;
	Uint m = 0;

	SK_ClearProgramState(sk);
	TAILQ_FOREACH(si, &sk->insns, insns) {
		AG_TableSelectRow(tbl, m++);
		if (SK_ExecInsn(sk, si) == -1) {
			AG_TextMsgFromError();
			break;
		}
	}
}

static void
OnOverlay(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_SELF();
	AG_Surface *lbl;
	AG_Rect r;
	int x, y;

	AG_PushTextState();
	switch (skv->sk->status) {
	case SK_WELL_CONSTRAINED:
		AG_TextColorRGB(255, 255, 255);
		break;
	case SK_UNDER_CONSTRAINED:
		AG_TextColorRGB(200, 200, 200);
		break;
	case SK_OVER_CONSTRAINED:
		AG_TextColorRGB(255, 255, 0);
		break;
	default:
		AG_TextColorRGB(255, 100, 100);
		break;
	}

	AG_PushBlendingMode(skv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	lbl = AG_TextRender(skv->sk->statusText);
	x = WIDGET(skv)->w - lbl->w;
	y = WIDGET(skv)->h - lbl->h;
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(skv);
	r.h = HEIGHT(skv);
	AG_PushClipRect(skv, &r);
	AG_WidgetBlit(skv, lbl, x, y);
	AG_PopClipRect(skv);
	
	AG_SurfaceFree(lbl);
	AG_PopTextState();

	AG_PopBlendingMode(skv);
}

void *
SK_Edit(void *p)
{
	extern SK_ToolOps *skToolkit[];
	extern Uint skToolkitCount;
	SK *sk = p;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	SK_View *skv;
	AG_Menu *menu;
	AG_MenuItem *pitem;
	AG_Pane *hp, *vp, *vp2;
	AG_Tlist *tl;
	AG_Table *tbl;
	SK_Tool *tool;
	Uint i;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(sk)->name);

	AG_SetStyle(win, "spacing", "0");
	AG_SetStyleF(win, "padding", "0 %d %d %d",
	    WIDGET(win)->paddingRight,
	    WIDGET(win)->paddingBottom,
	    WIDGET(win)->paddingLeft);

	skv = SK_ViewNew(NULL, sk, SK_VIEW_EXPAND);
	AG_SetEvent(skv, "widget-overlay", OnOverlay, NULL);
	
	for (i = 0; i < skToolkitCount; i++)
		SK_ViewRegTool(skv, skToolkit[i], NULL);

	menu = AG_MenuNew(win, AG_MENU_HFILL);
	pitem = AG_MenuNode(menu->root, _("File"), NULL);
	{
		AG_MenuAction(pitem, _("Import from vector image..."), NULL,
		    ImportSketchDlg, "%p,%p", sk, win);
		AG_MenuSeparator(pitem);
		AG_MenuActionKb(pitem, _("Close sketch"), NULL,
		    AG_KEY_W, AG_KEYMOD_CTRL, AGWINCLOSE(win));
	}
	pitem = AG_MenuNode(menu->root, _("Edit"), NULL);
	{
		/* TODO */
		AG_MenuAction(pitem, _("Undo"), NULL, NULL, "%p", NULL);
		AG_MenuAction(pitem, _("Redo"), NULL, NULL, "%p", NULL);
	}
	pitem = AG_MenuNode(menu->root, _("View"), NULL);
	{
		AG_MenuAction(pitem, _("New view..."), NULL,
		    CreateNewView, "%p,%p", win, sk);
		AG_MenuAction(pitem, _("Constraint graphs..."), NULL,
		    ViewConstraintGraphs, "%p,%p", win, sk);
	}
	pitem = AG_MenuNode(menu->root, _("Geometry"), NULL);
	{
		SK_GeometryMenu(skv, pitem);
	}
	
	hp = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT,
		    AG_PANE_EXPAND | AG_PANE_DIV1FILL);
		nb = AG_NotebookNew(vp->div[0], AG_NOTEBOOK_EXPAND);
		AG_ObjectAttach(hp->div[1], skv);

		SK_ViewSetEditPane(skv, vp);
		SK_ViewSetViewPane(skv, hp);

		ntab = AG_NotebookAdd(nb, _("Tools"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);
			AG_WidgetSetFocusable(tl, 0);
			TAILQ_FOREACH(tool, &skv->tools, tools) {
				AG_TlistAddPtr(tl,
				    (tool->ops->icon != NULL) ?
				     tool->ops->icon->s : NULL,
				    _(tool->ops->name), tool);
			}
			AG_SetEvent(tl, "tlist-selected", SelectTool,"%p",skv);
		}
		ntab = AG_NotebookAdd(nb, _("Nodes"), AG_BOX_VERT);
		{
			tl = AG_TlistNewPolled(ntab,
			    AG_TLIST_EXPAND | AG_TLIST_MULTI |
			    AG_TLIST_NOSELSTATE,
			    PollNodes, "%p", sk);

			AG_TlistSetDblClickFn(tl, NodeEdit, "%p", skv);
			AG_TlistSetPopupFn(tl, NodeMenu, "%p", skv);
			AG_TlistSetChangedFn(tl, NodeSelect, NULL);
			AG_TlistSizeHint(tl, _("<Polygon00>"), 4);
/*			AG_WidgetSetFocusable(tl, 0); */
		}
		ntab = AG_NotebookAdd(nb, _("Constraints"), AG_BOX_VERT);
		{
			AG_Box *hb;

			vp2 = AG_PaneNew(ntab, AG_PANE_VERT, AG_PANE_EXPAND |
			                                     AG_PANE_DIV1FILL);
			tl = AG_TlistNewPolled(vp2->div[0], AG_TLIST_EXPAND,
			    PollConstraints, "%p,%p", sk, NULL);

			AG_TlistSetDblClickFn(tl, ConstraintEdit, "%p", skv);
			AG_TlistSetPopupFn(tl,
			    ConstraintMenu, "%p,%p", sk, skv);
			AG_TlistSizeHint(tl, _("<Polygon00>"), 4);
/*			AG_WidgetSetFocusable(tl, 0); */

			tbl = AG_TableNewPolled(vp2->div[1],
			    AG_TABLE_MULTI|AG_TABLE_EXPAND,
			    PollInsns, "%p,%p", sk, NULL);

			AG_TableAddCol(tbl, _("Type"), "<COMPOSE_PAIR>", NULL);
			AG_TableAddCol(tbl, "n1", "<Circle0>", NULL);
			AG_TableAddCol(tbl, "n2", "<Circle0>", NULL);
			AG_TableAddCol(tbl, "n3", NULL, NULL);
/*			AG_WidgetSetFocusable(tbl, 0); */
			
			hb = AG_BoxNewHoriz(vp2->div[1], AG_BOX_HFILL |
			                                 AG_BOX_HOMOGENOUS);
			AG_ButtonNewFn(hb, 0, "All", ExecInsns, "%p,%p", sk, tbl);
			AG_ButtonNewFn(hb, 0, "Sel", ExecSelectedInsns, "%p,%p", sk, tbl);
		}
	}

	AG_PaneMoveDividerPct(hp, 40);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 50);
	AG_WidgetFocus(skv);
	return (win);
}
