/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Graphical user interface for scene graph edition.
 */

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

#include "sg.h"
#include "sg_matview.h"
#include "sg_gui.h"

#include "import/ply.h"

#include <string.h>
#include <math.h>

static void
FindSGNodes(AG_Tlist *tl, SG_Node *node, int depth)
{
	AG_TlistItem *it;
	SG_Node *cnode;

	it = AG_TlistAdd(tl, AGICON(EDA_NODE_ICON), "%s", node->name);
	it->depth = depth;
	it->p1 = node;
	it->selected = (node->flags & SG_NODE_SELECTED);

	if (!TAILQ_EMPTY(&node->cnodes)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(cnode, &node->cnodes, sgnodes)
			FindSGNodes(tl, cnode, depth+1);
	}
}

static void
PollNodes(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SG *sg = AG_PTR(1);

	AG_TlistClear(tl);
	FindSGNodes(tl, sg->root, 0);
	AG_TlistRestore(tl);
}

static void
SelectNode(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_TlistItem *it = AG_PTR(1);
	int state = AG_INT(2);
	SG_Node *node = it->p1;
	
	if (state) {
		node->flags |= SG_NODE_SELECTED;
	} else {
		node->flags &= ~(SG_NODE_SELECTED);
	}
}

static void
EditNode(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Pane *hp = AG_PTR(1);
	AG_Pane *vp = AG_PTR(2);
	SG_View *sgv = AG_PTR(3);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	AG_Window *pWin = AG_WidgetParentWindow(sgv);
	SG_Node *node = it->p1;
	AG_SizeReq rDiv;

	if (sgv->editPane != NULL) {
		AG_ObjectDetach(sgv->editPane);
	}
	if (node->ops->edit == NULL) {
		return;
	}
	sgv->editPane = (AG_Widget *)AG_BoxNew(vp->div[1], AG_BOX_VERT,
	    AG_BOX_EXPAND);
	node->ops->edit(node, sgv->editPane, sgv);

	AG_WidgetSizeReq(vp->div[1], &rDiv);
	AG_PaneSetDivisionMin(vp, 1, -1, rDiv.h+vp->wDiv);
	AG_PaneSetDivisionMin(hp, 0, rDiv.w+vp->wDiv, -1);
	AG_PaneMoveDivider(vp, WIDGET(vp)->h - rDiv.h);

	AG_WindowUpdate(pWin);
}

static int
CompareClass(const char *pat, const char *cname)
{
	const char *c;

	if (pat[0] == '*' && pat[1] == '\0') {
		return (0);
	}
	for (c = &pat[0]; *c != '\0'; c++) {
		if (c[0] == ':' && c[1] == '*' && c[2] == '\0') {
			if (c == &pat[0]) {
				return (0);
			}
			if (strncmp(cname, pat, c - &pat[0] + 1) == 0) {
				return (0);
			}
		}
	}
	return (1);
}

