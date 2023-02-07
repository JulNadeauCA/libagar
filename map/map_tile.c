/*
 * Copyright (c) 2021-2022 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Graphical tile. References an RG_Tile within an RG_Tileset.
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
MAP_Tile *
MAP_TileNew(MAP *map, MAP_Node *node, RG_Tileset *ts, Uint id)
{
	MAP_Tile *mt;

	mt = Malloc(sizeof(MAP_Tile));
	MAP_ItemInit(mt, MAP_ITEM_TILE);
	MAP_TileSet(mt, map, ts, id);
	TAILQ_INSERT_TAIL(&node->items, MAPITEM(mt), items);
	return (mt);
}

/*
 * Set or update the source RG_Tile of an existing MAP_Tile item.
 * The source rectangle is initialized to [0,0]:[w,h] of source tile.
 * The referenced tileset is paged in if needed.
 */
void
MAP_TileSet(MAP_Tile *mt, MAP *map, RG_Tileset *ts, Uint id)
{
	RG_Tile *tile;

	if (mt->obj != ts && mt->obj != NULL) {
		MAP_PageOut(map, "TS-", mt->obj);
	} else {
		MAP_PageIn(map, "TS-", ts);
	}
	mt->obj = ts;
	mt->id = id;
	mt->rs.x = 0;
	mt->rs.y = 0;

	if (ts != NULL &&
	    RG_LookupTile(ts,id, &tile) == 0 && tile->su != NULL) {
		mt->rs.w = tile->su->w;
		mt->rs.h = tile->su->h;
	}
}

static void
Init(void *mi)
{
	MAP_Tile *mt = MAPTILE(mi);

	memset(&mt->obj, 0, sizeof(RG_Tileset *) +   /* obj */
	                    sizeof(Uint) +           /* id */
	                    sizeof(int) +            /* xCenter */
	                    sizeof(int) +            /* yCenter */
	                    sizeof(int) +            /* xMotion */
	                    sizeof(int) +            /* yMotion */
	                    sizeof(AG_Rect));        /* rs */
}

static void *
Duplicate(MAP *map, MAP_Node *node, const void *mi)
{
	const MAP_Tile *mt = MAPTILE(mi);

	return (void *)MAP_TileNew(map, node, mt->obj, mt->id);
}

static int
Load(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Tile *mt = MAPTILE(mi);
	char tsName[AG_OBJECT_NAME_MAX];

	AG_CopyString(tsName, ds, sizeof(tsName));
	mt->id = (Uint)AG_ReadUint32(ds);
	mt->obj = AG_ObjectFindChild(OBJECT(map)->root, tsName);
	if (mt->obj == NULL || !AG_OfClass(mt->obj, "RG_Tileset:*")) {
		AG_SetError("No such tileset \"%s\"", tsName);
		mt->id = 0;
		mt->obj = NULL;
		return (-1);
	}
	mt->xCenter = (int)AG_ReadSint16(ds);
	mt->yCenter = (int)AG_ReadSint16(ds);
	mt->xMotion = (int)AG_ReadSint16(ds);
	mt->yMotion = (int)AG_ReadSint16(ds);

	mt->rs.x = AG_ReadSint16(ds);
	mt->rs.y = AG_ReadSint16(ds);
	mt->rs.w = AG_ReadUint16(ds);
	mt->rs.h = AG_ReadUint16(ds);

	return (0);
}

static void
Save(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Tile *mt = MAPTILE(mi);

	AG_WriteString(ds, OBJECT(mt->obj)->name);
	AG_WriteUint32(ds, mt->id);

	AG_WriteSint16(ds, (Sint16)mt->xCenter);
	AG_WriteSint16(ds, (Sint16)mt->yCenter);
	AG_WriteSint16(ds, (Sint16)mt->xMotion);
	AG_WriteSint16(ds, (Sint16)mt->yMotion);

	AG_WriteSint16(ds, (Sint16)mt->rs.x);
	AG_WriteSint16(ds, (Sint16)mt->rs.y);
	AG_WriteUint16(ds, (Uint16)mt->rs.w);
	AG_WriteUint16(ds, (Uint16)mt->rs.h);
}

