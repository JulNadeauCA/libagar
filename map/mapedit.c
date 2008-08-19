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
	AG_ObjectInitStatic(&mapEditor, &mapEditorClass);
	AG_ObjectSetName(&mapEditor, "_mapEditor");
	OBJECT(&mapEditor)->flags |= AG_OBJECT_RELOAD_PROPS;
	OBJECT(&mapEditor)->save_pfx = "/_mapEditor";

	/* Initialize the default tunables. */
	AG_SetUint32(&mapEditor, "default-map-width", 12);
	AG_SetUint32(&mapEditor, "default-map-height", 8);
	AG_SetUint32(&mapEditor, "default-brush-width", 5);
	AG_SetUint32(&mapEditor, "default-brush-height", 5);
}

static void
Destroy(void *p)
{
	AG_ObjectDestroy(&mapEditor.copybuf);
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

static void *
ConfigEditor(void *p)
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

AG_ObjectClass mapEditorPseudoClass = {
	"MAP(EditorPseudo)",
	sizeof(AG_Object),
	{ 0, 0 },
	NULL,			/* init */
	NULL,			/* free */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	ConfigEditor
};

AG_ObjectClass mapEditorClass = {
	"MAP(Editor)",
	sizeof(AG_Object),
	{ 0, 0 },
	NULL,		/* init */
	NULL,		/* free */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
