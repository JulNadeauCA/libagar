/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Interface to ODE (Open Dynamics Engine).
 */

#include <config/have_opengl.h>
#include <config/have_ode.h>
#if defined(HAVE_OPENGL) && defined(HAVE_ODE)

#include <core/core.h>
#include <core/util.h>
#include <gui/window.h>
#include <sg/sg.h>
#include <sg/sg_gui.h>

#include "pe_ode.h"

static void
Init(void *obj, const char *name)
{
	PE_Ode *pe = obj;

	PE_Init(pe, name);
	AG_ObjectSetOps(pe, &peOps);
}

static void
Destroy(void *obj)
{
	PE_Ode *pe = obj;
}

static int
Load(void *obj, AG_Netbuf *buf)
{
	PE_Ode *pe = obj;

	if (AG_ReadObjectVersion(buf, pe, NULL) != 0) {
		return (-1);
	}
	return (0);
}

static int
Save(void *obj, AG_Netbuf *buf)
{
	PE_Ode *pe = obj;

	AG_WriteObjectVersion(buf, pe);
	return (0);
}

static void *
Edit(void *obj)
{
	PE_Ode *pe = obj;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("ODE Physics Engine: <%s>"),
	    OBJECT(pe)->name);

	return (win);
}

static int
AttachObject(void *obj, SG_Object *so)
{
	return (0);
}

static void
DetachObject(void *obj, SG_Object *so)
{
}

const PE_Ops peOdeOps = {
	{
		"PE:PE_Ode",
		sizeof(PE_Ode),
		{ 0,0 },
		Init,
		NULL,			/* reinit */
		Destroy,
		Load,
		Save,
		Edit
	},
	AttachObject,
	DetachObject,
};

#endif /* HAVE_OPENGL and HAVE_ODE */
