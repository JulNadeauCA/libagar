/*	$Csoft: perso.c,v 1.56 2005/09/20 13:46:29 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/hbox.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/objsel.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/separator.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "perso.h"

const AG_Version agPersoVer = {
	"agar personage",
	3, 0
};

const AG_ActorOps agPersoOps = {
	{
		AG_PersoInit,
		AG_PersoReinit,
		AG_PersoDestroy,
		AG_PersoLoad,
		AG_PersoSave,
		AG_PersoEdit
	},
	AG_PersoMap,
	NULL,		/* unmap */
	AG_PersoUpdate,
	AG_PersoKeydown,
	AG_PersoKeyup,
	NULL,		/* mousemotion */
	NULL,		/* mousebuttondown */
	NULL,		/* mousebuttonup */
	NULL,		/* joyaxis */
	NULL,		/* joyball */
	NULL,		/* joyhat */
	NULL		/* joybutton */
};

AG_Perso *
AG_PersoNew(void *parent, const char *name)
{
	AG_Perso *ps;

	ps = Malloc(sizeof(AG_Perso), M_OBJECT);
	AG_PersoInit(ps, name);
	AG_ObjectAttach(parent, ps);
	return (ps);
}

int
AG_PersoCanWalkTo(void *p, int xo, int yo)
{
	AG_Actor *go = p;
	AG_Map *m = go->parent;
	int dx0 = go->g_map.x0 + xo;
	int dy0 = go->g_map.y0 + yo;
	int dx1 = go->g_map.x1 + xo;
	int dy1 = go->g_map.y1 + yo;
	AG_Node *n;
	int x, y;

	if (dx0 < 0 || dy0 < 0 || dx0 >= m->mapw || dy0 >= m->maph ||
	    dx1 < 0 || dy1 < 0 || dx1 >= m->mapw || dy1 >= m->maph)
		return (0);

	for (y = dy0; y < dy1; y++) {
		for (x = dx0; x < dx1; x++) {
			AG_Node *n = &m->map[y][x];
			AG_Nitem *r;

			TAILQ_FOREACH(r, &n->nrefs, nrefs) {
				if (r->p != go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1 &&
				    r->flags & AG_NITEM_BLOCK) {
					dprintf("%s: blocked\n",
					    AGOBJECT(go)->name);
					return (0);
				}
			}
		}
	}
	return (1);
}

static Uint32
move(void *obj, Uint32 ival, void *arg)
{
	AG_Actor *go = obj;
	AG_Perso *ps = obj;
	int xo = 0, yo = 0;

	switch (go->g_map.da) {
	case 0:	  xo = -go->g_map.dv; break;
	case 90:  yo = -go->g_map.dv; break;
	case 180: xo = +go->g_map.dv; break;
	case 270: yo = +go->g_map.dv; break;
	}

	if ((xo != 0 || yo != 0) &&
	    AG_PersoCanWalkTo(ps, xo, yo)) {
		AG_ActorMoveSprite(ps, xo, yo);
	}
	return (ival);
}

void
AG_PersoInit(void *obj, const char *name)
{
	AG_Perso *ps = obj;

	AG_ActorInit(ps, "perso", name, &agPersoOps);
	ps->tileset = NULL;
	ps->name[0] = '\0';
	ps->flags = 0;
	ps->level = 0;
	ps->exp = 0;
	ps->age = 0;
	ps->seed = 0;			/* TODO arc4random */
	ps->hp = ps->maxhp = 1;
	ps->mp = ps->maxmp = 0;
	ps->nzuars = 0;
	pthread_mutex_init(&ps->lock, NULL);
	AG_SetTimeout(&ps->move_to, move, NULL, 0);
}

void
AG_PersoReinit(void *obj)
{
	AG_Perso *ps = obj;
	
	AG_DelTimeout(ps, &ps->move_to);

	AG_ActorReinit(obj);
	
	if (ps->tileset != NULL) {
		AG_ObjectPageOut(ps->tileset, AG_OBJECT_GFX);
		AG_ObjectDelDep(ps, ps->tileset);
		ps->tileset = NULL;
	}
}

