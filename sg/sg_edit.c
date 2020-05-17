/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/sg/sg_load_ply.h>

#include <string.h>
#include <math.h>

static void
FindSGNodes(AG_Tlist *_Nonnull tl, SG_Node *_Nonnull node, int depth)
{
	AG_TlistItem *it;
	SG_Node *chld;

	it = AG_TlistAddS(tl, sgIconNode.s, AGOBJECT(node)->name);
	it->cat = "node";
	it->depth = depth;
	it->p1 = node;
	it->selected = (node->flags & SG_NODE_SELECTED);

	if (!TAILQ_EMPTY(&AGOBJECT(node)->children) ||
	    AG_OfClass(node, "SG_Node:SG_Object:*")) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		OBJECT_FOREACH_CLASS(chld, node, sg_node, "SG_Node:*") {
			FindSGNodes(tl, chld, depth+1);
		}
		AG_PostEvent(node, "edit-list-poll", "%p,%i", tl, depth);
	}
}

void
SG_GUI_PollNodes(AG_Event *event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG *sg = SG_PTR(1);

	AG_TlistClear(tl);
	AG_ObjectLock(sg);
	FindSGNodes(tl, sg->root, 0);
	AG_ObjectUnlock(sg);
	AG_TlistRestore(tl);
}

static void
SelectNode(AG_Event *_Nonnull event)
{
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(1);
	const int state = AG_INT(2);

	if (strcmp(it->cat, "node") == 0) {
		SG_Node *node = it->p1;

		AG_ObjectLock(node);
		AG_SETFLAGS(node->flags, SG_NODE_SELECTED, state);
		AG_ObjectUnlock(node);
	} else if (strcmp(it->cat, "facet") == 0) {
		SG_Facet *f = it->p1;

		AG_ObjectLock(f->obj);
		AG_SETFLAGS(f->flags, SG_FACET_SELECTED, state);
		AG_ObjectUnlock(f->obj);
	}
}

void
SG_GUI_EditNode(SG_Node *node, AG_Widget *editArea, SG_View *sv)
{
	AG_Widget *wEdit;
	
	sv->editNode = node;

	if (sv->editArea != NULL) {
		AG_ObjectDetach(sv->editArea);
		sv->editArea = NULL;
	}
	sv->editArea = (AG_Widget *)AG_BoxNew(editArea,
	    AG_BOX_VERT, AG_BOX_EXPAND);

	if (NODE_OPS(node)->edit != NULL) {
		wEdit = NODE_OPS(node)->edit(node, sv);
		if (wEdit != NULL && AG_OfClass(wEdit, "AG_Widget:*"))
			AG_ObjectAttach(sv->editArea, wEdit);
	}

	if (sv->flags & SG_VIEW_EDIT_STATUS) {
		M_Vector3 v;

		v = SG_NodePos(node);
		AG_Snprintf(sv->editStatus, sizeof(sv->editStatus),
		    _("Editing %s (%f,%f,%f)"),
		    OBJECT(node)->name, v.x, v.y, v.z);
	}
	AG_WidgetUpdate(sv);
}

static void
ListLibraryItems(AG_Tlist *_Nonnull tl, const char *_Nonnull pat, int depth,
    AG_ObjectClass *_Nonnull cl)
{
	AG_ObjectClass *clSub;
	AG_TlistItem *it;
	
	if (AG_ClassIsNamed(cl, pat)) {
		it = AG_TlistAddPtr(tl, sgIconNode.s, cl->name, cl);
		it->depth = depth;
	}
	TAILQ_FOREACH(clSub, &cl->pvt.sub, pvt.subclasses)
		ListLibraryItems(tl, pat, depth+1, clSub);
}

SG_Node *
SG_GUI_CreateNode(SG *sg, AG_ObjectClass *cl)
{
	char name[AG_OBJECT_NAME_MAX+30];
	SG_Node *node;
	int num = 0;

	AG_ObjectLock(sg);
tryname:
	AG_Snprintf(name, sizeof(name), "%s%i", cl->name, num++);
	if (SG_FindNode(sg, name) != NULL)
		goto tryname;

	node = Malloc(cl->size);
	AG_ObjectInit(node, cl);
	AG_ObjectSetNameS(node, name);
	AG_ObjectAttach(sg->root, node);
	
	AG_ObjectUnlock(sg);

	return (node);
}

