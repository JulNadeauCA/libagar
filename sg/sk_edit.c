/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include <gui/window.h>
#include <gui/menu.h>
#include <gui/notebook.h>
#include <gui/tlist.h>
#include <gui/pane.h>
#include <gui/mpane.h>
#include <gui/file_dlg.h>
#include <gui/table.h>
#include <gui/separator.h>
#include <gui/label.h>
#include <gui/graph.h>
#include <gui/spinbutton.h>
#include <gui/hbox.h>
#include <gui/numerical.h>

#include "sk.h"
#include "sg_gui.h"

#include <string.h>
#include <math.h>

#ifdef EDITION

static void SK_NodeEditGeneric(SK_Node *, AG_Widget *, SK_View *);

extern SK_ToolOps skSelectToolOps;
extern SK_ToolOps skPointToolOps;
extern SK_ToolOps skLineToolOps;
extern SK_ToolOps skCircleToolOps;
extern SK_ToolOps skDimensionToolOps;
extern SK_ToolOps skMeasureToolOps;

SK_ToolOps *skToolkit[] = {
	&skSelectToolOps,
	&skPointToolOps,
	&skLineToolOps,
	&skCircleToolOps,
	&skDimensionToolOps,
	&skMeasureToolOps,
};
Uint skToolkitCount = sizeof(skToolkit) / sizeof(skToolkit[0]);

static int
CompareClass(const char *pat, const char *cname)
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
ListLibraryItems(AG_Tlist *tl, const char *cname, int depth)
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
			it = AG_TlistAdd(tl, AGICON(OBJ_ICON), "%s", ops->name);
			it->p1 = ops;
			it->depth = depth;
			strlcpy(subname, ops->name, sizeof(subname));
			strlcat(subname, ":*", sizeof(subname));
			ListLibraryItems(tl, subname, depth+1);
		}
	}
}

static void
ImportSketchDlg(AG_Event *event)
{
	SK *sk = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Window *win;
	AG_FileDlg *dlg;

	win = AG_WindowNew(0);
	dlg = AG_FileDlgNew(win, AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|
	                         AG_FILEDLG_EXPAND);
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
NodeDelete(AG_Event *event)
{
	SK_Node *node = AG_PTR(1);
	SK_View *skv = AG_PTR(2);

	SK_ViewCloseEditPane(skv);
	if (node->ops->del != NULL) {
		if (node->ops->del(node) == -1)
			AG_TextMsgFromError();
	} else {
		AG_TextMsg(AG_MSG_ERROR, _("Entity cannot be deleted"));
	}
	SK_Update(skv->sk);
}

static void
PlotCluster(SK *sk, AG_Graph *gf, SK_Cluster *cl)
{
	char nodeName[SK_NODE_NAME_MAX];
	SK_Constraint *ct;
	AG_GraphVertex *v1, *v2;
	AG_GraphEdge *edge;

	AG_GraphFreeVertices(gf);

	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if ((v1 = AG_GraphVertexFind(gf, ct->n1)) == NULL) {
			v1 = AG_GraphVertexNew(gf, ct->n1);
			AG_GraphVertexLabel(v1, "%s",
			    SK_NodeNameCopy(ct->n1, nodeName,
			    sizeof(nodeName)));
		}
		if ((v2 = AG_GraphVertexFind(gf, ct->n2)) == NULL) {
			v2 = AG_GraphVertexNew(gf, ct->n2);
			AG_GraphVertexLabel(v2, "%s",
			    SK_NodeNameCopy(ct->n2, nodeName,
			    sizeof(nodeName)));
		}
		edge = AG_GraphEdgeNew(gf, v1, v2, ct);
		AG_GraphEdgeLabel(edge, "%s", skConstraintNames[ct->type]);
	}
	AG_GraphAutoPlace(gf, 800, 800);
}

static void
SelectCluster(AG_Event *event)
{
	SK *sk = AG_PTR(1);
	AG_Graph *gf = AG_PTR(2);
	SK_Cluster *cl = AG_PTR(3);

	PlotCluster(sk, gf, cl);
}

