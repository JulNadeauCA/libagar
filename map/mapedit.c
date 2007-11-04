/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <gui/widget.h>
#include <gui/window.h>
#include <gui/box.h>
#include <gui/checkbox.h>
#include <gui/spinbutton.h>
#include <gui/mspinbutton.h>

#include "map.h"
#include "mapedit.h"

const AG_ObjectOps mapEditorPseudoOps = {
	"MAP_EditorPseudo",
	sizeof(AG_Object),
	{ 0, 0 },
	NULL,			/* init */
	NULL,			/* reinit */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	MAP_EditorConfig	/* edit */
};

extern int mapViewAnimatedBg, mapViewBgTileSize;
extern int mapViewEditSelOnly;

int mapEditorInited = 0;
MAP_Editor mapEditor;

int mapDefaultWidth = 9;		/* Default map geometry */
int mapDefaultHeight = 9;
int mapDefaultBrushWidth = 9;		/* Default brush geometry */
int mapDefaultBrushHeight = 9;

void
MAP_EditorInit(void)
{
	AG_ObjectInit(&mapEditor, "_mapEditor", &mapEditorOps);
	OBJECT(&mapEditor)->flags |= (AG_OBJECT_RELOAD_PROPS|
	                                AG_OBJECT_STATIC);
	OBJECT(&mapEditor)->save_pfx = "/_mapEditor";

	/* Attach a pseudo-object for dependency keeping purposes. */
	AG_ObjectInit(&mapEditor.pseudo, "_mapEditor", &mapEditorPseudoOps);
	OBJECT(&mapEditor.pseudo)->flags |= (AG_OBJECT_NON_PERSISTENT|
				               AG_OBJECT_STATIC|
	                                       AG_OBJECT_INDESTRUCTIBLE);
	AG_ObjectAttach(agWorld, &mapEditor.pseudo);

	/*
	 * Allocate the copy/paste buffer.
	 * Use AG_OBJECT_READONLY to avoid circular reference in case a user
	 * attempts to paste contents of the copy buffer into itself.
	 */
	MAP_Init(&mapEditor.copybuf, "_copyPasteBuffer");
	OBJECT(&mapEditor.copybuf)->flags |= (AG_OBJECT_NON_PERSISTENT|
				              AG_OBJECT_STATIC|
	                                      AG_OBJECT_INDESTRUCTIBLE|
					      AG_OBJECT_READONLY);
	AG_ObjectAttach(&mapEditor.pseudo, &mapEditor.copybuf);

	/* Initialize the default tunables. */
	AG_SetUint32(&mapEditor, "default-map-width", 12);
	AG_SetUint32(&mapEditor, "default-map-height", 8);
	AG_SetUint32(&mapEditor, "default-brush-width", 5);
	AG_SetUint32(&mapEditor, "default-brush-height", 5);
}

static void
Destroy(void *p)
{
	MAP_Destroy(&mapEditor.copybuf);
}

void
MAP_EditorSave(AG_DataSource *buf)
{
	AG_WriteUint8(buf, 0);				/* Pad: mapViewBg */
	AG_WriteUint8(buf, (Uint8)mapViewAnimatedBg);
	AG_WriteUint16(buf, (Uint16)mapViewBgTileSize);
	AG_WriteUint8(buf, (Uint8)mapViewEditSelOnly);

	AG_WriteUint16(buf, (Uint16)mapDefaultWidth);
	AG_WriteUint16(buf, (Uint16)mapDefaultHeight);
	AG_WriteUint16(buf, (Uint16)mapDefaultBrushWidth);
	AG_WriteUint16(buf, (Uint16)mapDefaultBrushHeight);
}

void
MAP_EditorLoad(AG_DataSource *buf)
{
	AG_ReadUint8(buf);				/* Pad: mapViewBg */
	mapViewAnimatedBg = (int)AG_ReadUint8(buf);
	mapViewBgTileSize = (int)AG_ReadUint16(buf);
	mapViewEditSelOnly = (int)AG_ReadUint8(buf);

	mapDefaultWidth = (int)AG_ReadUint16(buf);
	mapDefaultHeight = (int)AG_ReadUint16(buf);
	mapDefaultBrushWidth = (int)AG_ReadUint16(buf);
	mapDefaultBrushHeight = (int)AG_ReadUint16(buf);
}

void *
MAP_EditorConfig(void *p)
{
	AG_Window *win;
	AG_Checkbox *cb;
	AG_Spinbutton *sb;
	AG_MSpinbutton *msb;
	AG_Box *bo;

	win = AG_WindowNew(AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, _("Map editor settings"));

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	AG_BoxSetSpacing(bo, 5);
	{
		cb = AG_CheckboxNew(bo,0 , _("Moving tiles"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &mapViewAnimatedBg);

		sb = AG_SpinbuttonNew(bo, 0, _("Tile size: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &mapViewBgTileSize);
		AG_SpinbuttonSetMin(sb, 2);
		AG_SpinbuttonSetMax(sb, 16384);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		cb = AG_CheckboxNew(bo, 0, _("Selection-bounded edition"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &mapViewEditSelOnly);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		msb = AG_MSpinbuttonNew(bo, 0, "x",
		    _("Default map geometry: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &mapDefaultWidth);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &mapDefaultHeight);
		AG_MSpinbuttonSetMin(msb, 1);
		AG_MSpinbuttonSetMax(msb, MAP_WIDTH_MAX);
		
		msb = AG_MSpinbuttonNew(bo, 0, "x",
		    _("Default brush geometry: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT,
		    &mapDefaultBrushWidth);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT,
		    &mapDefaultBrushHeight);
		AG_MSpinbuttonSetMin(msb, 1);
		AG_MSpinbuttonSetMax(msb, MAP_WIDTH_MAX);
	}
	return (win);
}

const AG_ObjectOps mapEditorOps = {
	"MAP_Editor",
	sizeof(AG_Object),
	{ 0, 0 },
	NULL,		/* init */
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
