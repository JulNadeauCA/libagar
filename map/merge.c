/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include <gui/window.h>
#include <gui/hbox.h>
#include <gui/textbox.h>
#include <gui/button.h>
#include <gui/tlist.h>
#include <gui/toolbar.h>

#include "map.h"
#include "mapedit.h"
#include "icons.h"

#include <string.h>

const AG_Version merge_ver = {
	"agar merge tool",
	6, 0
};

static void merge_create_brush(AG_Event *);
static void merge_edit_brush(AG_Event *);
static void merge_remove_brush(AG_Event *);
static void merge_interpolate(MAP *, MAP_Node *,
	                      MAP_Item *, MAP *, MAP_Node *,
			      MAP_Item *);

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
merge_init(MAP_Tool *t)
{
	AG_Window *win;
	AG_HBox *hb;
	AG_Button *bu;
	AG_Textbox *tb;

	win = MAP_ToolWindow(t, "mapedit-tool-merge");

	hb = AG_HBoxNew(win, AG_HBOX_HFILL);
	{
		tb = AG_TextboxNew(hb, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS,
		    _("Name: "));
		AG_SetEvent(tb, "textbox-return", merge_create_brush, "%p", tb);

		bu = AG_ButtonNew(hb, 0, _("Create"));
		AG_ButtonSetPadding(bu, 6);			/* Align */
		AG_ButtonSetFocusable(bu, 0);
		AG_SetEvent(bu, "button-pushed", merge_create_brush, "%p", tb);
	}

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
#if 0
		bu = AG_ButtonNew(hb, 0, _("Load set"));
		AG_SetEvent(bu, "button-pushed", merge_load_brush_set, NULL);
		bu = AG_ButtonNew(hb, 0, _("Save set"));
		AG_SetEvent(bu, "button-pushed", merge_save_brush_set, NULL);
#endif
		bu = AG_ButtonNew(hb, 0, _("Edit"));
		AG_SetEvent(bu, "button-pushed", merge_edit_brush, "%p", win);
		bu = AG_ButtonNew(hb, 0, _("Remove"));
		AG_SetEvent(bu, "button-pushed", merge_remove_brush, NULL);
	}

	brushes_tl = AG_TlistNew(win, AG_TLIST_MULTI|AG_TLIST_EXPAND);
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
	}
	TAILQ_INIT(&brushes);
}

static void
merge_destroy(MAP_Tool *t)
{
	merge_free_brushes();
}

static void
merge_create_brush(AG_Event *event)
{
	char brush_name[AG_OBJECT_NAME_MAX];
	char m_name[AG_OBJECT_NAME_MAX];
	AG_Textbox *name_tbox = AG_PTR(1);
	MAP *m;

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

	m = MAP_New(NULL, m_name);
	if (AG_ObjectLoad(m) == -1) {
		extern int mapDefaultBrushWidth, mapDefaultBrushHeight;

		if (MAP_AllocNodes(m, mapDefaultBrushWidth,
		    mapDefaultBrushHeight) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "MAP_AllocNodes: %s",
			    AG_GetError());
			goto fail;
		}
	}

	TAILQ_INSERT_HEAD(&brushes, OBJECT(m), cobjs);
	AG_TlistDeselectAll(brushes_tl);
	AG_TlistSelect(brushes_tl, AG_TlistAddPtrHead(brushes_tl, NULL,
	    m_name, m));
	AG_TextboxPrintf(name_tbox, NULL);
	return;
fail:
	AG_ObjectDestroy(m);
}

static void
merge_edit_brush(AG_Event *event)
{
	AG_TlistItem *it;
	AG_Window *pwin = AG_PTR(1);

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		MAP *brush = it->p1;
		AG_Window *win;
		AG_Toolbar *tbar;

		if (!it->selected)
			continue;

		if ((win = AG_WindowNewNamed(0, "mapedit-tool-merge-%s",
		    OBJECT(brush)->name)) == NULL)
			continue;

		tbar = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 1, 0);
		MAP_ViewNew(win, brush, MAP_VIEW_EDIT|MAP_VIEW_GRID|
		                        MAPVIEW_PROPS, tbar, NULL);
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
merge_remove_brush(AG_Event *event)
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
			    OBJECT(brush)->name);
			if ((win = AG_FindWindow(wname)) != NULL) {
				AG_WindowHide(win);
				AG_ViewDetach(win);
			}

			TAILQ_REMOVE(&brushes, brush, cobjs);
			AG_TlistDel(brushes_tl, it);
			AG_ObjectDestroy(brush);
		}
	}
}

static void
merge_interpolate(MAP *sm, MAP_Node *sn, MAP_Item *sr,
    MAP *dm, MAP_Node *dn, MAP_Item *dr)
{
	/* TODO */
}

static int
merge_effect(MAP_Tool *t, MAP_Node *n)
{
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	AG_TlistItem *it;
	
	/* Avoid circular references. XXX ugly */
	if (strncmp(OBJECT(m)->name, "brush(", 6) == 0)
		return (1);
	
	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		MAP *sm;
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
				MAP_Node *sn = &sm->map[sy][sx];
				MAP_Node *dn = &m->map[dy][dx];
				MAP_Item *sr, *dr;

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
merge_load(MAP_Tool *t, AG_DataSource *buf)
{
	Uint32 i, nbrushes;
	
	if (AG_ReadVersion(buf, &merge_ver, NULL) != 0)
		return (-1);
	merge_free_brushes();
	AG_TlistClear(brushes_tl);

	nbrushes = AG_ReadUint32(buf);
	for (i = 0; i < nbrushes; i++) {
		char m_name[AG_OBJECT_NAME_MAX];
		MAP *nbrush;

		AG_CopyString(m_name, buf, sizeof(m_name));
		nbrush = Malloc(sizeof(MAP));
		MAP_Init(nbrush, m_name);
		map_load(nbrush, buf);

		TAILQ_INSERT_HEAD(&brushes, OBJECT(nbrush), cobjs);
		AG_TlistAddPtrHead(brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

static int
merge_save(MAP_Tool *t, AG_DataSource *buf)
{
	AG_Object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	AG_WriteVersion(buf, &merge_ver);
	count_offs = AG_Tell(buf);			/* Skip count */
	AG_WriteUint32(buf, 0);	
	TAILQ_FOREACH(ob, &brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		AG_WriteString(buf, ob->name);
		map_save(brush, buf);
		nbrushes++;
	}
	AG_WriteUint32At(buf, nbrushes, count_offs);	/* Write count */
	return (0);
}

static int
merge_cursor(MAP_Tool *t, SDL_Rect *rd)
{
	MAP_View *mv = t->mv;
	MAP_Item *r;
	MAP *sm;
	int sx, sy, dx, dy;
	AG_TlistItem *it;
	int rv = -1;
	
	/* XXX ugly work around circular ref */
	if (strncmp(OBJECT(mv->map)->name, "brush(", 6) == 0)
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
				MAP_Node *sn = &sm->map[sy][sx];

				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					MAP_ItemDraw(mv->map, r,
					    WIDGET(mv)->cx + dx,
					    WIDGET(mv)->cy + dy,
					    mv->cam);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS) {
					MAP_ViewDraw_props(mv, sn, dx, dy,
					    -1, -1);
				}
			}
		}
	}
	return (rv);
}

const MAP_Tool mapMergeOps = {
	"Merge", N_("Merge Pattern"),
	&mapIconMerge,
	sizeof(MAP_Tool),
	0,
	merge_init,
	merge_destroy,
	NULL,			/* pane */
	NULL,			/* edit */
	merge_cursor,
	merge_effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
	NULL			/* pane */
};