static void
PollClusters(AG_Event *event)
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
ViewConstraintGraphs(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1);
	SK *sk = AG_PTR(2);
	AG_Graph *gf;
	AG_Window *win;
	SK_Node *node;
	AG_Pane *pane;
	AG_Tlist *tl;
	AG_TlistItem *ti;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Constraint graph of <%s>"),
	    OBJECT(sk)->name);
	
	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	gf = AG_GraphNew(pane->div[1], AG_GRAPH_EXPAND);
	tl = AG_TlistNewPolled(pane->div[0], AG_TLIST_EXPAND,
	    PollClusters, "%p", sk);
	AG_TlistPrescale(tl, "<Original>", 6);
	AG_TlistSetDblClickFn(tl,
	    SelectCluster, "%p,%p", sk, gf);
//	AG_ButtonNewFn(pane->div[0], AG_BUTTON_HFILL, _("Update"),
//	    SelectCluster, "%p,%p,%p", sk, gf, &sk->ctGraph);

	PlotCluster(sk, gf, &sk->ctGraph);

	AG_WindowShow(win);
	AG_WindowAttach(winParent, win);
}

static void
CreateNewView(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1);
	SK *sk = AG_PTR(2);
	SK_View *skv;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s", OBJECT(sk)->name);
	AG_WindowSetPaddingTop(win, 0);
	AG_WindowSetSpacing(win, 0);
	skv = SK_ViewNew(win, sk, SK_VIEW_EXPAND);
	AG_WindowSetGeometry(win, agView->w/6, agView->h/6,
	                     2*agView->w/3, 2*agView->h/3);
	AG_WidgetFocus(skv);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
NodeMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SK_Node *node = AG_TlistSelectedItemPtr(tl);
	SK *sk = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	AG_PopupMenu *pm;

	pm = AG_PopupNew(skv);
	AG_MenuAction(pm->item, _("Delete entity"), TRASH_ICON,
	    NodeDelete, "%p,%p", node, skv);
	AG_PopupShow(pm);
}

