/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic input device object. This is the parent class to AG_Mouse and
 * AG_Keyboard.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

AG_Object agInputDevices;		/* Input devices VFS */

static void
Init(void *obj)
{
	AG_InputDevice *id = obj;

	id->flags = 0;
	id->drv = NULL;
	id->desc = NULL;
	AG_InitEventQ(&id->events);
}

static void
Destroy(void *obj)
{
	AG_InputDevice *id = obj;

	Free(id->desc);
	AG_FreeEventQ(&id->events);
}

AG_ObjectClass agInputDeviceClass = {
	"AG_InputDevice",
	sizeof(AG_InputDevice),
	{ 0,0 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
