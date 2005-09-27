/*	$Csoft: merge.c,v 1.7 2005/07/24 06:55:57 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#ifdef MAP

#include <engine/view.h>

#include <engine/widget/hbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/toolbar.h>

#include "map.h"
#include "mapedit.h"

#include <string.h>

const AG_Version merge_ver = {
	"agar merge tool",
	6, 0
};

static void merge_create_brush(int, union evarg *);
static void merge_edit_brush(int, union evarg *);
static void merge_remove_brush(int, union evarg *);
static void merge_interpolate(AG_Map *, AG_Node *,
	                      AG_Nitem *, AG_Map *, AG_Node *,
			      AG_Nitem *);

static TAILQ_HEAD(, ag_object) brushes = TAILQ_HEAD_INITIALIZER(brushes);
static AG_Tlist *brushes_tl;

enum {
	NODE, FILL,
	EDNW, EDNO, EDNE,
	EDWE, EDEA,
	EDSW, EDSO, EDSE
};

static const int
merge_table[9][9] = {
       /* nw    n     ne    w     fill  e     sw    s     se */
	{ EDNW, EDNO, EDNO, EDWE, NODE, FILL, FILL, FILL, FILL }, /* nw */
	{ EDNO, EDNO, EDNO, FILL, NODE, FILL, FILL, FILL, FILL }, /* n */
	{ EDNO, EDNO, NODE, 0   , NODE, EDEA, FILL, FILL, EDEA }, /* ne */
	{ EDWE, FILL, FILL, EDWE, NODE, FILL, EDWE, FILL, FILL }, /* w */
	{ FILL, FILL, FILL, FILL, NODE, FILL, FILL, FILL, FILL }, /* fill */
	{ FILL, FILL, EDEA, FILL, NODE, EDEA, FILL, FILL, EDEA }, /* e */
	{ EDWE, 0,    EDEA, EDWE, NODE, 0,    EDSW, EDSO, EDSO }, /* sw */
	{ FILL, FILL, FILL, FILL, NODE, FILL, EDSO, EDSO, EDSO }, /* s */
	{ EDWE, 0,    EDEA, EDNO, NODE, EDNO, EDSO, EDSO, EDSE }  /* se */
};

static void
merge_init(AG_Maptool *t)
{
	AG_Window *win;
	AG_HBox *hb;
	AG_Button *bu;
	AG_Textbox *tb;

	win = AG_MaptoolWindow(t, "mapedit-tool-merge");

	hb = AG_HBoxNew(win, AG_HBOX_WFILL);
	{
		tb = AG_TextboxNew(hb, _("Name: "));
		AG_SetEvent(tb, "textbox-return", merge_create_brush, "%p", tb);

		bu = AG_ButtonNew(hb, _("Create"));
		AG_ButtonSetPadding(bu, 6);			/* Align */
		AG_ButtonSetFocusable(bu, 0);
		AG_SetEvent(bu, "button-pushed", merge_create_brush, "%p", tb);
	}

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_WFILL);
	{
#if 0
		bu = AG_ButtonNew(hb, _("Load set"));
		AG_SetEvent(bu, "button-pushed", merge_load_brush_set, NULL);
		bu = AG_ButtonNew(hb, _("Save set"));
		AG_SetEvent(bu, "button-pushed", merge_save_brush_set, NULL);
#endif
		bu = AG_ButtonNew(hb, _("Edit"));
		AG_SetEvent(bu, "button-pushed", merge_edit_brush, "%p", win);
		bu = AG_ButtonNew(hb, _("Remove"));
		AG_SetEvent(bu, "button-pushed", merge_remove_brush, NULL);
	}

	brushes_tl = AG_TlistNew(win, AG_TLIST_MULTI);
}