static void
FindNodes(AG_Tlist *tl, SK_Node *node, int depth)
{
	AG_TlistItem *it;
	SK_Node *cnode;

	it = AG_TlistAdd(tl, AGICON(EDA_NODE_ICON), "%s%u", node->ops->name,
	    node->name);
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
PollNodes(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SK *sk = AG_PTR(1);

	AG_TlistBegin(tl);
	FindNodes(tl, (SK_Node *)sk->root, 0);
	AG_TlistEnd(tl);
}

static void
NodeSelect(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
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
SelectTool(AG_Event *event)
{
	SK_View *skv = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	SK_Tool *tool = it->p1;

	SK_ViewSelectTool(skv, tool, NULL);
}

static void
NodeEdit(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Pane *hp = AG_PTR(1);
	AG_Pane *vp = AG_PTR(2);
	SK_View *skv = AG_PTR(3);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	AG_Window *pWin = AG_WidgetParentWindow(skv);
	SK_Node *node = it->p1;
	AG_SizeReq rDiv;

	SK_ViewCloseEditPane(skv);
	if (node->ops->edit == NULL) {
		return;
	}
	skv->editPane = (AG_Widget *)AG_BoxNew(vp->div[1], AG_BOX_VERT,
	                                       AG_BOX_EXPAND);
	node->ops->edit(node, skv->editPane, skv);
	SK_NodeEditGeneric(node, skv->editPane, skv);

	AG_WidgetSizeReq(vp->div[1], &rDiv);
	AG_PaneSetDivisionMin(vp, 1, -1, rDiv.h + vp->wDiv);
	AG_PaneSetDivisionMin(hp, 0, rDiv.w + vp->wDiv, -1);
	AG_PaneMoveDivider(vp, WIDGET(vp)->h - rDiv.h);
	AG_WindowUpdate(pWin);
}

static void
PollConstraints(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Node *node = AG_PTR(2);
	SK_Constraint *ct;
	AG_TlistItem *it;

	AG_TlistBegin(tl);
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		char ctName[64];
		char name1[SK_NODE_NAME_MAX];
		char name2[SK_NODE_NAME_MAX];
		
		if (node != NULL) {
			if (node != ct->n1 && node != ct->n2)
				continue;
		}

		switch (ct->type) {
		case SK_DISTANCE:
			snprintf(ctName, sizeof(ctName),
			    "%s(%.03f %s)", _("Distance"),
			    ct->ct_distance, sk->uLen->abbr);
			break;
		case SK_ANGLE:
			snprintf(ctName, sizeof(ctName),
			    "%s(%.02f\xc2\xb0)", _("Angle"),
			    SG_Degrees(ct->ct_angle));
			break;
		default:
			strlcpy(ctName, _(skConstraintNames[ct->type]),
			    sizeof(ctName));
			break;
		}
		it = AG_TlistAdd(tl, NULL, "%s: %s,%s", ctName,
		    SK_NodeNameCopy(ct->n1, name1, sizeof(name1)),
		    SK_NodeNameCopy(ct->n2, name2, sizeof(name2)));
		it->p1 = ct;
	}
	AG_TlistEnd(tl);
}

static void
UpdateConstraint(AG_Event *event)
{
	SK_View *skv = AG_PTR(1);
	SK_Constraint *ct = AG_PTR(2);

	SK_Update(skv->sk);
}

static void
ConstraintEdit(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Pane *hp = AG_PTR(1);
	AG_Pane *vp = AG_PTR(2);
	SK_View *skv = AG_PTR(3);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	AG_Window *pWin = AG_WidgetParentWindow(skv);
	SK_Constraint *ct = it->p1;
	AG_SizeReq rDiv;
	AG_Numerical *num;

	SK_ViewCloseEditPane(skv);
	skv->editPane = (AG_Widget *)AG_BoxNew(vp->div[1], AG_BOX_VERT,
	                                       AG_BOX_EXPAND);
	AG_LabelNewStatic(skv->editPane, AG_LABEL_HFILL,
	    _("Type: %s"), skConstraintNames[ct->uType]);

	switch (ct->uType) {
	case SK_DISTANCE:
		num = AG_NumericalNew(skv->editPane, AG_NUMERICAL_HFILL, "m",
		    _("Distance: "));
		AG_NumericalSetIncrement(num, 0.1);
		AG_NumericalSetMin(num, 0.0);
		SG_WidgetBindReal(num, "value", &ct->ct_distance);
		AG_WidgetSetFocusable(num, 0);
		AG_SetEvent(num, "numerical-changed",
		    UpdateConstraint, "%p,%p", skv, ct);
		break;
	case SK_ANGLE:
		num = AG_NumericalNew(skv->editPane, AG_NUMERICAL_HFILL, NULL,
		    _("Angle: "));
		AG_NumericalSetIncrement(num, 1.0);
		SG_WidgetBindReal(num, "value", &ct->ct_angle);
		AG_WidgetSetFocusable(num, 0);
		AG_SetEvent(num, "numerical-changed",
		    UpdateConstraint, "%p,%p", skv, ct);
		break;
	default:
		break;
	}

	AG_WidgetSizeReq(vp->div[1], &rDiv);
	AG_PaneSetDivisionMin(vp, 1, -1, rDiv.h + vp->wDiv);
	AG_PaneSetDivisionMin(hp, 0, rDiv.w + vp->wDiv, -1);
	AG_PaneMoveDivider(vp, WIDGET(vp)->h - rDiv.h);
	AG_WindowUpdate(pWin);
}

static void
ConstraintDelete(AG_Event *event)
{
	SK_Constraint *ct = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	SK *sk = skv->sk;

	SK_ViewCloseEditPane(skv);

	SK_NodeDelConstraint(ct->n1, ct);
	SK_NodeDelConstraint(ct->n2, ct);
	SK_DelConstraint(&sk->ctGraph, ct);
	SK_Update(sk);
}

static void
ConstraintMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SK_Constraint *ct = AG_TlistSelectedItemPtr(tl);
	SK *sk = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	AG_PopupMenu *pm;

	pm = AG_PopupNew(skv);
	AG_MenuAction(pm->item, _("Delete constraint"), TRASH_ICON,
	    ConstraintDelete, "%p,%p", ct, skv);
	AG_PopupShow(pm);
}

