/*
 * Copyright (c) 2009-2013 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Multiple-window graphics driver framework. Under this framework, each
 * Agar window has a corresponding "native" window managed by the driver.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

AG_DriverMw *agDriverMw = NULL;		/* Root driver instance */
AG_List     *agModalWindows = NULL;	/* Modal window stack */
int          agModalWindowsRefs = 0;

static void
Init(void *obj)
{
	AG_DriverMw *dmw = obj;

	dmw->flags = 0;
	dmw->win = NULL;

	if (agModalWindowsRefs++ == 0 &&
	    (agModalWindows = AG_ListNew()) == NULL)
		AG_FatalError(NULL);
}

static void
Destroy(void *obj)
{
	if (--agModalWindowsRefs == 0) {
		AG_ListDestroy(agModalWindows);
		agModalWindows = NULL;
	}
}

AG_ObjectClass agDriverMwClass = {
	"AG_Driver:AG_DriverMw",
	sizeof(AG_DriverMw),
	{ 1,4 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