static void
ListLibraryItems(AG_Tlist *tl, const char *cname, int depth)
{
	char subname[SG_CLASS_MAX];
	const char *c;
	AG_TlistItem *it;
	int i, j;

	for (i = 0; i < sgElementsCnt; i++) {
		SG_NodeOps *ops = sgElements[i];
		
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
InsertInstance(AG_Event *event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_Tlist *tl = AG_SELF();
	SG *sg = AG_PTR(1);
	AG_TlistItem *it;
	SG_NodeOps *ops;
	SG_Node *node;
	int num = 0;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		return;
	}
	ops = it->p1;

tryname:
	snprintf(name, sizeof(name), "%s%i", ops->name, num++);
	if (SG_FindNode(sg, name) != NULL)
		goto tryname;

	node = Malloc(ops->size, M_SG);
	ops->init(node, name);
	SG_NodeAttach(sg->root, node);
}

static void
ImportMeshFromPLY(AG_Event *event)
{
	char name[SG_NODE_NAME_MAX];
	SG *sg = AG_PTR(1);
	char *path = AG_STRING(2);
	SG_Object *so;
	int num = 0;

tryname:
	snprintf(name, sizeof(name), "Mesh%i", num++);
	if (SG_FindNode(sg, name) != NULL) {
		goto tryname;
	}
	so = SG_ObjectNew(sg->root, name);
	if (SG_ObjectLoadPLY(so, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Loading %s: %s", path, AG_GetError());
		/* XXX */
	}
}

static void
ImportMeshDlg(AG_Event *event)
{
	SG *sg = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Window *win;
	AG_FileDlg *dlg;

	win = AG_WindowNew(0);
	dlg = AG_FileDlgNew(win, AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|
	                         AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(dlg, _("Stanford .PLY Format"), "*.ply",
	    ImportMeshFromPLY, "%p", sg);
	
	AG_WindowShow(win);
}

static void
NodeSelect(AG_Event *event)
{
	SG_View *sgv = AG_PTR(1);
	SG_Node *node = AG_PTR(2);

}

static void
PollChildNodes(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();
	SG_Node *node = AG_PTR(1);
	SG_Node *cnode;

	AG_TableBegin(tbl);
	TAILQ_FOREACH(cnode, &node->cnodes, sgnodes) {
		AG_TableAddRow(tbl, "%s:%s", cnode->name, cnode->ops->name);
	}
	AG_TableEnd(tbl);
}

static void
NodeInfo(AG_Event *event)
{
	SG_View *sgv = AG_PTR(1);
	SG_Node *node = AG_PTR(2);
	AG_Window *win;
	AG_Matview *mv;
	AG_Table *tbl;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	SG_Vector pos = SG_NodePos(node);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Node information: %s"), node->name);

	AG_LabelNewStatic(win, 0, _("Name: %s"), node->name);
	AG_LabelNewStatic(win, 0, _("Parent Node: %s"),
	    node->pNode != NULL ? node->pNode->name : "NULL");
	AG_LabelNewStatic(win, 0, _("Class: %s"), node->ops->name);
	AG_LabelNewStatic(win, 0, _("Size: %ub"), (Uint)node->ops->size);
	AG_LabelNewStatic(win, 0, _("Flags: 0x%x"), node->flags);
	AG_LabelNewStatic(win, 0, _("Position: %f,%f,%f"),
	    pos.x, pos.y, pos.z);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAddTab(nb, _("T-matrix"), AG_BOX_VERT);
	{
		static SG_Matrix Tp;

		mv = AG_MatviewNew(ntab, &node->T, 0);
		AG_MatviewSetNumericalFmt(mv, "%.06f");
		AG_TextSize("-0.000000", &mv->ent_w, &mv->ent_h);
	}
	ntab = AG_NotebookAddTab(nb, _("Children"), AG_BOX_VERT);
	{
		tbl = AG_TableNewPolled(ntab, AG_TABLE_EXPAND,
		    PollChildNodes, "%p", node);
		AG_TableAddCol(tbl, _("Name"), "<XXXXXXXXXXXXXX>", NULL);
		AG_TableAddCol(tbl, _("Class"), NULL, NULL);
	}
	AG_WindowShow(win);
}

static void
NodeDelete(AG_Event *event)
{
	SG_Node *node = AG_PTR(1);
}

static void
ViewNew(AG_Event *event)
{
	char name[SG_NODE_NAME_MAX];
	SG *sg = AG_PTR(1);
	SG_View *sv;
	AG_Window *win;
	int num = 0;
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s", OBJECT(sg)->name);

	sv = SG_ViewNew(win, sg, SG_VIEW_EXPAND);
tryname:
	snprintf(name, sizeof(name), "cView%d", num++);
	if (SG_FindNode(sg, name) != NULL) {
		goto tryname;
	}
	sv->cam = SG_CameraNew(sg->root, name);
	SG_Translate3(sv->cam, 0.0, 0.0, -10.0);

	AG_WindowShow(win);
}

static void
NodePopupMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SG_Node *node = AG_TlistSelectedItemPtr(tl);
	SG *sg = AG_PTR(1);
	SG_View *sgv = AG_PTR(2);
	AG_PopupMenu *pm;
	SG_Node *nOther;
	int commonClass = 1;
	const SG_NodeOps *commonOps = NULL;
	Uint nsel = 0;

	if (node == NULL) {
		return;
	}
	SG_FOREACH_NODE(nOther, sg, sg_node) {
		if ((sg->flags & SG_NODE_SELECTED) == 0) {
			continue;
		}
		if (commonOps != NULL && commonOps != nOther->ops) {
			commonClass = 0;
		}
		commonOps = nOther->ops;
		nsel++;
	}
	pm = AG_PopupNew(sgv);
	AG_MenuSection(pm->item, "[Node: %s]", node->name);
	AG_MenuSeparator(pm->item);
	if (node->ops->menuInstance != NULL) {
		node->ops->menuInstance(node, pm->item, sgv);
		AG_MenuSeparator(pm->item);
	}
	AG_MenuAction(pm->item, _("Node information..."), OBJEDIT_ICON,
	    NodeInfo, "%p,%p", sgv, node);
	AG_MenuAction(pm->item, _("Delete node"), TRASH_ICON,
	    NodeDelete, "%p", node);

	if (nsel > 1) {
		if (commonClass && node->ops->menuClass != NULL) {
			AG_MenuSeparator(pm->item);
			AG_MenuSection(pm->item, _("[Class: %s]"),
			    commonOps->name);
			node->ops->menuClass(sg, pm->item, sgv);
		}
#if 0
		AG_MenuSeparator(pm->item);
		AG_MenuSection(pm->item, _("[All selections]"));
		AG_MenuAction(pm->item, _("    Delete selected nodes"),
		    TRASH_ICON, DeleteSelectedNoes, "%p", sg);
#endif
	}
	AG_PopupShow(pm);
}

void *
SG_Edit(void *p)
{
	SG *sg = p;
	AG_Window *win;
	SG_View *sv;
	AG_Menu *menu;
	AG_MenuItem *pitem;
	AG_Pane *hp;
	int i, dx;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s", OBJECT(sg)->name);
	AG_WindowSetPaddingTop(win, 0);
	AG_WindowSetSpacing(win, 0);

	sv = Malloc(sizeof(SG_View), M_OBJECT);
	SG_ViewInit(sv, sg, SG_VIEW_EXPAND);
	
	menu = AG_MenuNew(win, AG_MENU_HFILL);
	pitem = AG_MenuAddItem(menu, _("File"));
	{
		AG_MenuAction(pitem, _("Import mesh..."), CLOSE_ICON,
		    ImportMeshDlg, "%p,%p", sg, win);
		AG_MenuSeparator(pitem);
		AG_MenuActionKb(pitem, _("Close scene"), CLOSE_ICON,
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
		    ViewNew, "%p", sg);
//		AG_MenuAction(pitem, _("Switch camera..."), -1,
//		    SwitchCameraDlg, "%p", sv);
	}
	
	hp = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Tlist *tl;
		AG_Pane *vp;
		AG_MPane *mp;

		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT,
		    AG_PANE_EXPAND|AG_PANE_DIV1FILL);
		nb = AG_NotebookNew(vp->div[0], AG_NOTEBOOK_EXPAND);
		ntab = AG_NotebookAddTab(nb, _("Library"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_TREE|AG_TLIST_EXPAND);
			AG_TlistSizeHint(tl, "<Isocahedron>", 2);
			AG_SetEvent(tl, "tlist-dblclick", InsertInstance, "%p",
			    sg);
			WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
			ListLibraryItems(tl, "*", 0);
		}
		ntab = AG_NotebookAddTab(nb, _("Nodes"), AG_BOX_VERT);
		{
			AG_MenuItem *popup;

			tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_TREE|
			                       AG_TLIST_EXPAND|AG_TLIST_MULTI);
			AG_TlistSizeHint(tl, "<Isocahedron>", 2);
			AG_TlistSetPopupFn(tl, NodePopupMenu, "%p,%p", sg, sv);
			AG_SetEvent(tl, "tlist-poll", PollNodes, "%p", sg);
			AG_SetEvent(tl, "tlist-changed", SelectNode, NULL);
			WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
		}

		mp = AG_MPaneNew(hp->div[1], AG_MPANE1, AG_MPANE_EXPAND);
		AG_ObjectAttach(mp->panes[0], sv);
		if ((sv->cam = SG_FindNode(sg, "Camera0")) == NULL) {
			AG_TextMsg(AG_MSG_ERROR, "Cannot find Camera0!");
		}
		AG_SetEvent(tl, "tlist-dblclick", EditNode, "%p,%p,%p",
		    hp, vp, sv);
	}
	
	AG_WindowSetGeometry(win, agView->w/6, agView->h/6,
	                     2*agView->w/3, 2*agView->h/3);
	AG_WidgetFocus(sv);
	return (win);
}

#endif /* HAVE_OPENGL */