static void
merge_free_brushes(void)
{
	AG_Object *ob, *nob;

	for (ob = TAILQ_FIRST(&brushes);
	     ob != TAILQ_END(&brushes);
	     ob = nob) {
		nob = TAILQ_NEXT(ob, cobjs);
		AG_ObjectDestroy(ob);
		Free(ob, M_OBJECT);
	}
	TAILQ_INIT(&brushes);
}

static void
merge_destroy(AG_Maptool *t)
{
	merge_free_brushes();
}

static void
merge_create_brush(int argc, union evarg *argv)
{
	char brush_name[AG_OBJECT_NAME_MAX];
	char m_name[AG_OBJECT_NAME_MAX];
	AG_Textbox *name_tbox = argv[1].p;
	AG_Map *m;

	AG_TextboxCopyString(name_tbox, brush_name,
	    sizeof(brush_name) - sizeof("brush()"));
	if (brush_name[0] == '\0') {
		AG_TextMsg(AG_MSG_ERROR, _("No brush name was given."));
		return;
	}
	
	snprintf(m_name, sizeof(m_name), "brush(%s)", brush_name);
	if (AG_TlistFindText(brushes_tl, m_name) != NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("A `%s' brush exists."), m_name);
		return;
	}

	m = AG_MapNew(NULL, m_name);
	if (AG_ObjectLoad(m) == -1) {
		extern int agMapDefaultBrushWidth, agMapDefaultBrushHeight;

		if (AG_MapAllocNodes(m, agMapDefaultBrushWidth,
		    agMapDefaultBrushHeight) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "AG_MapAllocNodes: %s",
			    AG_GetError());
			goto fail;
		}
	}

	TAILQ_INSERT_HEAD(&brushes, AGOBJECT(m), cobjs);
	AG_TlistDeselectAll(brushes_tl);
	AG_TlistSelect(brushes_tl, AG_TlistAddPtrHead(brushes_tl, NULL,
	    m_name, m));
	AG_TextboxPrintf(name_tbox, NULL);
	return;
fail:
	AG_ObjectDestroy(m);
	Free(m, M_OBJECT);
}

static void
merge_edit_brush(int argc, union evarg *argv)
{
	AG_TlistItem *it;
	AG_Window *pwin = argv[1].p;

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		AG_Map *brush = it->p1;
		AG_Window *win;
		AG_Toolbar *tbar;

		if (!it->selected)
			continue;

		if ((win = AG_WindowNew(0, "mapedit-tool-merge-%s",
		    AGOBJECT(brush)->name)) == NULL)
			continue;

		tbar = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 1, 0);
		AG_MapviewNew(win, brush, AG_MAPVIEW_EDIT|AG_MAPVIEW_GRID|
		                        MAPVIEW_PROPS, tbar, NULL);
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
merge_remove_brush(int argc, union evarg *argv)
{
	AG_TlistItem *it, *nit;

	for (it = TAILQ_FIRST(&brushes_tl->items);
	     it != TAILQ_END(&brushes_tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if (it->selected) {
			char wname[AG_OBJECT_NAME_MAX];
			AG_Object *brush = it->p1;
			AG_Window *win;

			snprintf(wname, sizeof(wname),
			    "win-mapedit-tool-merge-%s",
			    AGOBJECT(brush)->name);
			if ((win = AG_FindWindow(wname)) != NULL) {
				AG_WindowHide(win);
				AG_ViewDetach(win);
			}

			TAILQ_REMOVE(&brushes, brush, cobjs);
			AG_TlistDel(brushes_tl, it);
			AG_ObjectDestroy(brush);
			Free(brush, M_OBJECT);
		}
	}
}

static void
merge_interpolate(AG_Map *sm, AG_Node *sn, AG_Nitem *sr,
    AG_Map *dm, AG_Node *dn, AG_Nitem *dr)
{
	/* TODO */
}

static int
merge_effect(AG_Maptool *t, AG_Node *n)
{
	AG_Mapview *mv = t->mv;
	AG_Map *m = mv->map;
	AG_TlistItem *it;
	
	/* Avoid circular references. XXX ugly */
	if (strncmp(AGOBJECT(m)->name, "brush(", 6) == 0)
		return (1);
	
	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		AG_Map *sm;
		int sx, sy, dx, dy;

		if (!it->selected)
			continue;

		sm = it->p1;
		for (sy = 0, dy = mv->cy;
		     sy < sm->maph && dy < m->maph;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx;
			     sx < sm->mapw && dx < m->mapw;
			     sx++, dx++) {
				AG_Node *sn = &sm->map[sy][sx];
				AG_Node *dn = &m->map[dy][dx];
				AG_Nitem *sr, *dr;

				TAILQ_FOREACH(sr, &sn->nrefs, nrefs) {
					TAILQ_FOREACH(dr, &dn->nrefs, nrefs) {
						merge_interpolate(sm, sn, sr,
						    m, dn, dr);
					}
				}
			}
		}
	}
	return (1);
}