static void
Draw(MAP_View *_Nonnull mv, MAP_Item *_Nonnull mi, int rx, int ry, int ncam)
{
	MAP_Tile *mt = MAPTILE(mi);
	RG_Tile *tile;
	RG_TileVariant *var;
	RG_Transform *xf, *xf2;
//	const int tileSz = mv->map->cameras[ncam].tilesz;

	if (RG_LookupTile(mt->obj, mt->id, &tile) != 0) {
		Debug(mv, "Tile %s:%u not found\n",
		    (mt->obj != NULL) ? OBJECT(mt->obj)->name : "NULL",
		    mt->id);
		return;
	}
	Debug(mv, "Draw %s:%u @ %d,%d cam %d\n",
	    OBJECT(mt->obj)->name, mt->id, rx,ry, ncam);

	SLIST_FOREACH(var, &tile->vars, vars) {
		if (var->view != mv) {
			continue;
		}
		for (xf = TAILQ_FIRST(&mi->transforms), xf2 = TAILQ_FIRST(&var->transforms);
		     xf != TAILQ_END(&mi->transforms) && xf2 != TAILQ_END(&var->transforms);
		     xf = TAILQ_NEXT(xf, transforms), xf2 = TAILQ_NEXT(xf2, transforms)) {
			if (!RG_TransformCompare(xf, xf2))
				break;
		}
		if (xf == TAILQ_END(&mi->transforms) &&
		    xf2 == TAILQ_END(&var->transforms))
			break;
	}
	if (var == NULL) {
		RG_Transform *xf;
		const AG_Surface *Stile = tile->su;
		AG_Surface *Sx;
		
		var = Malloc(sizeof(RG_TileVariant));
		TAILQ_INIT(&var->transforms);
		RG_TransformChainDup(&mi->transforms, &var->transforms);
		var->surface = AG_SurfaceRGBA(
		    Stile->w, Stile->h, Stile->format.BitsPerPixel,
		    Stile->flags & AG_SURFACE_COLORKEY,
		    Stile->format.Rmask,
		    Stile->format.Gmask,
		    Stile->format.Bmask,
		    Stile->format.Amask);
		if (var->surface == NULL) {
			AG_FatalError(NULL);
		}
		var->view = mv;
		AG_SurfaceCopy(var->surface, Stile);

		TAILQ_FOREACH(xf, &mi->transforms, transforms) {
			Sx = xf->func(var->surface, xf->nArgs, xf->args);
			if (Sx != var->surface) {
				AG_SurfaceFree(var->surface);
				var->surface = Sx;
			}
		}

		var->texture = AG_WidgetMapSurface(mv, var->surface);

		SLIST_INSERT_HEAD(&tile->vars, var, vars);

		Debug(mv, "Draw %s:%u @ %d,%d: New variant %p\n",
		    OBJECT(mt->obj)->name, mt->id, rx,ry, var);
	} else {
		Debug(mv, "Draw %s:%u @ %d,%d: Found variant %p (last drawn %u)\n",
		    OBJECT(mt->obj)->name, mt->id, rx,ry, var, var->last_drawn);
	}
	var->last_drawn = AG_GetTicks();

#if 0
	if (tileSz != MAP_TILESZ_DEF) {
		x = rx + mi->data.tile.xCenter * tileSz / MAP_TILESZ_DEF +
		         mi->data.tile.xMotion * tileSz / MAP_TILESZ_DEF;

		y = ry + mi->data.tile.yCenter * tileSz / MAP_TILESZ_DEF +
		         mi->data.tile.yMotion * tileSz / MAP_TILESZ_DEF;

		BlitSurfaceScaled(m, su, &mi->data.tile.rs, x,y, cam);
	} else {
		x = rx + mi->data.tile.xCenter + mi->data.tile.xMotion;
		y = ry + mi->data.tile.yCenter + mi->data.tile.yMotion;

		AG_WidgetBlit(mv, su, x,y);
		AG_SurfaceBlit(su,
		    debug_su ? NULL : &mi->data.tile.rs,
		    agView->v, x,y);
	}
#endif
}

static int
Extent(MAP *map, void *mi, AG_Rect *rd, int ncam)
{
	MAP_Tile *mt = MAPTILE(mi);
	const int tileSz = map->cameras[ncam].tilesz;

//	if (RG_LookupTile(mt->obj, mt->id, NULL) == -1) {
//		return (-1);
//	}
	if (tileSz != MAP_TILESZ_DEF) {
		rd->x = mt->xCenter * tileSz / MAP_TILESZ_DEF +
		        mt->xMotion * tileSz / MAP_TILESZ_DEF;
		rd->y = mt->yCenter * tileSz / MAP_TILESZ_DEF +
		        mt->yMotion * tileSz / MAP_TILESZ_DEF;
		rd->w = mt->rs.w * tileSz / MAP_TILESZ_DEF;
		rd->h = mt->rs.h * tileSz / MAP_TILESZ_DEF;
	} else {
		rd->x = mt->xCenter + mt->xMotion;
		rd->y = mt->yCenter + mt->yMotion;
		rd->w = mt->rs.w;
		rd->h = mt->rs.h;
	}
	return (0);
}

static int
Collide(void *_Nonnull mi, int x, int y)
{
	/* TODO */
	return (0);
}

MAP_ItemClass mapTileClass = {
	N_("Tile"),
	"https://libagar.org/",
	N_("Graphical tile"),
	MAP_ITEM_TILE,
	0,
	sizeof(MAP_Tile),
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
