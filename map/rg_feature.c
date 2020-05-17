/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/window.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>

#include <string.h>

void
RG_FeatureInit(void *p, RG_Tileset *ts, int flags, const RG_FeatureOps *ops)
{
	RG_Feature *ft = p;
	RG_Feature *oft;
	Uint featno = 0;
	
tryname:
	Snprintf(ft->name, sizeof(ft->name), "%s #%d", ops->type, featno);
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
	ft->nRefs = 0;
/*	TAILQ_INIT(&ft->sketches); */
	TAILQ_INIT(&ft->pixmaps);
}

#if 0
RG_FeatureSketch *
RG_FeatureAddSketch(RG_Feature *ft, RG_Sketch *sk)
{
	RG_FeatureSketch *fsk;

	fsk = Malloc(sizeof(RG_FeatureSketch));
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
		Free(fsk);
	}
}
#endif

RG_FeaturePixmap *
RG_FeatureAddPixmap(RG_Feature *ft, RG_Pixmap *px)
{
	RG_FeaturePixmap *fpx;

	fpx = Malloc(sizeof(RG_FeaturePixmap));
	fpx->flags = 0;
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
		Free(fpx);
	}
}

void
RG_FeatureDestroy(RG_Feature *ft)
{
/*	RG_FeatureSketch *fsk, *nfsk; */
	RG_FeaturePixmap *fpx, *nfpx;

	if (ft->ops->destroy != NULL)
		ft->ops->destroy(ft);
#if 0
	for (fsk = TAILQ_FIRST(&ft->sketches);
	     fsk != TAILQ_END(&ft->sketches);
	     fsk = nfsk) {
		nfsk = TAILQ_NEXT(fsk, sketches);
		Free(fsk);
	}
#endif	
	for (fpx = TAILQ_FIRST(&ft->pixmaps);
	     fpx != TAILQ_END(&ft->pixmaps);
	     fpx = nfpx) {
		nfpx = TAILQ_NEXT(fpx, pixmaps);
		Free(fpx);
	}
}

int
RG_FeatureLoad(void *p, AG_DataSource *buf)
{
	RG_Feature *ft = p;

	if (ft->ops->load != NULL &&
	    ft->ops->load(ft, buf) == -1) {
		return (-1);
	}
	return (0);
}

void
RG_FeatureSave(void *p, AG_DataSource *buf)
{
	RG_Feature *ft = p;

	ft->ops->save(ft, buf);
}

void
RG_FeatureOpenMenu(RG_Tileview *tv, int x, int y)
{
	RG_Feature *ft = tv->tv_feature.ft;
	
	if (tv->tv_feature.menu != NULL) {
		AG_PopupShowAt(tv->tv_feature.menu, x,y);
		return;
	}
	if (ft->ops->menu == NULL) {
		return;
	}
	tv->tv_feature.menu = AG_PopupNew(tv);
	ft->ops->menu(ft, tv->tv_feature.menu->root);
	AG_PopupShowAt(tv->tv_feature.menu, x,y);
}

void
RG_FeatureCloseMenu(RG_Tileview *tv)
{
	if (tv->tv_feature.menu != NULL) {
		AG_PopupDestroy(tv->tv_feature.menu);
		tv->tv_feature.menu = NULL;
	}
}

