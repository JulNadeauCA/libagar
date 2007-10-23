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

#include <core/load_den.h>
#include <core/load_xcf.h>

#include <string.h>
#include <stdarg.h>

#include "iconmgr.h"
#include "view.h"

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

static void
LoadIconRaw(SDL_Surface *su, const Uint32 *data)
{
	const Uint32 *src = &data[0];
	Uint8 *dst = su->pixels;
	int x, y;
	
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			AG_PutPixel(su, dst, *src);
			dst += su->format->BytesPerPixel;
			src++;
		}
	}
}

static void
LoadIconConvert(SDL_Surface *su, const Uint32 *data)
{
	const Uint32 *src = &data[0];
	Uint8 *dst = su->pixels;
	int x, y;
	
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			AG_PutPixel(su, dst, AG_SurfacePixel(*src));
			dst += su->format->BytesPerPixel;
			src++;
		}
	}
}

/* Load a set of icons from agarpaint-generated .h data. */
void
AG_LoadIcons(AG_IconMgr *im, const Uint32 **icons, Uint count)
{
	SDL_Surface *su;
	Uint32 Rmask, Gmask, Bmask, Amask;
	Uint i;

	dprintf("loading %u static icons\n", count);
	for (i = 0; i < count; i++) {
		const Uint32 *data = icons[i];

		su = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
		    data[0], data[1], agSurfaceFmt->BitsPerPixel,
		    agSurfaceFmt->Rmask, agSurfaceFmt->Gmask,
		    agSurfaceFmt->Bmask, agSurfaceFmt->Amask);
		if (su == NULL) {
			AG_FatalError("SDL_CreateRGBSurface: %s",
			    SDL_GetError());
		}
		Rmask = data[2];
		Gmask = data[3];
		Bmask = data[4];
		Amask = data[5];

		if (Rmask == agSurfaceFmt->Rmask &&
		    Gmask == agSurfaceFmt->Gmask &&
		    Bmask == agSurfaceFmt->Bmask &&
		    Amask == agSurfaceFmt->Amask) {
			dprintf("load %dx%d icon raw\n", su->w, su->h);
			LoadIconRaw(su, &data[6]);
		} else {
			dprintf("load %dx%d icon converted\n", su->w, su->h);
			LoadIconConvert(su, &data[6]);
		}
	}
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
	Uint i;

	for (i = 0; i < im->nicons; i++) {
		SDL_FreeSurface(im->icons[i]);
	}
	Free(im->icons, M_GFX);
}

/* Return the icon associated with an object class, if any. */
SDL_Surface *
AG_ObjectIcon(void *p)
{
	/* TODO */
	return (NULL);
}
