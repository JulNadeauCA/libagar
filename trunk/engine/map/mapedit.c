/*	$Csoft: mapedit.c,v 1.5 2005/09/19 01:25:18 vedge Exp $	*/

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

#ifdef MAP

#include <engine/prop.h>
#include <engine/objmgr.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>

#include "map.h"
#include "mapedit.h"

const AG_ObjectOps agMapEditorOps = {
	NULL,				/* init */
	NULL,				/* reinit */
	AG_MapEditorDestroy,
	NULL,				/* load */
	NULL,				/* save */
	NULL				/* edit */
};

const AG_ObjectOps agMapEditorPseudoOps = {
	NULL,			/* init */
	NULL,			/* reinit */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	AG_MapEditorConfig	/* edit */
};

extern int agMapviewAnimatedBg, agMapviewBgTileSize;
extern int agMapviewEditSelOnly;

AG_MapEditor agMapEditor;

int agEditMode = 0;			/* Start up in edition mode */
int agMapDefaultWidth = 9;		/* Default map geometry */
int agMapDefaultHeight = 9;
int agMapDefaultBrushWidth = 9;		/* Default brush geometry */
int agMapDefaultBrushHeight = 9;

void
AG_MapEditorInit(void)
{
	AG_ObjectInit(&agMapEditor, "object", "map-editor", &agMapEditorOps);
	AGOBJECT(&agMapEditor)->flags |= (AG_OBJECT_RELOAD_PROPS|
	                                   AG_OBJECT_STATIC);
	AGOBJECT(&agMapEditor)->save_pfx = "/map-editor";

	/* Attach a pseudo-object for dependency keeping purposes. */
	AG_ObjectInit(&agMapEditor.pseudo, "object", "map-editor",
	    &agMapEditorPseudoOps);
	AGOBJECT(&agMapEditor.pseudo)->flags |= (AG_OBJECT_NON_PERSISTENT|
				                  AG_OBJECT_STATIC|
	                                          AG_OBJECT_INDESTRUCTIBLE);
	AG_ObjectAttach(agWorld, &agMapEditor.pseudo);

	/*
	 * Allocate the copy/paste buffer.
	 * Use AG_OBJECT_READONLY to avoid circular reference in case a user
	 * attempts to paste contents of the copy buffer into itself.
	 */
	AG_MapInit(&agMapEditor.copybuf, "copybuf");
	AGOBJECT(&agMapEditor.copybuf)->flags |= (AG_OBJECT_NON_PERSISTENT|
				               AG_OBJECT_STATIC|
	                                       AG_OBJECT_INDESTRUCTIBLE|
					       AG_OBJECT_READONLY);
	AG_ObjectAttach(&agMapEditor.pseudo, &agMapEditor.copybuf);

	agEditMode = 1;

	/* Initialize the default tunables. */
	AG_SetUint32(&agMapEditor, "default-map-width", 12);
	AG_SetUint32(&agMapEditor, "default-map-height", 8);
	AG_SetUint32(&agMapEditor, "default-brush-width", 5);
	AG_SetUint32(&agMapEditor, "default-brush-height", 5);

	/* Initialize the object manager. */
	AG_ObjMgrInit();
	AG_WindowShow(AG_ObjMgrWindow());
}

void
AG_MapEditorDestroy(void *p)
{
	map_destroy(&agMapEditor.copybuf);
	AG_ObjMgrDestroy();
}

void
AG_MapEditorSave(AG_Netbuf *buf)
{
	AG_WriteUint8(buf, 0);				/* Pad: agMapviewBg */
	AG_WriteUint8(buf, (Uint8)agMapviewAnimatedBg);
	AG_WriteUint16(buf, (Uint16)agMapviewBgTileSize);
	AG_WriteUint8(buf, (Uint8)agMapviewEditSelOnly);

	AG_WriteUint16(buf, (Uint16)agMapDefaultWidth);
	AG_WriteUint16(buf, (Uint16)agMapDefaultHeight);
	AG_WriteUint16(buf, (Uint16)agMapDefaultBrushWidth);
	AG_WriteUint16(buf, (Uint16)agMapDefaultBrushHeight);
}

void
AG_MapEditorLoad(AG_Netbuf *buf)
{
	AG_ReadUint8(buf);				/* Pad: agMapviewBg */
	agMapviewAnimatedBg = (int)AG_ReadUint8(buf);
	agMapviewBgTileSize = (int)AG_ReadUint16(buf);
	agMapviewEditSelOnly = (int)AG_ReadUint8(buf);

	agMapDefaultWidth = (int)AG_ReadUint16(buf);
	agMapDefaultHeight = (int)AG_ReadUint16(buf);
	agMapDefaultBrushWidth = (int)AG_ReadUint16(buf);
	agMapDefaultBrushHeight = (int)AG_ReadUint16(buf);
}

void *
AG_MapEditorConfig(void *p)
{
	AG_Window *win;
	AG_Checkbox *cb;
	AG_Spinbutton *sb;
	AG_MSpinbutton *msb;
	AG_Box *bo;

	win = AG_WindowNew(AG_WINDOW_NO_VRESIZE, NULL);
	AG_WindowSetCaption(win, _("Map editor settings"));

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	AG_BoxSetSpacing(bo, 5);
	{
		cb = AG_CheckboxNew(bo, _("Moving tiles"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &agMapviewAnimatedBg);

		sb = AG_SpinbuttonNew(bo, _("Tile size: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &agMapviewBgTileSize);
		AG_SpinbuttonSetMin(sb, 2);
		AG_SpinbuttonSetMax(sb, 16384);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		cb = AG_CheckboxNew(bo, _("Selection-bounded edition"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT,
		    &agMapviewEditSelOnly);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		msb = AG_MSpinbuttonNew(bo, "x", _("Default map geometry: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT,
		    &agMapDefaultWidth);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT,
		    &agMapDefaultHeight);
		AG_MSpinbuttonSetMin(msb, 1);
		AG_MSpinbuttonSetMax(msb, AG_MAP_MAXWIDTH);
		
		msb = AG_MSpinbuttonNew(bo, "x", _("Default brush geometry: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT,
		    &agMapDefaultBrushWidth);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT,
		    &agMapDefaultBrushHeight);
		AG_MSpinbuttonSetMin(msb, 1);
		AG_MSpinbuttonSetMax(msb, AG_MAP_MAXWIDTH);
	}
	return (win);
}

#endif /* MAP */
