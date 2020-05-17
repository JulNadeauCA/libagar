/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/numerical.h>

#include <agar/map/map.h>

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
	AG_ObjectInit(&mapEditor, &mapEditorClass);
	OBJECT(&mapEditor)->flags |= AG_OBJECT_STATIC;
	AG_ObjectSetName(&mapEditor, "_mapEditor");

	/* Initialize the default tunables. */
	AG_SetUint32(&mapEditor, "map-width", 12);
	AG_SetUint32(&mapEditor, "map-height", 8);
	AG_SetUint32(&mapEditor, "brush-width", 5);
	AG_SetUint32(&mapEditor, "brush-height", 5);
}

static void
Destroy(void *_Nonnull p)
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

static void *_Nonnull
ConfigEditor(void *_Nonnull p)
{
	AG_Window *win;
	AG_MSpinbutton *msb;
	AG_Box *bo;

	if ((win = AG_WindowNew(AG_WINDOW_NOVRESIZE)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, _("Map editor settings"));

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	AG_SetStyle(bo, "spacing", "5");

	AG_CheckboxNewInt(bo, 0, _("Moving tiles"), &mapViewAnimatedBg);

	AG_NumericalNewIntR(bo, 0, "px", _("Tile size: "),
	    &mapViewBgTileSize, 2, 16384);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);

	AG_CheckboxNewInt(bo, 0, _("Selection-bounded edition"),
	    &mapViewEditSelOnly);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		msb = AG_MSpinbuttonNew(bo, 0, "x",
		    _("Default map geometry: "));
		AG_BindInt(msb, "xvalue", &mapDefaultWidth);
		AG_BindInt(msb, "yvalue", &mapDefaultHeight);
		AG_MSpinbuttonSetMin(msb, 1);
		AG_MSpinbuttonSetMax(msb, MAP_WIDTH_MAX);
		
		msb = AG_MSpinbuttonNew(bo, 0, "x",
		    _("Default brush geometry: "));
		AG_BindInt(msb, "xvalue", &mapDefaultBrushWidth);
		AG_BindInt(msb, "yvalue", &mapDefaultBrushHeight);
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
	NULL,			/* reset */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	ConfigEditor
};

AG_ObjectClass mapEditorClass = {
	"MAP(Editor)",
	sizeof(AG_Object),
	{ 0, 0 },
	NULL,			/* init */
	NULL,			/* reset */
	Destroy,
	NULL,			/* load */
	NULL,			/* save */
	NULL			/* edit */
};