static void
PollInsns(AG_Event *event)
{
	char name1[SK_NODE_NAME_MAX];
	char name2[SK_NODE_NAME_MAX];
	char name3[SK_NODE_NAME_MAX];
	AG_Table *tbl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Insn *si;

	AG_TableBegin(tbl);
	TAILQ_FOREACH(si, &sk->insns, insns) {
		switch (si->type) {
		case SK_COMPOSE_PAIR:
			AG_TableAddRow(tbl, "%s:%s:%s:%s", "COMPOSE_PAIR",
			    SK_NodeNameCopy(si->n[0],name1,sizeof(name1)),
			    SK_NodeNameCopy(si->n[1],name2,sizeof(name2)),
			    "");
			break;
		case SK_COMPOSE_RING:
			AG_TableAddRow(tbl, "%s:%s:%s:%s", "COMPOSE_RING",
			    SK_NodeNameCopy(si->n[0],name1,sizeof(name1)),
			    SK_NodeNameCopy(si->n[1],name2,sizeof(name2)),
			    SK_NodeNameCopy(si->n[2],name3,sizeof(name3)));
			break;
		}
	}
	AG_TableEnd(tbl);
}

static void
ExecSelectedInsns(AG_Event *event)
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
ExecInsns(AG_Event *event)
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
DrawStatus(AG_Event *event)
{
	SK_View *skv = AG_SELF();
	SDL_Surface *lbl;
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

	lbl = AG_TextRender(skv->sk->statusText);
	x = WIDGET(skv)->w - lbl->w;
	y = WIDGET(skv)->h - lbl->h;
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }

	AG_WidgetPushClipRect(skv, 0, 0, WIDGET(skv)->w, WIDGET(skv)->h);
	AG_WidgetBlit(skv, lbl, x, y);
	AG_WidgetPopClipRect(skv);
	
	SDL_FreeSurface(lbl);
	AG_PopTextState();
}

void *
SK_Edit(void *p)
{
	extern SK_ToolOps *skToolkit[];
	extern Uint skToolkitCount;
	SK *sk = p;
	AG_Window *win;
	SK_View *skv;
	AG_Menu *menu;
	AG_MenuItem *pitem;
	AG_Pane *hp;
	int i, dx;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s", OBJECT(sk)->name);
	AG_WindowSetPaddingTop(win, 0);
	AG_WindowSetSpacing(win, 0);

	skv = Malloc(sizeof(SK_View), M_OBJECT);
	SK_ViewInit(skv, sk, SK_VIEW_EXPAND);
	SK_ViewPostDrawFn(skv, DrawStatus, NULL);
	
	for (i = 0; i < skToolkitCount; i++)
		SK_ViewRegTool(skv, skToolkit[i], NULL);

	menu = AG_MenuNew(win, AG_MENU_HFILL);
	pitem = AG_MenuAddItem(menu, _("File"));
	{
		AG_MenuAction(pitem, _("Import from vector image..."), -1,
		    ImportSketchDlg, "%p,%p", sk, win);
		AG_MenuSeparator(pitem);
		AG_MenuActionKb(pitem, _("Close sketch"), -1,
		    SDLK_w, KMOD_CTRL,
		    AG_WindowCloseGenEv, "%p", win);
	}
	
	pitem = AG_MenuAddItem(menu, _("Edit"));
	{
		/* TODO */
		AG_MenuAction(pitem, _("Undo"), -1, NULL, "%p", NULL);
		AG_MenuAction(pitem, _("Redo"), -1, NULL, "%p", NULL);
	}
	pitem = AG_MenuAddItem(menu, _("View"));
	{
		AG_MenuAction(pitem, _("New view..."), -1,
		    CreateNewView, "%p,%p", win, sk);
		AG_MenuAction(pitem, _("Constraint graphs..."), -1,
		    ViewConstraintGraphs, "%p,%p", win, sk);
	}
	
	hp = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Tlist *tl;
		AG_Pane *vp;
		AG_MPane *mp;
		AG_Table *tbl;

		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT,
		    AG_PANE_EXPAND|AG_PANE_DIV1FILL);
