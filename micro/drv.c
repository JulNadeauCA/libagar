/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Base Micro-Agar Driver Object.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/micro/window.h>

#ifdef __BBC__
extern MA_DriverClass maDriverBBC;
#endif
#ifdef __C64__
extern MA_DriverClass maDriverC64;
#endif
#ifdef __NES__
extern MA_DriverClass maDriverNES;
#endif
extern MA_DriverClass maDriverDUMMY;

MA_Driver      *maDriver = NULL;		/* Current driver instance */
MA_DriverClass *maDriverOps = NULL;		/* Current driver class */

/*
 * Available driver classes
 */
MA_DriverClass *maDriverList[] = {
#ifdef __BBC__
	&maDriverBBC,				/* Acorn BBC Micro */
#endif
#ifdef __C64__
	&maDriverC64,				/* Commodore 64 */
#endif
#ifdef __C128__
	&maDriverC128,				/* C128 */
#endif
#ifdef __GEOS__
	&maDriverGEOS,				/* GEOS */
#endif
#ifdef __NES__
	&maDriverNES,				/* Nintendo */
#endif
	&maDriverDUMMY,				/* Dummy driver */
	NULL
};

/* Create a new driver instance. */
MA_Driver *
MA_DriverOpen(MA_DriverClass *dc)
{
	if (maDriver != NULL) {
		AG_FatalError("Driver exists");
	}
	if ((maDriver = AG_ObjectNew(NULL, dc->name, AGCLASS(dc))) == NULL) {
		return (NULL);
	}
	if (dc->open(NULL) == -1) {
		AG_ObjectDestroy(maDriver);
		return (NULL);
	}
	AG_ObjectSetNameS(maDriver, dc->name);
	return (maDriver);
}

/* Close and destroy a driver. */
void
MA_DriverClose(MA_Driver *drv)
{
	AG_ObjectDetach(drv);
	MADRIVER_CLASS(drv)->close();
	AG_ObjectDestroy(drv);

	maDriver = NULL;
}

/* Return the resolution (px) of the parent display device, if applicable. */
void
AG_GetDisplaySize(Uint16 *w, Uint16 *h)
{
	/* TODO */
	*w = 320;
	*h = 240;
}

static void
Init(void *_Nonnull obj)
{
	MA_Driver *drv = obj;

	drv->flags = 0;
	drv->nSprites = 0;
}

AG_ObjectClass maDriverClass = {
	"MA_Driver",
	sizeof(MA_Driver),
	{ 1,6 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
