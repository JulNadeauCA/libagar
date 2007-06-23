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
 * Graphical interface for SK(3) object edition.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/objmgr.h>

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

#include "sk.h"

#include <string.h>
#include <math.h>

#ifdef EDITION

static void SK_NodeEditGeneric(SK_Node *, AG_Widget *, SK_View *);

extern SK_ToolOps skPointToolOps;
extern SK_ToolOps skLineToolOps;
extern SK_ToolOps skCircleToolOps;

SK_ToolOps *skToolkit[] = {
	&skPointToolOps,
	&skLineToolOps,
	&skCircleToolOps
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
}

static void
CreateNewView(AG_Event *event)
{
	AG_Window *winParent = AG_OBJECT(1,"AG_Widget:AG_Window:*");
	SK *sk = AG_OBJECT(2,"SK");
	SK_View *skv;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s", AGOBJECT(sk)->name);
	skv = SK_ViewNew(win, sk, SK_VIEW_EXPAND|SK_VIEW_FOCUS);
	AG_WindowShow(win);
	AG_WindowAttach(winParent, win);
}

static void
NodePopupMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SK_Node *node = AG_TlistSelectedItemPtr(tl);
	SK *sk = AG_PTR(1);
	SK_View *skv = AG_PTR(2);
	AG_PopupMenu *pm;

	pm = AG_PopupNew(skv);
	AG_MenuAction(pm->item, _("Delete entity"), TRASH_ICON,
	    NodeDelete, "%p", node);
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

	AG_TlistClear(tl);
	FindNodes(tl, (SK_Node *)sk->root, 0);
	AG_TlistRestore(tl);
}

static void
SelectNode(AG_Event *event)
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

	dprintf("select tool %s\n", tool->ops->name);
	SK_ViewSelectTool(skv, tool, NULL);
}

static void
EditNode(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Pane *hp = AG_PTR(1);
	AG_Pane *vp = AG_PTR(2);
	SK_View *skv = AG_PTR(3);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	AG_Window *pWin = AG_WidgetParentWindow(skv);
	SK_Node *node = it->p1;
	int hPane, wPane;

	if (skv->editPane != NULL) {
		AG_ObjectDetach(skv->editPane);
	}
	if (node->ops->edit == NULL) {
		return;
	}
	skv->editPane = (AG_Widget *)AG_BoxNew(vp->div[1], AG_BOX_VERT,
	    AG_BOX_EXPAND);
	node->ops->edit(node, skv->editPane, skv);
	SK_NodeEditGeneric(node, skv->editPane, skv);
	AG_WidgetScale(vp->div[1], -1, -1);
	hPane = AGWIDGET(vp->div[1])->h;
	wPane = AGWIDGET(vp->div[1])->w;
	AG_PaneSetDivisionMin(vp, 1, -1, hPane + vp->dw);
	AG_PaneSetDivisionMin(hp, 0, wPane + vp->dw, -1);
	AG_PaneMoveDivider(vp, AGWIDGET(vp)->h);
	AG_WindowScale(pWin, AGWIDGET(pWin)->w, AGWIDGET(pWin)->h);
	AG_WINDOW_UPDATE(pWin);
}