//		AG_PaneSetDivisionMin(vp, 0, 0, 0);
//		AG_PaneSetDivisionMin(vp, 1, 0, 0);
		nb = AG_NotebookNew(vp->div[0], AG_NOTEBOOK_EXPAND);
		mp = AG_MPaneNew(hp->div[1], AG_MPANE1, AG_MPANE_EXPAND);
		AG_ObjectAttach(mp->panes[0], skv);
	
		ntab = AG_NotebookAddTab(nb, _("Tools"), AG_BOX_VERT);
		{
			AG_Tlist *tl;
			SK_Tool *tool;

			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);
			AG_WidgetSetFocusable(tl, 0);
			TAILQ_FOREACH(tool, &skv->tools, tools) {
				AG_TlistAddPtr(tl, AGICON(tool->ops->icon),
				    _(tool->ops->name), tool);
			}

			AG_SetEvent(tl, "tlist-selected", SelectTool, "%p",
			    skv);
		}
		ntab = AG_NotebookAddTab(nb, _("Nodes"), AG_BOX_VERT);
		{
			tl = AG_TlistNewPolled(ntab,
			    AG_TLIST_TREE|AG_TLIST_EXPAND|AG_TLIST_MULTI|
			    AG_TLIST_NOSELSTATE,
			    PollNodes, "%p", sk);
			AG_TlistSetDblClickFn(tl,
			    NodeEdit, "%p,%p,%p", hp, vp, skv);
			AG_TlistSetPopupFn(tl,
			    NodeMenu, "%p,%p", sk, skv);
			AG_TlistSetChangedFn(tl,
			    NodeSelect, NULL);

			AG_TlistPrescale(tl, _("<Polygon00>"), 4);
			AG_WidgetSetFocusable(tl, 0);
		}
		ntab = AG_NotebookAddTab(nb, _("Constraints"), AG_BOX_VERT);
		{
			tl = AG_TlistNewPolled(ntab,
			    AG_TLIST_TREE|AG_TLIST_EXPAND,
			    PollConstraints, "%p,%p", sk, NULL);
			AG_TlistSetDblClickFn(tl,
			    ConstraintEdit, "%p,%p,%p", hp, vp, skv);
			AG_TlistSetPopupFn(tl,
			    ConstraintMenu, "%p,%p", sk, skv);

			AG_TlistPrescale(tl, _("<Polygon00>"), 4);
			AG_WidgetSetFocusable(tl, 0);
		}
		ntab = AG_NotebookAddTab(nb, _("Instructions"), AG_BOX_VERT);
		{
			AG_HBox *hBox;

			tbl = AG_TableNewPolled(ntab,
			    AG_TABLE_MULTI|AG_TABLE_EXPAND,
			    PollInsns, "%p,%p", sk, NULL);
			AG_TableAddCol(tbl, _("Type"), "<COMPOSE_PAIR>", NULL);
			AG_TableAddCol(tbl, "n1", "<Circle0>", NULL);
			AG_TableAddCol(tbl, "n2", "<Circle0>", NULL);
			AG_TableAddCol(tbl, "n3", NULL, NULL);
			AG_WidgetSetFocusable(tbl, 0);
			
			hBox = AG_HBoxNew(ntab, AG_HBOX_HFILL|
			                        AG_HBOX_HOMOGENOUS);
			{
				AG_ButtonNewFn(hBox, 0,
				    _("Execute program"),
				    ExecInsns, "%p,%p", sk, tbl);
				AG_ButtonNewFn(hBox, 0,
				    _("Execute selected"),
				    ExecSelectedInsns, "%p,%p", sk, tbl);
			}
		}
	}
	
	AG_WindowSetGeometry(win, agView->w/6, agView->h/6,
	                     2*agView->w/3, 2*agView->h/3);
	AG_WidgetFocus(skv);
	return (win);
}

static void
SK_NodeEditGeneric(SK_Node *node, AG_Widget *box, SK_View *skv)
{
	AG_Tlist *tl;
	AG_Label *lbl;
	AG_Spinbutton *sb;

	AG_SeparatorNewHoriz(box);

	sb = AG_SpinbuttonNew(box, 0, _("Name: "));
	AG_WidgetBindUint32(sb, "value", &node->name);

//	lbl = AG_LabelNewPolledMT(box, AG_LABEL_HFILL, &skv->sk->lock,
//	    "Flags: <%[flags]>", &node->flags);
//	AG_LabelFlag(lbl, 0, "SELECTED", SK_NODE_SELECTED);

	AG_LabelNewStaticString(box, 0, _("Geometric constraints: "));
	tl = AG_TlistNewPolled(box, AG_TLIST_POLL|AG_TLIST_TREE|AG_TLIST_EXPAND,
	    PollConstraints, "%p,%p", SKNODE(node)->sk, node);
	AG_WidgetSetFocusable(tl, 0);
}

#endif /* EDITION */
#endif /* HAVE_OPENGL */
