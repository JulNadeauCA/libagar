/*	$Csoft$	*/

/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/sg.h>
#include <agar/sc.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "voxelpath.h"

/*
 * Since our trajectory planning algorithm must be integer-only, we use
 * the STEPLEN constant to adjust maximum travel vs. sub-step accuracy.
 */
#define STEPMAX	(0xffffffff-1)
#define STEPLEN	204800
#define MAXSTEPS (STEPMAX/STEPLEN)

static int x1 = 10, y_1 = 10, z1 = 10;
static int x2 = 100, y2 = 70, z2 = 80;

/*
 * Trace a linear path. We simply find the component with the largest
 * difference and integrate the other components over it.
 */
void
VoxelLine(SG_Voxel *vol)
{
	int dx, dy, dz;
	int x, y, z;
	int vx, vy, vz;
	int xdir, ydir, zdir;
	int step;

	printf("Line: [%d,%d,%d] -> [%d,%d,%d]\n", x1, y_1, z1, x2, y2, z2);

	dx = abs(x2 - x1);
	dy = abs(y2 - y_1);
	dz = abs(z2 - z1);
	xdir = (x2 > x1) ? 1 : -1;
	ydir = (y2 > y_1) ? 1 : -1;
	zdir = (z2 > z1) ? 1 : -1;

	printf("Delta = %d,%d,%d\n", dx, dy, dz);
	printf("Dir = %d,%d,%d\n", xdir, ydir, zdir);

	if (dx >= dy && dx >= dz) {
		if (dx == 0) {
			return;
		}
		dy = dy*STEPLEN/dx;
		dz = dz*STEPLEN/dx;
		dx *= STEPLEN;
		for (x = 0, step = 0;
		     x < dx && step < MAXSTEPS;
		     x += STEPLEN, step++) {
			y = dy*ydir*step;
			z = dz*zdir*step;
			SG_VoxelSet3(vol,
			    x1 + xdir*step,
			    y_1 + y/STEPLEN,
			    z1 + z/STEPLEN,
			    0.5);
		}
	} else if (dy >= dx && dy >= dz) {
		if (dy == 0) {
			return;
		}
		dx = dx*STEPLEN/dy;
		dz = dz*STEPLEN/dy;
		dy *= STEPLEN;
		for (y = 0, step = 0;
		     y < dy && step < MAXSTEPS;
		     y += STEPLEN, step++) {
			x = dx*xdir*step;
			z = dz*zdir*step;
			SG_VoxelSet3(vol,
			    x1 + x/STEPLEN,
			    y_1 + ydir*step,
			    z1 + z/STEPLEN,
			    0.5);
		}
	} else if (dz >= dx && dz >= dy) {
		if (dz == 0) {
			return;
		}
		dx = dx*STEPLEN/dz;
		dy = dy*STEPLEN/dz;
		dz *= STEPLEN;
		for (z = 0, step = 0;
		     z < dz && step < MAXSTEPS;
		     z += STEPLEN, step++) {
			x = dx*xdir*step;
			y = dy*ydir*step;
			SG_VoxelSet3(vol,
			    x1 + x/STEPLEN,
			    y_1 + y/STEPLEN,
			    z1 + zdir*step,
			    0.5);
		}
	}
}

/* Trace a circular path on a plane. */
static void
SG_VoxelCircle(SG_Voxel *vol, int wx, int wy, int wz, int radius)
{
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		SG_VoxelSet3(vol, wx+x, wy+y, wz, 1.0);
		SG_VoxelSet3(vol, wx+x, wy-y, wz, 1.0);
		SG_VoxelSet3(vol, wx-x, wy+y, wz, 1.0);
		SG_VoxelSet3(vol, wx-x, wy-y, wz, 1.0);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;

		SG_VoxelSet3(vol, wx+y, wy+x, wz, 1.0);
		SG_VoxelSet3(vol, wx+y, wy-x, wz, 1.0);
		SG_VoxelSet3(vol, wx-y, wy+x, wz, 1.0);
		SG_VoxelSet3(vol, wx-y, wy-x, wz, 1.0);
	}
	SG_VoxelSet3(vol, wx-radius, wy, wz, 1.0);
	SG_VoxelSet3(vol, wx+radius, wy, wz, 1.0);
}

static void
DoClearVoxel(AG_Event *event)
{
	SG_Voxel *vol = AG_PTR(1);

	SG_VoxelReset(vol, 0.0);
}

static void
DoFillVoxel(AG_Event *event)
{
	SG_Voxel *vol = AG_PTR(1);

	SG_VoxelReset(vol, 1.0);
}

static void
DoTraceLine(AG_Event *event)
{
	SG_Voxel *vol = AG_PTR(1);

	SG_VoxelReset(vol, 0.0);
	VoxelLine(vol);
}

void
VoxelPathDialog(SG_Voxel *vol)
{
	AG_Window *win;
	AG_Box *box;
	AG_Spinbutton *sb;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Voxel Operations");
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_LEFT, 0);
	AG_ButtonAct(win, AG_BUTTON_HFILL, "Clear", DoClearVoxel, "%p", vol);
	AG_ButtonAct(win, AG_BUTTON_HFILL, "Fill", DoFillVoxel, "%p", vol);
	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNewStatic(box, 0, "Line: ");

		sb = AG_SpinbuttonNew(box, 0, "X1:");
		AG_WidgetBindInt(sb, "value", &x1);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);
		
		sb = AG_SpinbuttonNew(box, 0, "Y1:");
		AG_WidgetBindInt(sb, "value", &y_1);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);

		sb = AG_SpinbuttonNew(box, 0, "Z1:");
		AG_WidgetBindInt(sb, "value", &z1);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);
		
		sb = AG_SpinbuttonNew(box, 0, "X2:");
		AG_WidgetBindInt(sb, "value", &x2);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);
		
		sb = AG_SpinbuttonNew(box, 0, "Y2:");
		AG_WidgetBindInt(sb, "value", &y2);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);

		sb = AG_SpinbuttonNew(box, 0, "Z2:");
		AG_WidgetBindInt(sb, "value", &z2);
		AG_SetEvent(sb, "spinbutton-changed", DoTraceLine, "%p", vol);

		AG_ButtonAct(box, AG_BUTTON_HFILL, "OK", DoTraceLine, "%p",
		    vol);
	}

	AG_WindowShow(win);
}

void
VoxelPathView(SG *sg, SG_Voxel *vol)
{
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Trajectory");
	SG_ViewNew(win, sg, SG_VIEW_EXPAND);
	AG_WindowSetGeometry(win, 0, 0, agView->w, agView->h-300);
	AG_WindowShow(win);
}