void
AG_PersoDestroy(void *obj)
{
	AG_ActorDestroy(obj);
}

int
AG_PersoLoad(void *obj, AG_Netbuf *buf)
{
	AG_Perso *ps = obj;
	void *tileset;
	Uint32 name;

	if (AG_ReadVersion(buf, &agPersoVer, NULL) != 0)
		return (-1);
	
	if (AG_ActorLoad(ps, buf) == -1)
		return (-1);

	pthread_mutex_lock(&ps->lock);
	AG_CopyString(ps->name, buf, sizeof(ps->name));
	ps->flags = AG_ReadUint32(buf);
	name = AG_ReadUint32(buf);

	if (name != 0) {
		if (AG_ObjectFindDep(ps, name, &ps->tileset) == -1) {
			goto fail;
		}
		if (ps->tileset != NULL) {
			AG_ObjectAddDep(ps, ps->tileset);
			AG_ObjectPageIn(ps->tileset, AG_OBJECT_GFX);
		}
	}

	ps->level = AG_ReadSint32(buf);
	ps->exp = AG_ReadUint32(buf);
	ps->age = (int)AG_ReadUint32(buf);
	ps->seed = AG_ReadUint32(buf);
	ps->maxhp = (int)AG_ReadUint32(buf);
	ps->hp = (int)AG_ReadUint32(buf);
	ps->maxmp = (int)AG_ReadUint32(buf);
	ps->mp = (int)AG_ReadUint32(buf);
	ps->nzuars = (u_int)AG_ReadUint32(buf);

	pthread_mutex_unlock(&ps->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ps->lock);
	return (-1);
}

int
AG_PersoSave(void *obj, AG_Netbuf *buf)
{
	AG_Perso *ps = obj;

	AG_WriteVersion(buf, &agPersoVer);
	
	if (AG_ActorSave(ps, buf) == -1)
		return (-1);

	pthread_mutex_lock(&ps->lock);
	AG_WriteString(buf, ps->name);
	AG_WriteUint32(buf, ps->flags);
	AG_WriteUint32(buf, AG_ObjectEncodeName(ps, ps->tileset));

	AG_WriteSint32(buf, ps->level);
	AG_WriteUint32(buf, ps->exp);
	AG_WriteUint32(buf, ps->age);
	AG_WriteUint32(buf, ps->seed);
	AG_WriteUint32(buf, (Uint32)ps->maxhp);
	AG_WriteUint32(buf, (Uint32)ps->hp);
	AG_WriteUint32(buf, (Uint32)ps->maxmp);
	AG_WriteUint32(buf, (Uint32)ps->mp);
	AG_WriteUint32(buf, (Uint32)ps->nzuars);
	pthread_mutex_unlock(&ps->lock);
	return (0);
}

void *
AG_PersoEdit(void *obj)
{
	AG_Perso *ps = obj;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_VBox *vb;

	win = AG_WindowNew(AG_WINDOW_DETACH|AG_WINDOW_NO_VRESIZE, NULL);
	AG_WindowSetCaption(win, _("Character \"%s\""), AGOBJECT(ps)->name);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Informations"), AG_BOX_VERT);
	{
		AG_Textbox *tb;
		AG_Spinbutton *sbu;
		AG_HBox *hb;
		AG_ObjectSelector *os;

		tb = AG_TextboxNew(ntab, _("Name: "));
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, ps->name,
		    sizeof(ps->name));

		os = AG_ObjectSelectorNew(ntab, AG_OBJSEL_PAGE_GFX, ps, agWorld,
		    _("Tileset: "));
		AG_ObjectSelectorMaskType(os, "tileset");
		AG_WidgetBind(os, "object", AG_WIDGET_POINTER, &ps->tileset);
		AG_ObjectSelectorSelect(os, ps->tileset);

		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		sbu = AG_SpinbuttonNew(ntab, _("Level: "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_SINT32, &ps->level);

		sbu = AG_SpinbuttonNew(ntab, _("Experience: "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_UINT32, &ps->exp);

		sbu = AG_SpinbuttonNew(ntab, _("Zuars: "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_UINT32, &ps->nzuars);

		hb = AG_HBoxNew(ntab, AG_HBOX_HOMOGENOUS|AG_HBOX_WFILL);
		{
			sbu = AG_SpinbuttonNew(hb, _("HP: "));
			AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &ps->hp);
			
			sbu = AG_SpinbuttonNew(hb, " / ");
			AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &ps->maxhp);
		}
		
		hb = AG_HBoxNew(ntab, AG_HBOX_HOMOGENOUS|AG_HBOX_WFILL);
		{
			sbu = AG_SpinbuttonNew(hb, _("MP: "));
			AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &ps->mp);
			
			sbu = AG_SpinbuttonNew(hb, " / ");
			AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &ps->maxmp);
		}
	}
	ntab = AG_NotebookAddTab(nb, _("Position"), AG_BOX_VERT);
	AG_ActorEdit(AGACTOR(ps), ntab);
	return (win);
}

