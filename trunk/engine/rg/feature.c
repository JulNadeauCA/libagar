/*	$Csoft: feature.c,v 1.7 2005/02/11 04:50:41 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>

#include "tileset.h"
#include "tileview.h"

void
feature_init(void *p, struct tileset *ts, int flags,
    const struct feature_ops *ops)
{
	struct feature *ft = p;
	struct feature *oft;
	unsigned int featno = 0;
	
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

struct feature_sketch *
feature_insert_sketch(struct feature *ft, struct sketch *sk)
{
	struct feature_sketch *fsk;

	fsk = Malloc(sizeof(struct feature_sketch), M_RG);
	fsk->sk = sk;
	fsk->x = 0;
	fsk->y = 0;
	fsk->visible = 1;
	TAILQ_INSERT_TAIL(&ft->sketches, fsk, sketches);
	return (fsk);
}

void
feature_remove_sketch(struct feature *ft, struct sketch *sk)
{
	struct feature_sketch *fsk;

	TAILQ_FOREACH(fsk, &ft->sketches, sketches) {
		if (fsk->sk == sk)
			break;
	}
	if (fsk != NULL) {
		TAILQ_REMOVE(&ft->sketches, fsk, sketches);
		Free(fsk, M_RG);
	}
}

struct feature_pixmap *
feature_insert_pixmap(struct feature *ft, struct pixmap *px)
{
	struct feature_pixmap *fpx;

	fpx = Malloc(sizeof(struct feature_pixmap), M_RG);
	fpx->px = px;
	fpx->x = 0;
	fpx->y = 0;
	fpx->visible = 1;
	TAILQ_INSERT_TAIL(&ft->pixmaps, fpx, pixmaps);
	return (fpx);
}

void
feature_remove_pixmap(struct feature *ft, struct pixmap *px)
{
	struct feature_pixmap *fpx;

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
feature_destroy(struct feature *ft)
{
	struct feature_sketch *fsk, *nfsk;
	struct feature_pixmap *fpx, *nfpx;

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
feature_load(void *p, struct netbuf *buf)
{
	struct feature *ft = p;

	if (ft->ops->load != NULL &&
	    ft->ops->load(ft, buf) == -1) {
		return (-1);
	}
	return (0);
}

void
feature_save(void *p, struct netbuf *buf)
{
	struct feature *ft = p;

	ft->ops->save(ft, buf);
}

void
feature_open_menu(struct tileview *tv, int x, int y)
{
	struct feature *ft = tv->tv_feature.ft;
	
	if (tv->tv_feature.menu != NULL)
		feature_close_menu(tv);

	if (ft->ops->menu == NULL)
		return;

	tv->tv_feature.menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(tv->tv_feature.menu);

	tv->tv_feature.menu_item = menu_add_item(tv->tv_feature.menu, NULL);
	tv->tv_feature.menu->sel_item = tv->tv_feature.menu_item;

	ft->ops->menu(ft, tv->tv_feature.menu_item);

	tv->tv_feature.menu_win = menu_expand(tv->tv_feature.menu,
	    tv->tv_feature.menu_item, x, y);
}

void
feature_close_menu(struct tileview *tv)
{
	struct AGMenu *menu = tv->tv_feature.menu;
	struct AGMenuItem *item = tv->tv_feature.menu_item;

	menu_collapse(menu, item);
	object_destroy(menu);
	Free(menu, M_OBJECT);

	tv->tv_feature.menu = NULL;
	tv->tv_feature.menu_item = NULL;
	tv->tv_feature.menu_win = NULL;
}

