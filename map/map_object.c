/*
 * Copyright (c) 2022 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Dynamic map object.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>
#include <agar/gui/numerical.h>

#include <agar/map/map.h>

MAP_Object *
MAP_ObjectNew(void *parent, const char *name)
{
	MAP_Object *mo;

	mo = Malloc(sizeof(MAP_Object));
	AG_ObjectInit(mo, &mapObjectClass);
	AG_ObjectSetNameS(mo, name);
	AG_ObjectAttach(parent, mo);
	return (mo);
}

static void
Init(void *obj)
{
	MAP_Object *mo = MAPOBJECT(obj);

	mo->id = 0;
	mo->flags = MAP_OBJECT_VALID;
	mo->locs = NULL;
	mo->nLocs = 0;
}

#if 0
static void *
Duplicate(MAP *map, MAP_Node *node, const void *mi)
{
	const MAP_Link *ml = MAPLINK(mi);

	return (void *)MAP_LinkNew(map, node, ml->map, ml->x, ml->y, ml->dir);
}
#endif

static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	MAP_Object *mo = MAPOBJECT(obj);

	mo->id = (Uint)AG_ReadUint32(ds);
	mo->flags = (Uint)AG_ReadUint32(ds);
	if ((mo->flags & MAP_OBJECT_VALID) == 0) {
		AG_SetErrorS("Invalid flags");
		return (-1);
	}
	mo->locs = NULL;             /* Will be updated from MAP_NodeLoad() */
	mo->nLocs = 0;
	return (0);
}

static int
Save(void *obj, AG_DataSource *ds)
{
	MAP_Object *mo = MAPOBJECT(obj);

	AG_WriteUint32(ds, (Uint32)mo->id);
	AG_WriteUint32(ds, (Uint32)mo->flags);
	
	return (0);
}

static int
Update(void *obj)
{
	return (0);
}

static void
Draw(void *obj, MAP_View *mv, const AG_Rect *rd, MAP_ObjectView view)
{
	AG_Color c;
	MAP_Object *mo = obj;

	if (mo->flags & MAP_OBJECT_SELECTED) {
		AG_ColorRGB(&c, 0,255,0);
	} else {
		AG_ColorRGB(&c, 0,100,0);
	}
	AG_DrawRectOutline(mv, rd, &c);
}

static void
DrawGL(void *obj, MAP_View *mv)
{
}

static void
Edit(void *obj, AG_Widget *b, MAP_Tool *tool)
{
	MAP_Object *mo = obj;

	AG_LabelNew(b, 0, _("Object ID: 0x%x"), mo->id);
	AG_LabelNewPolled(b, 0, _("Object Flags: 0x%x"), &mo->flags);
}

MAP_ObjectClass mapObjectClass = {
	{
		"MAP_Object",
		sizeof(MAP_Object),
		{ 1,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		Load,
		Save,
		NULL		/* edit */
	},
	Update,
	Draw,
	DrawGL,
	Edit
};
