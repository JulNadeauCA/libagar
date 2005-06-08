/*	$Csoft: feature.c,v 1.10 2005/04/14 06:19:44 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>

#include "tileset.h"
#include "tileview.h"

void
AG_FeatureInit(void *p, RG_Tileset *ts, int flags,
    const RG_FeatureOps *ops)
{
	RG_Feature *ft = p;
	RG_Feature *oft;
	u_int featno = 0;
	
tryname:
	snprintf(ft->name, sizeof(ft->name), "%s #%d", ops->type, featno);
	TAILQ_FOREACH(oft, &ts->features, features) {
		if (strcmp(oft->name, ft->name) == 0)
			break;
	}
	if (oft != NULL) {
		featno++;
		goto tryname;
	}
	ft->ts = ts;
	ft->flags = flags;
	ft->ops = ops;
	ft->nrefs = 0;
	TAILQ_INIT(&ft->sketches);
	TAILQ_INIT(&ft->pixmaps);
}

RG_FeatureSketch *
RG_FeatureAddSketch(RG_Feature *ft, RG_Sketch *sk)
{
	RG_FeatureSketch *fsk;

	fsk = Malloc(sizeof(RG_FeatureSketch), M_RG);
	fsk->sk = sk;
	fsk->x = 0;
	fsk->y = 0;
	fsk->visible = 1;
	TAILQ_INSERT_TAIL(&ft->sketches, fsk, sketches);
	return (fsk);
}

void
RG_FeatureDelSketch(RG_Feature *ft, RG_Sketch *sk)
{
	RG_FeatureSketch *fsk;

	TAILQ_FOREACH(fsk, &ft->sketches, sketches) {
		if (fsk->sk == sk)
			break;
	}
	if (fsk != NULL) {
		TAILQ_REMOVE(&ft->sketches, fsk, sketches);
		Free(fsk, M_RG);
	}
}

RG_FeaturePixmap *
RG_FeatureAddPixmap(RG_Feature *ft, RG_Pixmap *px)
{
	RG_FeaturePixmap *fpx;

	fpx = Malloc(sizeof(RG_FeaturePixmap), M_RG);
	fpx->px = px;
	fpx->x = 0;
	fpx->y = 0;
	fpx->visible = 1;
	TAILQ_INSERT_TAIL(&ft->pixmaps, fpx, pixmaps);
	return (fpx);
}

void
RG_FeatureDelPixmap(RG_Feature *ft, RG_Pixmap *px)
{
	RG_FeaturePixmap *fpx;

	TAILQ_FOREACH(fpx, &ft->pixmaps, pixmaps) {
		if (fpx->px == px)
			break;
	}
	if (fpx != NULL) {
		TAILQ_REMOVE(&ft->pixmaps, fpx, pixmaps);
		Free(fpx, M_RG);
	}
}

void
AG_FeatureDestroy(RG_Feature *ft)
{
	RG_FeatureSketch *fsk, *nfsk;
	RG_FeaturePixmap *fpx, *nfpx;

#ifdef DEBUG
	if (ft->nrefs > 0)
		dprintf("%s is referenced\n", ft->name);
#endif

	if (ft->ops->destroy != NULL)
		ft->ops->destroy(ft);

	for (fsk = TAILQ_FIRST(&ft->sketches);
	     fsk != TAILQ_END(&ft->sketches);
	     fsk = nfsk) {
		nfsk = TAILQ_NEXT(fsk, sketches);
		Free(fsk, M_RG);
	}
	
	for (fpx = TAILQ_FIRST(&ft->pixmaps);
	     fpx != TAILQ_END(&ft->pixmaps);
	     fpx = nfpx) {
		nfpx = TAILQ_NEXT(fpx, pixmaps);
		Free(fpx, M_RG);
	}
}

int
RG_FeatureLoad(void *p, AG_Netbuf *buf)
{
	RG_Feature *ft = p;

	if (ft->ops->load != NULL &&
	    ft->ops->load(ft, buf) == -1) {
		return (-1);
	}
	return (0);
}

void
RG_FeatureSave(void *p, AG_Netbuf *buf)
{
	RG_Feature *ft = p;

	ft->ops->save(ft, buf);
}

void
RG_FeatureOpenMenu(RG_Tileview *tv, int x, int y)
{
	RG_Feature *ft = tv->tv_feature.ft;
	
	if (tv->tv_feature.menu != NULL)
		RG_FeatureCloseMenu(tv);

	if (ft->ops->menu == NULL)
		return;

	tv->tv_feature.menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(tv->tv_feature.menu);

	tv->tv_feature.menu_item = AG_MenuAddItem(tv->tv_feature.menu, NULL);
	tv->tv_feature.menu->sel_item = tv->tv_feature.menu_item;

	ft->ops->menu(ft, tv->tv_feature.menu_item);

	tv->tv_feature.menu_win = AG_MenuExpand(tv->tv_feature.menu,
	    tv->tv_feature.menu_item, x, y);
}

void
RG_FeatureCloseMenu(RG_Tileview *tv)
{
	AG_Menu *menu = tv->tv_feature.menu;
	AG_MenuItem *item = tv->tv_feature.menu_item;

	AG_MenuCollapse(menu, item);
	AG_ObjectDestroy(menu);
	Free(menu, M_OBJECT);

	tv->tv_feature.menu = NULL;
	tv->tv_feature.menu_item = NULL;
	tv->tv_feature.menu_win = NULL;
}

