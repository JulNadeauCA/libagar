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

/*
 * Icon resource object.
 */

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>

#include <core/load_den.h>
#include <core/load_xcf.h>

#include <string.h>
#include <stdarg.h>

AG_IconMgr agIconMgr;

const AG_ObjectOps agIconMgrOps = {
	"AG_IconMgr",
	sizeof(AG_IconMgr),
	{ 0, 0 },
	AG_IconMgrInit,
	NULL,
	AG_IconMgrDestroy,
	NULL,
	NULL,
	NULL
};

void
AG_IconMgrInit(void *p, const char *name)
{
	AG_IconMgr *im = p;
	int i;

	AG_ObjectInit(im, name, &agIconMgrOps);
	im->icons = Malloc(sizeof(SDL_Surface *), M_GFX);
	im->nicons = 0;
}

static void
AG_IconMgrLoadIcon(SDL_Surface *su, const char *lbl, void *p)
{
	AG_IconMgr *im = p;

	im->icons = Realloc(im->icons, (im->nicons+1)*sizeof(SDL_Surface *));
	im->icons[im->nicons++] = su;
}

/*
 * Load a set of icon from XCF data contained within a .den file located
 * in the standard den-path.
 */
int
AG_IconMgrLoadFromDenXCF(AG_IconMgr *im, const char *name)
{
	char path[MAXPATHLEN];
	AG_Den *den;
	Uint32 i;

	if (AG_ConfigFile("den-path", name, "den", path, sizeof(path)) == -1 ||
	    (den = AG_DenOpen(path, AG_DEN_READ)) == NULL)
		return (-1);
	
	for (i = 0; i < den->nmembers; i++) {
		if (AG_XCFLoad(den->buf, den->members[i].offs,
		    AG_IconMgrLoadIcon, im) == -1) {
			goto fail;
		}
	}
	dprintf("%s: loaded %u icons\n", name, (Uint)i);

	AG_DenClose(den);
	return (0);
fail:
	AG_DenClose(den);
	return (-1);
}

void
AG_IconMgrDestroy(void *p)
{
	AG_IconMgr *im = p;
	int i;

	for (i = 0; i < im->nicons; i++) {
		SDL_FreeSurface(im->icons[i]);
	}
	Free(im->icons, M_GFX);
}