void
SG_GUI_DeleteNode(SG_Node *node, SG_View *sv)
{
	if (node == sv->sg->root) {
		Strlcpy(sv->editStatus, _("Cannot delete root"),
		    sizeof(sv->editStatus));
		return;
	}
	if (node == SGNODE(sv->cam) || node == SGNODE(sv->camTrans)) {
		Strlcpy(sv->editStatus, _("Cannot delete active camera"),
		    sizeof(sv->editStatus));
		return;
	}
	if (node == sv->editNode) {
		if (sv->editArea != NULL) {
			AG_ObjectDetach(sv->editArea);
			sv->editArea = NULL;
		}
		sv->editNode = NULL;
	}
	if (sv->flags & SG_VIEW_EDIT_STATUS) {
		AG_Snprintf(sv->editStatus, sizeof(sv->editStatus),
		    _("Deleted %s"),
		    OBJECT(node)->name);
	}
	AG_ObjectDetach(node);
	AG_WidgetUpdate(sv);
}

static void
DeleteNode(AG_Event *_Nonnull event)
{
	SG_Node *node = SG_NODE_PTR(1);
	SG_View *sv = SG_VIEW_PTR(2);

	SG_GUI_DeleteNode(node, sv);
}

static void
NodePopupFindCommon(SG_Node *_Nonnull node,
    const SG_NodeClass *_Nonnull *_Nullable commonCls,
    int *_Nonnull commonFlag, int *_Nonnull nSel)
{
	SG *sg = node->sg;
	SG_Node *chld;

	if (!AG_OfClass(node, "SG_Node:*") ||
	    (node->flags & SG_NODE_SELECTED) == 0) {
		return;
	}
	if ((*commonCls) != NULL && (*commonCls) != NODE_OPS(node)) {
		*commonFlag = 0;
	}
	(*commonCls) = NODE_OPS(node);
	(*nSel)++;

	OBJECT_FOREACH_CHILD(chld, sg, sg_node)
		NodePopupFindCommon(chld, commonCls, commonFlag, nSel);
}

void
SG_GUI_NodePopupMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG *sg = SG_PTR(1);
	SG_View *sv = SG_VIEW_PTR(2);
	AG_TlistItem *tlItem = AG_TlistSelectedItem(tl);
	SG_Node *node;
	AG_PopupMenu *pm;
	int commonFlag = 1, nSel = 0, x, y;
	const SG_NodeClass *commonCls = NULL;

	if (strcmp(tlItem->cat, "node") != 0) {
		return;
	}
	if ((node = tlItem->p1) == NULL || node == sg->root) {
		return;
	}
	AG_ObjectLock(sg);
	NodePopupFindCommon(sg->root, &commonCls, &commonFlag, &nSel);
	AG_ObjectUnlock(sg);
		   
	x = WIDGET(tl)->drv->mouse->x - WIDGET(tl)->rView.x1;
	y = WIDGET(tl)->drv->mouse->y - WIDGET(tl)->rView.y1;

	if (sv->pmNode != NULL) {
		AG_PopupShowAt(sv->pmView, x, y);
		return;
	}
	if ((sv->pmNode = pm = AG_PopupNew(tl)) == NULL) {
		return;
	}
	AG_MenuSection(pm->root, "[Node: %s]", AGOBJECT(node)->name);
	AG_MenuSeparator(pm->root);
	if (NODE_OPS(node)->menuInstance != NULL) {
		NODE_OPS(node)->menuInstance(node, pm->root, sv);
		AG_MenuSeparator(pm->root);
	}
	AG_MenuAction(pm->root, _("Delete node"), agIconTrash.s,
	    DeleteNode, "%p,%p", node, sv);

	if (nSel > 1 && commonFlag && NODE_OPS(node)->menuClass != NULL) {
		AG_MenuSeparator(pm->root);
		AG_MenuSection(pm->root, _("[Class: %s]"),
		    ((AG_ObjectClass *)commonCls)->name);
		NODE_OPS(node)->menuClass(sg, pm->root, sv);
	}

	AG_PopupShowAt(pm, x, y);
}