static int
merge_load(AG_Maptool *t, AG_Netbuf *buf)
{
	Uint32 i, nbrushes;
	
	if (AG_ReadVersion(buf, &merge_ver, NULL) != 0)
		return (-1);
	merge_free_brushes();
	AG_TlistClear(brushes_tl);

	nbrushes = AG_ReadUint32(buf);
	for (i = 0; i < nbrushes; i++) {
		char m_name[AG_OBJECT_NAME_MAX];
		AG_Map *nbrush;

		AG_CopyString(m_name, buf, sizeof(m_name));
		nbrush = Malloc(sizeof(AG_Map), M_OBJECT);
		AG_MapInit(nbrush, m_name);
		map_load(nbrush, buf);

		TAILQ_INSERT_HEAD(&brushes, AGOBJECT(nbrush), cobjs);
		AG_TlistAddPtrHead(brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

static int
merge_save(AG_Maptool *t, AG_Netbuf *buf)
{
	AG_Object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	AG_WriteVersion(buf, &merge_ver);

	count_offs = AG_NetbufTell(buf);				/* Skip count */
	AG_WriteUint32(buf, 0);	
	TAILQ_FOREACH(ob, &brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		AG_WriteString(buf, ob->name);
		map_save(brush, buf);
		nbrushes++;
	}
	AG_PwriteUint32(buf, nbrushes, count_offs);	/* Write count */
	return (0);
}

static int
merge_cursor(AG_Maptool *t, SDL_Rect *rd)
{
	AG_Mapview *mv = t->mv;
	AG_Nitem *r;
	AG_Map *sm;
	int sx, sy, dx, dy;
	AG_TlistItem *it;
	int rv = -1;
	
	/* XXX ugly work around circular ref */
	if (strncmp(AGOBJECT(mv->map)->name, "brush(", 6) == 0)
		return (-1);

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		if (!it->selected) {
			continue;
		}
		sm = it->p1;
		for (sy = 0, dy = rd->y;
		     sy < sm->maph;
		     sy++, dy += AGMTILESZ(mv)) {
			for (sx = 0, dx = rd->x;
			     sx < sm->mapw;
			     sx++, dx += AGMTILESZ(mv)) {
				AG_Node *sn = &sm->map[sy][sx];

				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					AG_NitemDraw(mv->map, r,
					    AGWIDGET(mv)->cx + dx,
					    AGWIDGET(mv)->cy + dy,
					    mv->cam);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS) {
					AG_MapviewDraw_props(mv, sn, dx, dy,
					    -1, -1);
				}
			}
		}
	}
	return (rv);
}

const AG_Maptool merge_tool = {
	"merge",
	N_("Merge Pattern"),
	MERGE_TOOL_ICON, -1,
	0,
	merge_init,
	merge_destroy,
	merge_load,
	merge_save,
	merge_cursor,
	merge_effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
	NULL			/* pane */
};

#endif /* MAP */