static void
PollConstraints(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();
	SK *sk = AG_PTR(1);
	SK_Node *node = AG_PTR(2);
	SK_Constraint *cons;

	AG_TableBegin(tbl);
	TAILQ_FOREACH(cons, &sk->constraints, constraints) {
		char name1[SK_NODE_NAME_MAX];
		char name2[SK_NODE_NAME_MAX];

		if (node != NULL) {
			if (node != cons->e1 && node != cons->e2)
				continue;
		}
		AG_TableAddRow(tbl, "%s:%s", skConstraintNames[cons->type],
		    SK_NodeNameCopy(cons->e1, name1, sizeof(name1)),
		    SK_NodeNameCopy(cons->e2, name2, sizeof(name2)));
	}
	AG_TableEnd(tbl);
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
	AG_WindowSetCaption(win, "%s", AGOBJECT(sk)->name);

	skv = Malloc(sizeof(SK_View), M_OBJECT);
	SK_ViewInit(skv, sk, SK_VIEW_EXPAND);
			
	for (i = 0; i < skToolkitCount; i++)
		SK_ViewRegTool(skv, skToolkit[i], NULL);

	menu = AG_MenuNew(win, AG_MENU_HFILL);
	pitem = AG_MenuAddItem(menu, _("File"));
	{
		AG_MenuAction(pitem, _("Import sketch..."), CLOSE_ICON,
		    ImportSketchDlg, "%p,%p", sk, win);
		AG_MenuSeparator(pitem);
		AG_ObjMgrGenericMenu(pitem, sk);
		AG_MenuSeparator(pitem);
		AG_MenuActionKb(pitem, _("Close document"), CLOSE_ICON,
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
		//AG_PaneSetDivisionMin(hp, 0, 0, 0);
		nb = AG_NotebookNew(vp->div[0], AG_NOTEBOOK_EXPAND);
		mp = AG_MPaneNew(hp->div[1], AG_MPANE1, AG_MPANE_EXPAND);
		AG_ObjectAttach(mp->panes[0], skv);
	
		ntab = AG_NotebookAddTab(nb, _("Tools"), AG_BOX_VERT);
		{
			AG_Tlist *tl;
			SK_Tool *tool;

			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);
			TAILQ_FOREACH(tool, &skv->tools, tools) {
				AG_TlistAddPtr(tl, AGICON(tool->ops->icon),
				    _(tool->ops->name), tool);
			}

			AG_SetEvent(tl, "tlist-selected", SelectTool, "%p",
			    skv);
		}
		ntab = AG_NotebookAddTab(nb, _("Nodes"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_TREE|
			                       AG_TLIST_EXPAND|AG_TLIST_MULTI);
			AG_TlistPrescale(tl, "<Polygon>", 4);
			AG_TlistSetPopupFn(tl, NodePopupMenu, "%p,%p", sk, skv);
			AG_TlistSetDblClickFn(tl, EditNode, "%p,%p,%p", hp, vp,
			    skv);
			AG_SetEvent(tl, "tlist-poll", PollNodes, "%p", sk);
			AG_SetEvent(tl, "tlist-changed", SelectNode, NULL);
			AGWIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
		}
		ntab = AG_NotebookAddTab(nb, _("Constraints"), AG_BOX_VERT);
		{
			tbl = AG_TableNewPolled(ntab,
			    AG_TABLE_MULTI|AG_TABLE_EXPAND,
			    PollConstraints, "%p,%p", sk, NULL);
			AG_TableAddCol(tbl, _("Type"), NULL, NULL);
			AG_TableAddCol(tbl, _("Node 1"), "<Circle88>", NULL);
			AG_TableAddCol(tbl, _("Node 2"), "<Circle88>", NULL);
			AGWIDGET(tbl)->flags &= ~(AG_WIDGET_FOCUSABLE);
		}
	}
	
	AG_WindowScale(win, -1, -1);
	AG_WindowSetGeometry(win, agView->w/6, agView->h/6,
	                     2*agView->w/3, 2*agView->h/3);
	AG_WidgetFocus(skv);
	return (win);
}

void
SK_NodeEditGeneric(SK_Node *node, AG_Widget *box, SK_View *skv)
{
	AG_Table *tbl;
	AG_Label *lbl;

	AG_SeparatorNewHoriz(box);

	lbl = AG_LabelNewPolledMT(box, AG_LABEL_HFILL, &skv->sk->lock,
	    "Flags: <%[flags]>", &node->flags);
	AG_LabelFlag(lbl, 0, "SELECTED", SK_NODE_SELECTED);

	AG_LabelNewStaticString(box, 0, _("Geometric constraints: "));
	tbl = AG_TableNewPolled(box, AG_TABLE_MULTI|AG_TABLE_EXPAND,
	    PollConstraints, "%p,%p", SKNODE(node)->sk, node);
	AG_TableAddCol(tbl, _("Type"), NULL, NULL);
	AG_TableAddCol(tbl, _("Node 1"), "<Circle88>", NULL);
	AG_TableAddCol(tbl, _("Node 2"), "<Circle88>", NULL);
	AGWIDGET(tbl)->flags &= ~(AG_WIDGET_FOCUSABLE);
}

#endif /* EDITION */
#endif /* HAVE_OPENGL */
