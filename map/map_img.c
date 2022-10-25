/*
 * Copyright (c) 2021 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Graphical tile. References a rectangle in an image file.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>

#include <agar/map/map.h>

/*
 * Create a new tile from a specified RG_Tile (by id).
 * Size the source rectangle to fit.
 */
MAP_Img *
MAP_ImgNew(MAP *map, MAP_Node *node, Uint idx)
{
	MAP_Img *img;

	img = Malloc(sizeof(MAP_Img));
	MAP_ItemInit(img, MAP_ITEM_IMG);
	img->idx = idx;
	TAILQ_INSERT_TAIL(&node->items, MAPITEM(img), items);
	return (img);
}

static void
Init(void *mi)
{
	MAP_Img *img = MAPIMG(mi);

	memset(&img->idx, 0, sizeof(Uint) +           /* idx */
	                     sizeof(int) +            /* xCenter */
	                     sizeof(int) +            /* yCenter */
	                     sizeof(int) +            /* xMotion */
	                     sizeof(int) +            /* yMotion */
	                     sizeof(AG_Rect));        /* rs */
}

static void *
Duplicate(MAP *map, MAP_Node *node, const void *mi)
{
	const MAP_Img *img = MAPIMG(mi);
	MAP_Img *imgNew;

	imgNew = MAP_ImgNew(map, node, img->idx);
	memcpy(&imgNew->xCenter, &img->xCenter, sizeof(int) +     /* xCenter */
	                                        sizeof(int) +     /* yCenter */
	                                        sizeof(int) +     /* xMotion */
	                                        sizeof(int) +     /* yMotion */
	                                        sizeof(AG_Rect)); /* rs */
	return (void *)(imgNew);
}

static int
Load(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Img *img = MAPIMG(mi);

	img->idx = (Uint)AG_ReadUint32(ds);

	img->xCenter = (int)AG_ReadSint16(ds);
	img->yCenter = (int)AG_ReadSint16(ds);
	img->xMotion = (int)AG_ReadSint16(ds);
	img->yMotion = (int)AG_ReadSint16(ds);

	img->rs.x = (int)AG_ReadSint16(ds);
	img->rs.y = (int)AG_ReadSint16(ds);
	img->rs.w = (Uint)AG_ReadUint16(ds);
	img->rs.h = (Uint)AG_ReadUint16(ds);

	return (0);
}

static void
Save(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Img *img = MAPIMG(mi);

	AG_WriteUint32(ds, img->idx);

	AG_WriteSint16(ds, (Sint16)img->xCenter);
	AG_WriteSint16(ds, (Sint16)img->yCenter);
	AG_WriteSint16(ds, (Sint16)img->xMotion);
	AG_WriteSint16(ds, (Sint16)img->yMotion);

	AG_WriteSint16(ds, (Sint16)img->rs.x);
	AG_WriteSint16(ds, (Sint16)img->rs.y);
	AG_WriteUint16(ds, (Uint16)img->rs.w);
	AG_WriteUint16(ds, (Uint16)img->rs.h);
}

static void
Draw(MAP_View *_Nonnull mv, MAP_Item *_Nonnull mi, int rx, int ry, int ncam)
{
#ifdef AG_DEBUG
	MAP_Img *img = MAPIMG(mi);

	Debug(mv, "Draw image %u at %d,%d cam %d\n", img->idx, rx,ry, ncam);
#endif
}

static int
Extent(MAP *map, void *mi, AG_Rect *rd, int ncam)
{
	/* TODO */
	rd->x = 0;
	rd->y = 0;
	rd->w = 0;
	rd->h = 0;
	return (0);
}

static int
Collide(void *_Nonnull mi, int x, int y)
{
	/* TODO */
	return (0);
}

MAP_ItemClass mapImgClass = {
	N_("Image"),
	"https://libagar.org/",
	N_("Image file fragment"),
	MAP_ITEM_IMG,
	0,
	sizeof(MAP_Img),
	Init,
	NULL,		/* destroy */
	Duplicate,
	Load,
	Save,
	Draw,
	Extent,
	Collide,
	NULL		/* edit */
};