static void
EditNode(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Pane *editPane = AG_PANE_PTR(1);
	AG_Widget *editArea = AG_WIDGET_PTR(2);
	SG_View *sv = SG_VIEW_PTR(3);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	SG_Node *node = it->p1;

	if (strcmp(it->cat, "node") == 0) {
		SG_GUI_EditNode(node, editArea, sv);
	}
	/* TODO: facet, etc. */

	if (editPane != NULL) {
		AG_SizeReq rDiv;
		AG_WidgetSizeReq(editArea, &rDiv);
		rDiv.h += 20;
		AG_PaneMoveDivider(editPane,
		    WIDGET(editPane)->h - rDiv.h);
	}
}

static void
CreateNode(AG_Event *_Nonnull event)
{
	SG *sg = SG_PTR(1);
	AG_Pane *editPane = AG_PANE_PTR(2);
	AG_Widget *editArea = AG_WIDGET_PTR(3);
	SG_View *sv = SG_VIEW_PTR(4);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(5);
	AG_ObjectClass *cl = ti->p1;
	SG_Node *node;

	if ((node = SG_GUI_CreateNode(sg, cl)) != NULL) {
		AG_ObjectLock(node);
		node->flags |= SG_NODE_SELECTED;
		AG_ObjectUnlock(node);
		SG_GUI_EditNode(node, editArea, sv);
		if (editPane != NULL) {
			AG_SizeReq rDiv;
			AG_WidgetSizeReq(editArea, &rDiv);
			rDiv.h += 20;
			AG_PaneMoveDivider(editPane,
			    WIDGET(editPane)->h - rDiv.h);
		}
	}
}

void *
SG_Edit(void *p)
{
	SG *sg = p;
	AG_Window *win;
	SG_View *sv;
	AG_Menu *menu;
	AG_MenuItem *m;
	AG_Pane *hp;

	if (sg->def.cam == NULL) {
		AG_SetErrorS("No camera");
		return (NULL);
	}

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(sg)->name);
	AG_SetStyle(win, "spacing", "0");
	AG_SetStyleF(win, "padding", "0 %d %d %d",
	    WIDGET(win)->paddingRight,
	    WIDGET(win)->paddingBottom,
	    WIDGET(win)->paddingLeft);

	/* Main SG_View(3), later attached to Pane */
	sv = SG_ViewNew(NULL, sg, SG_VIEW_EXPAND | SG_VIEW_EDIT);
	sv->cam = sg->def.cam;

	/* Scene menu */
	menu = AG_MenuNew(win, AG_MENU_HFILL);
	{
		m = AG_MenuNode(menu->root, _("File"), NULL);
		SG_FileMenu(m, sg, win);
		m = AG_MenuNode(menu->root, _("Edit"), NULL);
		SG_EditMenu(m, sg, win);
		m = AG_MenuNode(menu->root, _("View"), NULL);
		SG_ViewMenu(m, sg, win, sv);
	}

	/* Edition area */
	hp = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Tlist *tl;
		AG_Pane *vp;

		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT, AG_PANE_EXPAND |
		                                          AG_PANE_DIV1FILL);
		nb = AG_NotebookNew(vp->div[0], AG_NOTEBOOK_EXPAND);
		ntab = AG_NotebookAdd(nb, _("Create Object"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND);
			AG_TlistSizeHint(tl, "<Isocahedron>", 2);
			AG_SetEvent(tl, "tlist-dblclick",
			    CreateNode, "%p%p%p%p", sg, vp, vp->div[1], sv);
			ListLibraryItems(tl, "SG_Node:*", 0, &agObjectClass);
		}
		ntab = AG_NotebookAdd(nb, _("Nodes"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND |
			                       AG_TLIST_MULTI);
			AG_TlistSizeHint(tl, "<Isocahedron>", 2);
			AG_TlistSetPopupFn(tl, SG_GUI_NodePopupMenu, "%p,%p", sg, sv);
			AG_SetEvent(tl, "tlist-poll", SG_GUI_PollNodes, "%p", sg);
			AG_SetEvent(tl, "tlist-changed", SelectNode, NULL);
		}

		AG_ObjectAttach(hp->div[1], sv);

		AG_SetEvent(tl, "tlist-dblclick",
		    EditNode, "%p,%p,%p", vp, vp->div[1], sv);
	}

	AG_PaneMoveDividerPct(hp, 30);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 50);
/*	AG_WidgetFocus(sv); */
	return (win);
}