void
AG_PersoMap(void *obj, void *space)
{
	AG_Perso *ps = obj;

	dprintf("%s: mapping into %s\n", AGOBJECT(ps)->name,
	    AGOBJECT(space)->name);

	if (AGOBJECT_TYPE(space, "map")) {
		AG_Map *m = space;

		if (AG_ActorMapSprite(ps, 0, 0, 0, ps->tileset, "Idle-S")
		    == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s->%s: %s",
			    AGOBJECT(obj)->name, AGOBJECT(space)->name,
			    AG_GetError());
		}
	}
#ifdef HAVE_OPENGL
	else if (AGOBJECT_TYPE(space, "scene")) {
		glBegin(GL_LINE_LOOP);
		glVertex3f(-3, +3, 0);
		glVertex3f(-3, -3, 0);
		glVertex3f(+3, -3, 0);
		glVertex3f(+3, +3, 0);
		glEnd();
	}
#endif
}

void
AG_PersoUpdate(void *space, void *obj)
{
	AG_Perso *ps = obj;
	
	dprintf("%s: update (space=%s)\n", AGOBJECT(ps)->name,
	    AGOBJECT(space)->name);
}

int
AG_PersoKeydown(void *p, int ks, int km)
{
	AG_Actor *go = p;
	AG_Perso *ps = p;

	switch (ks) {
	case SDLK_LEFT:
		go->g_map.da = 0;
		go->g_map.dv = 1;
		AG_ActorSetSprite(go, 0, 0, 0, ps->tileset, "Idle-W");
		break;
	case SDLK_UP:
		go->g_map.da = 90;
		go->g_map.dv = 1;
		AG_ActorSetSprite(go, 0, 0, 0, ps->tileset, "Idle-N");
		break;
	case SDLK_RIGHT:
		go->g_map.da = 180;
		go->g_map.dv = 1;
		AG_ActorSetSprite(go, 0, 0, 0, ps->tileset, "Idle-E");
		break;
	case SDLK_DOWN:
		go->g_map.da = 270;
		go->g_map.dv = 1;
		AG_ActorSetSprite(go, 0, 0, 0, ps->tileset, "Idle-S");
		break;
	}
	if (go->g_map.dv > 0) {
		AG_AddTimeout(p, &ps->move_to, 10);
	}
	return (0);
}

int
AG_PersoKeyup(void *p, int ks, int km)
{
	AG_Perso *ps = p;
	AG_Actor *go = p;

	switch (ks) {
	case SDLK_LEFT:
		if (go->g_map.da == 0) { goto stop; }
		break;
	case SDLK_UP:
		if (go->g_map.da == 90) { goto stop; }
		break;
	case SDLK_RIGHT:
		if (go->g_map.da == 180) { goto stop; }
		break;
	case SDLK_DOWN:
		if (go->g_map.da == 270) { goto stop; }
		break;
	}
	return (0);
stop:
	AG_DelTimeout(ps, &ps->move_to);
	go->g_map.dv = 0;
	return (0);
}
