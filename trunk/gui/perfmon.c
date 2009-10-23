/*
 * Copyright (c) 2001-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * "Performance monitor" tool. This gathers various statistics from the
 * event loop, such as the video refresh rate, the events rate and the
 * average idle times.
 */

#include <config/ag_debug.h>
#ifdef AG_DEBUG

#include <core/core.h>
#include <core/config.h>

#include "window.h"
#include "label.h"
#include "fixed_plotter.h"

#include "perfmon.h"

int agEventAvg = 0;		/* Number of events in last frame */
int agIdleAvg = 0;		/* Measured AG_Delay() granularity */
AG_Window *agPerfWindow = NULL;

static AG_FixedPlotter *agPerfGraph;
static AG_FixedPlotterItem *agPerfFPS, *agPerfEvnts, *agPerfIdle;

/* Show the performance monitor graph. */ 
AG_Window *
AG_PerfMonShow(void)
{
	AG_WindowShow(agPerfWindow);
	return (agPerfWindow);
}

/*
 * Update the performance counters.
 * XXX inefficient; remove this once the graph widget implements polling.
 */
void
AG_PerfMonUpdate(int rCur)
{
	static int einc = 0;

	AG_FixedPlotterDatum(agPerfFPS, rCur);
	AG_FixedPlotterDatum(agPerfEvnts, agEventAvg * 30 / 10);
	AG_FixedPlotterDatum(agPerfIdle, agIdleAvg);
	AG_FixedPlotterScroll(agPerfGraph, 1);

	if (++einc == 1) {
		agEventAvg = 0;
		einc = 0;
	}
}

/* Initialize the performance monitor window. */
void
AG_PerfMonInit(void)
{
	AG_Label *lbl;

	agPerfWindow = AG_WindowNewNamedS(0, "event-fps-counter");
	AG_WindowSetCaptionS(agPerfWindow, _("Performance counters"));
	AG_WindowSetPosition(agPerfWindow, AG_WINDOW_LOWER_CENTER, 0);
	lbl = AG_LabelNewPolled(agPerfWindow, AG_LABEL_HFILL,
	    "%d evnt, %dms idle", &agEventAvg, &agIdleAvg);
	AG_LabelSizeHint(lbl, 1, "00 evnt, 000ms idle");
	agPerfGraph = AG_FixedPlotterNew(agPerfWindow, AG_FIXED_PLOTTER_LINES,
	                                               AG_FIXED_PLOTTER_XAXIS|
						       AG_FIXED_PLOTTER_EXPAND);
	agPerfFPS = AG_FixedPlotterCurve(agPerfGraph, "refresh", 0,160,0, 99);
	agPerfEvnts = AG_FixedPlotterCurve(agPerfGraph, "event", 0,0,180, 99);
	agPerfIdle = AG_FixedPlotterCurve(agPerfGraph, "idle", 180,180,180, 99);
}

#endif /* AG_DEBUG */
