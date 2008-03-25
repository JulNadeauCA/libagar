/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Graphics Interface. This is the base class for graphical display drivers
 * (e.g., X11, SDL, OpenGL). This subsystem is used by the Agar GUI, but it
 * is independent from it (applications without any GUI can use GI directly).
 * Applications can operate on multiple displays/contexts/windows simply by
 * creating multiple GI objects.
 *
 * Unfortunately, many graphics interfaces (such as SDL) are forced to tie
 * input devices to display contexts. When this is the case, the GI driver
 * itself has to create and manage one or more ID (Input Device) objects.
 *
 * Basic graphical operations such as blitting, scaling and conversion
 * between pixel formats are also implemented in the GI library. The GI
 * class also provides primitives (e.g., lines, polygons) which are always
 * available, but may or may not be hardware-optimized depending on the
 * underlying graphics driver in use.
 */

#include <core/core.h>
#include <gui/window.h>
#include <gui/label.h>

const char *giTypeNames[] = {
	"Dummy",
	"Vector",
	"Hardware Framebuffer",
	"Software Framebuffer",
	"Network Protocol",
	"Encoder",
	"Other"
};

void
GI_InitSubsystem(void)
{
	AG_RegisterClass(&giClass);
}

void
GI_DestroySubsystem(void)
{
}

static void
Init(void *obj)
{
	GI *gi = obj;

	gi->type = GI_DISPLAY_DUMMY;
	gi->fb = NULL;
	gi->fbFmt = NULL;
	gi->flags = 0;
	gi->caps = 0;
	gi->refreshRate = -1;
	gi->vblank.upperBeam = 0;
	gi->vblank.lowerBeam = 0;
}

static void *
Edit(void *obj)
{
	GI *gi = obj;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Graphics Interface: %s", AGOBJECT(gi)->name);
	AG_LabelNew(win, 0, "Class: %s", AGOBJECT(gi)->cls->name);
	AG_LabelNew(win, 0, "Type: %s", giTypeNames[gi->type]);
	AG_LabelNew(win, 0, "Flags: 0x%x", gi->flags);
	AG_LabelNew(win, 0, "Capabilities: 0x%x", gi->caps);
	AG_LabelNew(win, 0, "Refresh Rate: %d", gi->refreshRate);
	return (win);
}

GI_Class giClass = {
	{
		"GI",
		sizeof(GI),
		{ 0,0 },
		Init,
		NULL,			/* reinit */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		Edit
	}
};
