/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/label.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/numerical.h>
#include <agar/gui/notebook.h>
#include <agar/gui/pane.h>
#include <agar/gui/separator.h>
#include <agar/gui/statusbar.h>
#include <agar/gui/tlist.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/map.h>
#include <agar/map/icons_data.h>
#include <agar/map/insert.h>

#include <string.h>
#include <stdlib.h>

int mapSmoothScaling = 0;

extern AG_ObjectClass mapActorClass;

void
MAP_InitSubsystem(void)
{
	AG_RegisterNamespace("MAP", "MAP_", "http://libagar.org/");
	AG_RegisterClass(&mapClass);
	AG_RegisterClass(&mapActorClass);
	AG_RegisterClass(&mapEditorClass);
	AG_RegisterClass(&mapEditorPseudoClass);
	AG_RegisterClass(&mapViewClass);

	mapIcon_Init();
}

void
MAP_DestroySubsystem(void)
{
	AG_UnregisterClass(&mapClass);
	AG_UnregisterClass(&mapActorClass);
	AG_UnregisterClass(&mapEditorClass);
	AG_UnregisterClass(&mapEditorPseudoClass);
	AG_UnregisterClass(&mapViewClass);
}

void
MAP_NodeInit(MAP_Node *node)
{
	TAILQ_INIT(&node->nrefs);
}

void
MAP_NodeDestroy(MAP *m, MAP_Node *node)
{
	MAP_Item *r, *nr;

	for (r = TAILQ_FIRST(&node->nrefs);
	     r != TAILQ_END(&node->nrefs);
	     r = nr) {
		nr = TAILQ_NEXT(r, nrefs);
		MAP_ItemDestroy(m, r);
		free(r);
	}
}

void
MAP_ItemInit(MAP_Item *r, enum map_item_type type)
{
	r->type = type;
	r->flags = 0;
	r->layer = 0;
	r->friction = 0;
	r->p = NULL;
	r->r_gfx.xcenter = 0;
	r->r_gfx.ycenter = 0;
	r->r_gfx.xmotion = 0;
	r->r_gfx.ymotion = 0;
	r->r_gfx.xorigin = 0;
	r->r_gfx.yorigin = 0;

	switch (type) {
	case MAP_ITEM_TILE:
		r->r_tile.obj = NULL;
		r->r_tile.id = 0;
		r->r_gfx.rs.x = 0;
		r->r_gfx.rs.y = 0;
		r->r_gfx.rs.w = 0;
		r->r_gfx.rs.h = 0;
		break;
	case MAP_ITEM_WARP:
		r->r_warp.map = NULL;
		r->r_warp.x = 0;
		r->r_warp.y = 0;
		r->r_warp.dir = 0;
		break;
	}

	RG_TransformChainInit(&r->transforms);
	TAILQ_INIT(&r->masks);
}

/*
 * Adjust the centering offset of a given node reference.
 * The parent map, if any, must be locked.
 */
void
MAP_ItemSetCenter(MAP_Item *r, int xcenter, int ycenter)
{
	r->r_gfx.xcenter = (Sint16)xcenter;
	r->r_gfx.ycenter = (Sint16)ycenter;
}

/*
 * Adjust the motion offset of a given node reference.
 * The parent map, if any, must be locked.
 */
void
MAP_ItemSetMotion(MAP_Item *r, int xmotion, int ymotion)
{
	r->r_gfx.xmotion = (Sint16)xmotion;
	r->r_gfx.ymotion = (Sint16)ymotion;
}

/*
 * Define the coefficient of friction/acceleration for a given node reference.
 * The parent map, if any, must be locked.
 */
void
MAP_ItemSetFriction(MAP_Item *r, int coeff)
{
	r->friction = (Sint8)coeff;
}

/* Set the layer attribute of a noderef. */
void
MAP_ItemSetLayer(MAP_Item *r, int layer)
{
	r->layer = layer;
}

void
MAP_ItemDestroy(MAP *m, MAP_Item *r)
{
	MAP_NodeMask *mask, *mask_next;
	
	RG_TransformChainDestroy(&r->transforms);

	for (mask = TAILQ_FIRST(&r->masks);
	     mask != TAILQ_END(&r->masks);
	     mask = mask_next) {
		mask_next = TAILQ_NEXT(mask, masks);
		MAP_NodeMaskDestroy(m, mask);
	}
	switch (r->type) {
	case MAP_ITEM_TILE:
		if (r->r_tile.obj != NULL) {
#ifdef AG_DEBUG
			if (!AG_OfClass(r->r_tile.obj, "RG_Tileset:*"))
				AG_FatalError(NULL);
#endif
			MAP_PageOut(m, "TS-", r->r_tile.obj);
		}
		break;
	case MAP_ITEM_WARP:
		free(r->r_warp.map);
		break;
	default:
		break;
	}
}

void
MAP_ItemAttrColor(Uint flag, int state, AG_Color *c)
{
	switch (flag) {
	case MAP_ITEM_BLOCK:
		if (state) {
			AG_ColorRGBA_8(c, 255,0,0,64);
		} else {
			AG_ColorRGBA_8(c, 0,255,0,32);
		}
		break;
	case MAP_ITEM_CLIMBABLE:
		if (state) {
			AG_ColorRGBA_8(c, 255,255,0,64);
		} else {
			AG_ColorRGBA_8(c, 255,0,0,32);
		}
		break;
	case MAP_ITEM_SLIPPERY:
		if (state) {
			AG_ColorRGBA_8(c, 0,0,255,64);
		} else {
			AG_ColorRGBA_8(c, 0,0,0,0);
		}
		break;
	case MAP_ITEM_JUMPABLE:
		if (state) {
			AG_ColorRGBA_8(c, 255,0,255,64);
		} else {
			AG_ColorRGBA_8(c, 0,0,0,0);
		}
		break;
	}
}

/* Allocate and initialize the node map. */
int
MAP_AllocNodes(MAP *m, Uint w, Uint h)
{
	Uint x, y;
	
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError(_("%ux%u nodes exceed %ux%u."), w, h,
		    MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	AG_ObjectLock(m);
	m->mapw = w;
	m->maph = h;
	m->map = Malloc(h*sizeof(MAP_Node *));
	for (y = 0; y < h; y++) {
		m->map[y] = Malloc(w*sizeof(MAP_Node));
		for (x = 0; x < w; x++) {
			MAP_NodeInit(&m->map[y][x]);
		}
	}
	AG_ObjectUnlock(m);
	return (0);
}

/* Release the node map. */
void
MAP_FreeNodes(MAP *m)
{
	Uint x, y;
	MAP_Node *node;

	AG_ObjectLock(m);
	if (m->map != NULL) {
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				node = &m->map[y][x];
				MAP_NodeDestroy(m, node);
			}
			free(m->map[y]);
		}
		free(m->map);
		m->map = NULL;
	}
	AG_ObjectUnlock(m);
}

static void
FreeLayers(MAP *_Nonnull m)
{
	m->layers = Realloc(m->layers, 1*sizeof(MAP_Layer));
	m->nLayers = 1;
	MAP_InitLayer(&m->layers[0], _("Layer 0"));
}

static void
FreeCameras(MAP *_Nonnull m)
{
	m->cameras = Realloc(m->cameras , 1*sizeof(MAP_Camera));
	m->nCameras = 1;
	MAP_InitCamera(&m->cameras[0], _("Camera 0"));
}

/* Resize a map, initializing new nodes and destroying any excess ones. */
int
MAP_Resize(MAP *m, Uint w, Uint h)
{
	MAP tm;
	Uint x, y;

	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError(_("%ux%u nodes exceed %ux%u."), w, h,
		    MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	AG_ObjectLock(m);

	/* Save the nodes to a temporary map, to preserve dependencies. */
	AG_ObjectInit(&tm, &mapClass);
	tm.flags |= AG_OBJECT_STATIC;

	if (MAP_AllocNodes(&tm, m->mapw, m->maph) == -1) {
		goto fail;
	}
	for (y = 0;
	     y < m->maph && y < h;
	     y++) {
		for (x = 0;
		     x < m->mapw && x < w;
		     x++) {
			MAP_NodeCopy(m, &m->map[y][x], -1, &tm, &tm.map[y][x],
			    -1);
		}
	}

	/* Resize the map, restore the original nodes. */
	MAP_FreeNodes(m);
	if (MAP_AllocNodes(m, w, h) == -1) {
		goto fail;
	}
	for (y = 0;
	     y < tm.maph && y < m->maph;
	     y++) {
		for (x = 0;
		     x < tm.mapw && x < m->mapw;
		     x++) {
			MAP_NodeCopy(&tm, &tm.map[y][x], -1, m, &m->map[y][x],
			    -1);
		}
	}

	if (m->origin.x >= (int)w) { m->origin.x = (int)w-1; }
	if (m->origin.y >= (int)h) { m->origin.y = (int)h-1; }

	AG_ObjectUnlock(m);
	AG_ObjectDestroy(&tm);
	return (0);
fail:
	AG_ObjectUnlock(m);
	AG_ObjectDestroy(&tm);
	return (-1);
}

/* Set the display scaling factor. */
void
MAP_SetZoom(MAP *m, int ncam, Uint zoom)
{
	MAP_Camera *cam = &m->cameras[ncam];

	AG_ObjectLock(m);
	cam->zoom = zoom;
	if ((cam->tilesz = cam->zoom*MAPTILESZ/100) > MAP_TILESZ_MAX) {
		cam->tilesz = MAP_TILESZ_MAX;
	}
	AG_ObjectUnlock(m);
}

MAP *
MAP_New(void *parent, const char *name)
{
	MAP *m;

	m = Malloc(sizeof(MAP));
	AG_ObjectInit(m, &mapClass);
	AG_ObjectSetNameS(m, name);
	AG_ObjectAttach(parent, m);
	return (m);
}

void
MAP_InitLayer(MAP_Layer *lay, const char *name)
{
	Strlcpy(lay->name, name, sizeof(lay->name));
	lay->visible = 1;
	lay->xinc = 1;
	lay->yinc = 1;
	lay->alpha = 255;
}

void
MAP_InitCamera(MAP_Camera *cam, const char *name)
{
	Strlcpy(cam->name, name, sizeof(cam->name));
	cam->x = 0;
	cam->y = 0;
	cam->flags = 0;
	cam->alignment = AG_MAP_CENTER;
	cam->zoom = 100;
	cam->tilesz = MAPTILESZ;
	cam->pixsz = 1;
}

int
MAP_AddCamera(MAP *m, const char *name)
{
	m->cameras = Realloc(m->cameras,
	    (m->nCameras+1)*sizeof(MAP_Camera));
	MAP_InitCamera(&m->cameras[m->nCameras], name);
	return (m->nCameras++);
}

/* Initialize undo block. */
static void
InitModBlk(MAP_ModBlk *_Nonnull blk)
{
	blk->mods = Malloc(sizeof(MAP_Mod));
	blk->nMods = 0;
	blk->cancel = 0;
}

/* Destroy undo block. */
static void
FreeModBlk(MAP *_Nonnull m, MAP_ModBlk *_Nonnull blk)
{
	Uint i;

	for (i = 0; i < blk->nMods; i++) {
		MAP_Mod *mm = &blk->mods[i];
	
		switch (mm->type) {
		case AG_MAPMOD_NODECHG:
			MAP_NodeDestroy(m, &mm->mm_nodechg.node);
			break;
		default:
			break;
		}
	}
	free(blk->mods);
}

void
MAP_InitModBlks(MAP *m)
{
	m->blks = Malloc(sizeof(MAP_ModBlk));
	m->nBlks = 1;
	InitModBlk(&m->blks[0]);
	m->curBlk = 0;
	m->nMods = 0;
	MAP_ModBegin(m);
}

static void
Init(void *_Nonnull obj)
{
	extern int mapEditorInited;			/* mapedit.c */
	extern int mapDefaultWidth;
	extern int mapDefaultHeight;
	MAP *m = obj;

	m->flags = 0;
	m->mapw = 0;
	m->maph = 0;
	m->cur_layer = 0;
	m->origin.x = 0;
	m->origin.y = 0;
	m->origin.layer = 0;
	m->redraw = 0;
	m->map = NULL;
	m->layers = Malloc(sizeof(MAP_Layer));
	m->nLayers = 1;
	m->cameras = Malloc(sizeof(MAP_Camera));
	m->nCameras = 1;
	TAILQ_INIT(&m->actors);
	
	MAP_InitLayer(&m->layers[0], _("Layer 0"));
	MAP_InitCamera(&m->cameras[0], _("Camera 0"));
	MAP_InitModBlks(m);

	if (!mapEditorInited) {
		mapEditorInited = 1;
		MAP_EditorInit();
	}
	MAP_AllocNodes(m, mapDefaultWidth, mapDefaultHeight);
	m->origin.x = mapDefaultWidth/2;
	m->origin.y = mapDefaultHeight/2;
}

/* Create a new layer. */
int
MAP_PushLayer(MAP *m, const char *name)
{
	char layname[MAP_LAYER_NAME_MAX];

	if (name[0] == '\0') {
		Snprintf(layname, sizeof(layname), _("Layer %u"), m->nLayers);
	} else {
		Strlcpy(layname, name, sizeof(layname));
	}

	if (m->nLayers+1 > MAP_LAYERS_MAX) {
		AG_SetErrorS(_("Too many layers."));
		return (-1);
	}
	m->layers = Realloc(m->layers, (m->nLayers+1)*sizeof(MAP_Layer));
	MAP_InitLayer(&m->layers[m->nLayers], layname);
	m->nLayers++;
	return (0);
}

/* Remove the last layer. */
void
MAP_PopLayer(MAP *m)
{
	if (--m->nLayers < 1)
		m->nLayers = 1;
}

/*
 * Set up a dependency between the map and an external Object
 * (or increment the reference count of existing dependency).
 */
void
MAP_PageIn(MAP *map, const char *resPrefix, void *resObj)
{
	char key[AG_VARIABLE_NAME_MAX];
	int nRefs;

	Strlcpy(key, resPrefix, sizeof(key));
	Strlcat(key, OBJECT(resObj)->name, sizeof(key));
	AG_BindObject(map,key, resObj);

	Strlcat(key, "-NREFS", sizeof(key));
	if (AG_Defined(map,key)) {
		if ((nRefs = AG_GetInt(map,key)) == 0) {
			if (AG_ObjectPageIn(resObj) == -1)
				AG_FatalError(NULL);
		}
		AG_SetInt(map,key, nRefs+1);
	} else {
		if (AG_ObjectPageIn(resObj) == -1) {
			AG_FatalError(NULL);
		}
		AG_SetInt(map,key, 1);
	}
}

/*
 * Decrement the reference count between the map and specified tileset.
 * Delete the reference if the reference count reaches 0.
 */
void
MAP_PageOut(MAP *map, const char *resPrefix, void *resObj)
{
	char key[AG_VARIABLE_NAME_MAX];
	char nrefkey[AG_VARIABLE_NAME_MAX];
	int nRefs;
	
	Strlcpy(key, resPrefix, sizeof(key));
	Strlcat(key, OBJECT(resObj)->name, sizeof(key));

	Strlcpy(nrefkey, key, sizeof(nrefkey));
	Strlcat(nrefkey, "-NREFS", sizeof(nrefkey));

	if (!AG_Defined(map,nrefkey)) {
		Debug(map, "No reference to %s\n", OBJECT(resObj)->name);
		return;
	}
	if ((nRefs = AG_GetInt(map,nrefkey)) > 0) {
		AG_SetInt(map,nrefkey, --nRefs);
	} else {
		AG_Unset(map,key);
		AG_Unset(map,nrefkey);
	}
}

/* Set or change the tile reference of a TILE item. */
void
MAP_ItemSetTile(MAP_Item *r, MAP *map, RG_Tileset *ts, Uint tile_id)
{
	RG_Tile *tile;

	if (r->r_tile.obj != ts && r->r_tile.obj != NULL) {
		MAP_PageOut(map, "TS-", r->r_tile.obj);
	} else {
		MAP_PageIn(map, "TS-", ts);
	}
	r->r_tile.obj = ts;
	r->r_tile.id = tile_id;
	r->r_gfx.rs.x = 0;
	r->r_gfx.rs.y = 0;

	if (ts != NULL &&
	    RG_LookupTile(ts, tile_id, &tile) == 0 &&
	    tile->su != NULL) {
		r->r_gfx.rs.w = tile->su->w;
		r->r_gfx.rs.h = tile->su->h;
	}
}

/*
 * Create a tile reference item.
 * The map must be locked.
 */
MAP_Item *
MAP_NodeAddTile(MAP *map, MAP_Node *node, RG_Tileset *ts, Uint32 tileid)
{
	MAP_Item *r;

	r = Malloc(sizeof(MAP_Item));
	MAP_ItemInit(r, MAP_ITEM_TILE);
	MAP_ItemSetTile(r, map, ts, tileid);
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

/*
 * Insert a reference to a location on another map.
 * The map must be locked.
 */
MAP_Item *
MAP_NodeAddWarpPoint(MAP *map, MAP_Node *node, const char *mapname,
    int x, int y, Uint dir)
{
	MAP_Item *r;

	r = Malloc(sizeof(MAP_Item));
	MAP_ItemInit(r, MAP_ITEM_WARP);
	r->r_warp.map = Strdup(mapname);
	r->r_warp.x = x;
	r->r_warp.y = y;
	r->r_warp.dir = dir;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

/*
 * Move a reference to a specified node and optionally assign to a
 * specified layer. This may cause tilesets to be paged in and out.
 */
void
MAP_NodeMoveItem(MAP *sm, MAP_Node *sn, MAP_Item *r, MAP *dm, MAP_Node *dn,
    int dlayer)
{
	AG_ObjectLock(sm);
	AG_ObjectLock(dm);

	TAILQ_REMOVE(&sn->nrefs, r, nrefs);
	TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);

	if (dlayer != -1)
		r->layer = dlayer;

	switch (r->type) {
	case MAP_ITEM_TILE:
		MAP_PageOut(sm, "TS-", r->r_tile.obj);
		MAP_PageIn(dm, "TS-", r->r_tile.obj);
		break;
	default:
		break;
	}
	
	AG_ObjectUnlock(dm);
	AG_ObjectUnlock(sm);
}

/*
 * Copy references from source node sn which are associated with slayer (or
 * all references if slayer is -1) to destination node dn, and associate
 * the copy with dlayer (or the original layer, if dlayer is -1).
 */
void
MAP_NodeCopy(MAP *sm, MAP_Node *sn, int slayer, MAP *dm, MAP_Node *dn,
    int dlayer)
{
	MAP_Item *sr;

	AG_ObjectLock(sm);
	AG_ObjectLock(dm);

	TAILQ_FOREACH(sr, &sn->nrefs, nrefs) {
		if (slayer != -1 &&
		    sr->layer != slayer) {
			continue;
		}
		MAP_NodeCopyItem(sr, dm, dn, dlayer);
	}
	
	AG_ObjectUnlock(dm);
	AG_ObjectUnlock(sm);
}

/*
 * Copy a node reference from one node to another.
 * Both the source and destination maps must be locked.
 */
MAP_Item *
MAP_NodeCopyItem(const MAP_Item *sr, MAP *dm, MAP_Node *dn, int dlayer)
{
	MAP_NodeMask *mask;
	MAP_Item *dr = NULL;
	RG_Transform *xf, *xfDup;

	/* Allocate a new noderef with the same data. */
	switch (sr->type) {
	case MAP_ITEM_TILE:
		dr = MAP_NodeAddTile(dm, dn, sr->r_tile.obj, sr->r_tile.id);
		dr->r_gfx.xcenter = sr->r_gfx.xcenter;
		dr->r_gfx.ycenter = sr->r_gfx.ycenter;
		dr->r_gfx.xmotion = sr->r_gfx.xmotion;
		dr->r_gfx.ymotion = sr->r_gfx.ymotion;
		dr->r_gfx.xorigin = sr->r_gfx.xorigin;
		dr->r_gfx.yorigin = sr->r_gfx.yorigin;
		memcpy(&dr->r_gfx.rs, &sr->r_gfx.rs, sizeof(AG_Rect));
		break;
	case MAP_ITEM_WARP:
		dr = MAP_NodeAddWarpPoint(dm, dn, sr->r_warp.map, sr->r_warp.x,
		    sr->r_warp.y, sr->r_warp.dir);
		break;
	default:
		AG_FatalError("Bad node type");
	}
	dr->flags = sr->flags;
	dr->layer = (dlayer == -1) ? sr->layer : dlayer;
	dr->friction = sr->friction;

	/* Inherit the transformations. */
	TAILQ_FOREACH(xf, &sr->transforms, transforms) {
		xfDup = Malloc(sizeof(RG_Transform));
		RG_TransformInit(xfDup , xf->type, xf->nArgs, xf->args);
		TAILQ_INSERT_TAIL(&dr->transforms, xfDup, transforms);
	}
	
	/* Inherit the node masks. */
	TAILQ_FOREACH(mask, &sr->masks, masks) {
		MAP_NodeMask *nmask;

		nmask = Malloc(sizeof(MAP_NodeMask));
		MAP_NodeMaskInit(nmask, mask->type);
		MAP_NodeMaskCopy(mask, dm, nmask);
		TAILQ_INSERT_TAIL(&dr->masks, nmask, masks);
	}
	return (dr);
}

/* Remove a noderef from a node and free it. */
void
MAP_NodeDelItem(MAP *map, MAP_Node *node, MAP_Item *r)
{
	AG_ObjectLock(map);
	TAILQ_REMOVE(&node->nrefs, r, nrefs);
	MAP_ItemDestroy(map, r);
	AG_ObjectUnlock(map);
	
	free(r);
}

/* Remove all references associated with the given layer. */
void
MAP_NodeRemoveAll(MAP *map, MAP_Node *node, int layer)
{
	MAP_Item *r, *nr;

	AG_ObjectLock(map);
	for (r = TAILQ_FIRST(&node->nrefs);
	     r != TAILQ_END(&node->nrefs);
	     r = nr) {
		nr = TAILQ_NEXT(r, nrefs);
		if (layer != -1 &&
		    layer != r->layer) {
			continue;
		}
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		MAP_ItemDestroy(map, r);
		free(r);
	}
	AG_ObjectUnlock(map);
}

/* Move all references from a layer to another. */
void
MAP_NodeSwapLayers(MAP *map, MAP_Node *node, int layer1, int layer2)
{
	MAP_Item *r;

	AG_ObjectLock(map);
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer == layer1)
			r->layer = layer2;
	}
	AG_ObjectUnlock(map);
}

/*
 * Move a noderef to the upper layer.
 * The map containing the node must be locked.
 */
void
MAP_NodeMoveItemUp(MAP_Node *node, MAP_Item *r)
{
	MAP_Item *next = TAILQ_NEXT(r, nrefs);

	if (next != NULL) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_AFTER(&node->nrefs, next, r, nrefs);
	}
}

/*
 * Move a noderef to the lower layer.
 * The map containing the node must be locked.
 */
void
MAP_NodeMoveItemDown(MAP_Node *node, MAP_Item *r)
{
	MAP_Item *prev = TAILQ_PREV(r, map_itemq, nrefs);

	if (prev != NULL) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_BEFORE(prev, r, nrefs);
	}
}

/*
 * Move a noderef to the tail of the queue.
 * The map containing the node must be locked.
 */
void
MAP_NodeMoveItemToTail(MAP_Node *node, MAP_Item *r)
{
	if (r != TAILQ_LAST(&node->nrefs, map_itemq)) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	}
}

/*
 * Move a noderef to the head of the queue.
 * The map containing the node must be locked.
 */
void
MAP_NodeMoveItemToHead(MAP_Node *node, MAP_Item *r)
{
	if (r != TAILQ_FIRST(&node->nrefs)) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_HEAD(&node->nrefs, r, nrefs);
	}
}

static void
Reset(void *_Nonnull p)
{
	MAP *m = p;
	Uint i;

	if (m->map != NULL)
		MAP_FreeNodes(m);
	if (m->layers != NULL)
		FreeLayers(m);
	if (m->cameras != NULL)
		FreeCameras(m);
	
	for (i = 0; i < m->nBlks; i++) {
		FreeModBlk(m, &m->blks[i]);
	}
	free(m->blks);
	MAP_InitModBlks(m);
}

static void
Destroy(void *_Nonnull p)
{
	MAP *m = p;

	free(m->layers);
	free(m->cameras);
}

/*
 * Load a node reference.
 * The map must be locked.
 */
int
MAP_ItemLoad(MAP *m, AG_DataSource *ds, MAP_Node *node, MAP_Item **r)
{
	char tileset[AG_OBJECT_NAME_MAX];
	RG_Tileset *ts;
	enum map_item_type type;
	Uint32 nmasks = 0, offs;
	Uint8 flags, layer, friction;
	Uint i;

	/* Read the type of reference, flags and the layer#. */
	type = (enum map_item_type)AG_ReadUint32(ds);
	flags = (Uint)AG_ReadUint32(ds);
	layer = AG_ReadUint8(ds);
	friction = AG_ReadSint8(ds);

	/* Read the reference data. */
	switch (type) {
	case MAP_ITEM_TILE:
		AG_CopyString(tileset, ds, sizeof(tileset));
		offs = AG_ReadUint32(ds);
		ts = AG_ObjectFindChild(OBJECT(m)->root, tileset);
		if (ts == NULL || !AG_OfClass(ts, "RG_Tileset:*")) {
			AG_SetError(_("No such tileset: \"%s\""), tileset);
			return (-1);
		}
		*r = MAP_NodeAddTile(m,node, ts,offs);
		(*r)->flags = flags;
		(*r)->layer = layer;
		(*r)->friction = friction;
		(*r)->r_gfx.xcenter = AG_ReadSint16(ds);
		(*r)->r_gfx.ycenter = AG_ReadSint16(ds);
		(*r)->r_gfx.xmotion = AG_ReadSint16(ds);
		(*r)->r_gfx.ymotion = AG_ReadSint16(ds);
		(*r)->r_gfx.xorigin = AG_ReadSint16(ds);
		(*r)->r_gfx.yorigin = AG_ReadSint16(ds);
		(*r)->r_gfx.rs.x = AG_ReadSint16(ds);
		(*r)->r_gfx.rs.y = AG_ReadSint16(ds);
		(*r)->r_gfx.rs.w = AG_ReadUint16(ds);
		(*r)->r_gfx.rs.h = AG_ReadUint16(ds);
		break;
	case MAP_ITEM_WARP:			/* TODO move to separate fn */
		{
			char map_id[AG_OBJECT_NAME_MAX];
			Uint32 ox, oy;
			Uint8 dir;

			if (AG_CopyString(map_id, ds, sizeof(map_id)) >=
			    sizeof(map_id)) {
				AG_SetErrorS(_("Warp map name is too big."));
				return (-1);
			}
			ox = AG_ReadUint32(ds);
			oy = AG_ReadUint32(ds);
			if (ox > MAP_WIDTH_MAX || oy > MAP_HEIGHT_MAX) {
				AG_SetErrorS(_("Invalid warp coordinates."));
				return (-1);
			}
			dir = AG_ReadUint8(ds);
			*r = MAP_NodeAddWarpPoint(m, node, map_id, ox, oy, dir);
			(*r)->flags = flags;
			(*r)->layer = layer;
			(*r)->friction = friction;
		}
		break;
	default:
		AG_SetError("Unknown map item type: %d", type);
		return (-1);
	}
	
	/* Read the transform chain. */
	if (RG_TransformChainLoad(ds, &(*r)->transforms) == -1)
		goto fail;

	/* Read the node masks. */
	if ((nmasks = AG_ReadUint32(ds)) > MAP_ITEM_MAXMASKS) {
		AG_SetError("Too many node masks: %u", (Uint)nmasks);
		goto fail;
	}
	for (i = 0; i < nmasks; i++) {
		MAP_NodeMask *mask;

		mask = Malloc(sizeof(MAP_NodeMask));
		MAP_NodeMaskInit(mask, 0);
		if (MAP_NodeMaskLoad(m, ds, mask) == -1) {
			free(mask);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&(*r)->masks, mask, masks);
	}
	return (0);
fail:
	if (*r != NULL) {
		MAP_ItemDestroy(m, *r);
		free(*r);
		*r = NULL;
	}
	return (-1);
}

int
MAP_NodeLoad(MAP *m, AG_DataSource *ds, MAP_Node *node)
{
	Uint32 nrefs;
	MAP_Item *r;
	Uint i;
	
	if ((nrefs = AG_ReadUint32(ds)) > MAP_NODE_ITEMS_MAX) {
		AG_SetErrorS(_("Too many noderefs."));
		return (-1);
	}
	for (i = 0; i < nrefs; i++) {
		if (MAP_ItemLoad(m, ds, node, &r) == -1) {
			MAP_NodeDestroy(m, node);
			MAP_NodeInit(node);
			return (-1);
		}
	}
	return (0);
}

void
MAP_AttachActor(MAP *m, MAP_Actor *a)
{
	AG_ObjectLock(a);
		
	if (a->g_map.x < 0 || a->g_map.x >= (int)m->mapw ||
	    a->g_map.y < 0 || a->g_map.y >= (int)m->maph)  {
		fprintf(stderr,
		    "Illegal coordinates: %s:%d,%d; not attaching %s\n",
		    OBJECT(m)->name, a->g_map.x, a->g_map.y,
		    OBJECT(a)->name);

		AG_ObjectUnlock(a);
		return;
	}

	MAP_PageIn(m, "ACTOR-", a);

	a->type = AG_ACTOR_MAP;
	a->parent = m;
	a->g_map.x0 = a->g_map.x;
	a->g_map.y0 = a->g_map.y;
	a->g_map.x1 = a->g_map.x;
	a->g_map.y1 = a->g_map.y;
	
	if (MAP_ACTOR_OPS(a)->map != NULL) {
		MAP_ACTOR_OPS(a)->map(a, m);
	}
	AG_ObjectUnlock(a);
}

void
MAP_DetachActor(MAP *m, MAP_Actor *a)
{
	AG_ObjectLock(a);

	if (AG_OfClass(m, "MAP:*")) {
		MAP_ActorUnmapTile(a);
		MAP_PageOut(m, "ACTOR-", a);
	}
	a->parent = NULL;

	AG_ObjectUnlock(a);
}

static int
Load(void *_Nonnull ob, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	MAP *m = ob;
	Uint32 w, h, origin_x, origin_y;
	Uint i, x, y;
	MAP_Actor *a;
	
	m->flags = (Uint)AG_ReadUint32(ds) & AG_MAP_SAVED_FLAGS;
	w = AG_ReadUint32(ds);
	h = AG_ReadUint32(ds);
	origin_x = AG_ReadUint32(ds);
	origin_y = AG_ReadUint32(ds);
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX ||
	    origin_x > MAP_WIDTH_MAX || origin_y > MAP_HEIGHT_MAX) {
		AG_SetErrorS(_("Invalid map geometry."));
		goto fail;
	}
	m->mapw = (Uint)w;
	m->maph = (Uint)h;
	m->origin.x = (int)origin_x;
	m->origin.y = (int)origin_y;
	
	/* Read the layer information. */
	if ((m->nLayers = (Uint)AG_ReadUint32(ds)) > MAP_LAYERS_MAX) {
		AG_SetErrorS(_("Too many layers."));
		goto fail;
	}
	if (m->nLayers < 1) {
		AG_SetErrorS(_("Missing zeroth layer."));
		goto fail;
	}
	m->layers = Realloc(m->layers, m->nLayers*sizeof(MAP_Layer));
	for (i = 0; i < m->nLayers; i++) {
		MAP_Layer *lay = &m->layers[i];

		AG_CopyString(lay->name, ds, sizeof(lay->name));
		lay->visible = (int)AG_ReadUint8(ds);
		lay->xinc = AG_ReadSint16(ds);
		lay->yinc = AG_ReadSint16(ds);
		lay->alpha = AG_ReadUint8(ds);
	}
	m->cur_layer = (int)AG_ReadUint8(ds);
	m->origin.layer = (int)AG_ReadUint8(ds);
	
	/* Read the camera information. */
	if ((m->nCameras = (Uint)AG_ReadUint32(ds)) > MAP_CAMERAS_MAX) {
		AG_SetErrorS(_("Too many cameras."));
		goto fail;
	}
	if (m->nCameras < 1) {
		AG_SetErrorS(_("Missing zeroth camera."));
		goto fail;
	}
	m->cameras = Realloc(m->cameras, m->nCameras*sizeof(MAP_Camera));
	for (i = 0; i < m->nCameras; i++) {
		MAP_Camera *cam = &m->cameras[i];

		AG_CopyString(cam->name, ds, sizeof(cam->name));
		cam->flags = (int)AG_ReadUint32(ds);
		if (i > 0 || m->flags & AG_MAP_SAVE_CAM0POS) {
			cam->x = (int)AG_ReadSint32(ds);
			cam->y = (int)AG_ReadSint32(ds);
		}
		cam->alignment =
		    (enum map_camera_alignment)AG_ReadUint8(ds);
		
		if (i > 0 || m->flags & AG_MAP_SAVE_CAM0ZOOM) {
			cam->zoom = (Uint)AG_ReadUint16(ds);
			cam->tilesz = (Uint)AG_ReadUint16(ds);
			cam->pixsz = cam->tilesz/MAPTILESZ;
		} else {
			cam->zoom = 100;
			cam->tilesz = MAPTILESZ;
			cam->pixsz = 1;
		}

		if (i == 0 && (m->flags & AG_MAP_SAVE_CAM0POS) == 0) {
			cam->x = m->origin.x*cam->tilesz - cam->tilesz/2;
			cam->y = m->origin.y*cam->tilesz - cam->tilesz/2;
		}
	}

	/* Allocate and load the nodes. */
	if (MAP_AllocNodes(m, m->mapw, m->maph) == -1) {
		goto fail;
	}
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			if (MAP_NodeLoad(m, ds, &m->map[y][x]) == -1)
				goto fail;
		}
	}

	/* Attach the actor objects. */
	TAILQ_FOREACH(a, &m->actors, actors) {
		Debug(m, "Attaching actor %s at %d,%d,%d\n",
		     OBJECT(a)->name, a->g_map.x, a->g_map.y,
		     a->g_map.l0);
		Debug(a, "Attached to map %s at %d,%d,%d\n",
		     OBJECT(m)->name, a->g_map.x, a->g_map.y,
		     a->g_map.l0);
		MAP_AttachActor(m, a);
	}
	return (0);
fail:
	return (-1);
}

/*
 * Save a node reference.
 * The noderef's parent map must be locked.
 */
void
MAP_ItemSave(MAP *m, AG_DataSource *ds, MAP_Item *r)
{
	AG_Offset nmasks_offs;
	Uint32 nmasks = 0;
	MAP_NodeMask *mask;

	/* Save the type of reference, flags and layer information. */
	AG_WriteUint32(ds, (Uint32)r->type);
	AG_WriteUint32(ds, (Uint32)r->flags);
	AG_WriteUint8(ds, r->layer);
	AG_WriteSint8(ds, r->friction);

	/* Save the reference. */
	switch (r->type) {
	case MAP_ITEM_TILE:
		AG_WriteString(ds, OBJECT(r->r_tile.obj)->name);
		AG_WriteUint32(ds, r->r_tile.id);
		break;
	case MAP_ITEM_WARP:
		AG_WriteString(ds, r->r_warp.map);
		AG_WriteUint32(ds, (Uint32)r->r_warp.x);
		AG_WriteUint32(ds, (Uint32)r->r_warp.y);
		AG_WriteUint8(ds, r->r_warp.dir);
		break;
	default:
		break;
	}
	if (r->type == MAP_ITEM_TILE) {
		AG_WriteSint16(ds, r->r_gfx.xcenter);
		AG_WriteSint16(ds, r->r_gfx.ycenter);
		AG_WriteSint16(ds, r->r_gfx.xmotion);
		AG_WriteSint16(ds, r->r_gfx.ymotion);
		AG_WriteSint16(ds, r->r_gfx.xorigin);
		AG_WriteSint16(ds, r->r_gfx.yorigin);
		AG_WriteSint16(ds, r->r_gfx.rs.x);
		AG_WriteSint16(ds, r->r_gfx.rs.y);
		AG_WriteUint16(ds, r->r_gfx.rs.w);
		AG_WriteUint16(ds, r->r_gfx.rs.h);
	}

	/* Save the transform chain. */
	RG_TransformChainSave(ds, &r->transforms);

	/* Save the masks. */
	nmasks_offs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	TAILQ_FOREACH(mask, &r->masks, masks) {
		MAP_NodeMaskSave(m, ds, mask);
		nmasks++;
	}
	AG_WriteUint32At(ds, nmasks, nmasks_offs);
}

void
MAP_NodeSave(MAP *m, AG_DataSource *ds, MAP_Node *node)
{
	MAP_Item *r;
	AG_Offset nrefs_offs;
	Uint32 nrefs = 0;

	nrefs_offs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->flags & MAP_ITEM_NOSAVE) {
			continue;
		}
		MAP_ItemSave(m, ds, r);
		nrefs++;
	}
	AG_WriteUint32At(ds, nrefs, nrefs_offs);
}

static int
Save(void *_Nonnull p, AG_DataSource *_Nonnull ds)
{
	MAP *m = p;
	Uint i, x, y;
	
	AG_WriteUint32(ds, (Uint32)(m->flags & AG_MAP_SAVED_FLAGS));
	AG_WriteUint32(ds, (Uint32)m->mapw);
	AG_WriteUint32(ds, (Uint32)m->maph);
	AG_WriteUint32(ds, (Uint32)m->origin.x);
	AG_WriteUint32(ds, (Uint32)m->origin.y);

	/* Write the layer information. */
	AG_WriteUint32(ds, (Uint32)m->nLayers);
	for (i = 0; i < m->nLayers; i++) {
		MAP_Layer *lay = &m->layers[i];

		AG_WriteString(ds, lay->name);
		AG_WriteUint8(ds, (Uint8)lay->visible);
		AG_WriteSint16(ds, lay->xinc);
		AG_WriteSint16(ds, lay->yinc);
		AG_WriteUint8(ds, lay->alpha);
	}
	AG_WriteUint8(ds, m->cur_layer);
	AG_WriteUint8(ds, m->origin.layer);
	
	/* Write the camera information. */
	AG_WriteUint32(ds, (Uint32)m->nCameras);
	for (i = 0; i < m->nCameras; i++) {
		MAP_Camera *cam = &m->cameras[i];

		AG_WriteString(ds, cam->name);
		AG_WriteUint32(ds, (Uint32)cam->flags);
		if (i == 0 && (m->flags & AG_MAP_SAVE_CAM0POS)) {
			AG_WriteSint32(ds, (Sint32)cam->x);
			AG_WriteSint32(ds, (Sint32)cam->y);
		}
		AG_WriteUint8(ds, (Uint8)cam->alignment);
		if (i == 0 && (m->flags & AG_MAP_SAVE_CAM0ZOOM)) {
			AG_WriteUint16(ds, (Uint16)cam->zoom);
			AG_WriteUint16(ds, (Uint16)cam->tilesz);
		}
	}

	/* Write the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++)
			MAP_NodeSave(m, ds, &m->map[y][x]);
	}
	return (0);
}

/* XXX 1.4 */
#if 0
/* Render surface s, scaled to rx,ry pixels. */
static void
BlitSurfaceScaled(MAP *_Nonnull m, AG_Surface *_Nonnull s, AG_Rect *_Nonnull rs,
    int rx, int ry, int cam)
{
	int x, y, sx, sy;
	Uint8 r1, g1, b1, a1;
	Uint32 c;
	int tilesz = m->cameras[cam].tilesz;
	int xSrc = (int)(rs->x*tilesz/MAPTILESZ);
	int ySrc = (int)(rs->y*tilesz/MAPTILESZ);
	int wSrc = (int)(rs->w*tilesz/MAPTILESZ);
	int hSrc = (int)(rs->h*tilesz/MAPTILESZ);

	AG_SurfaceLock(s);
	AG_SurfaceLock(agView->v);
	
	for (y = 0; y < hSrc; y++) {
		if ((sy = (y+ySrc)*MAPTILESZ/tilesz) >= s->h)
			break;

		for (x = 0; x < wSrc; x++) {
			if ((sx = (x+xSrc)*MAPTILESZ/tilesz) >= s->w)
				break;
		
			c = AG_GET_PIXEL(s, (Uint8 *)s->pixels +
			    sy*s->pitch +
			    sx*s->format->BytesPerPixel);
			
			if ((s->flags & AG_SRCCOLORKEY) &&
			    c == s->format->colorkey)
				continue;
		
			if (s->flags & AG_SRCALPHA) {
				AG_GetRGBA(c, s->format, &r1,&g1,&b1,&a1);
				AG_BLEND_RGBA2_CLIPPED(agView->v, rx+x, ry+y,
				    r1, g1, b1, a1, AG_ALPHA_OVERLAY);
			} else {
				AG_GetRGB(c, s->format, &r1,&g1,&b1);
				AG_PutPixel()
				AG_VIEW_PUT_PIXEL2_CLIPPED(rx+x, ry+y,
				    AG_MapRGB(agVideoFmt, r1,g1,b1));
			}
		}
	}
	AG_SurfaceUnlock(s);
	AG_SurfaceUnlock(agView->v);
}
#endif

static MAP_Item *_Nullable
LocateItem(MAP *_Nonnull m, MAP_Node *_Nonnull node, int xoffs, int yoffs,
    int xd, int yd, int ncam)
{
	AG_Rect rExt;
	MAP_Item *r;

	TAILQ_FOREACH_REVERSE(r, &node->nrefs, map_itemq, nrefs) {
		if (r->layer != m->cur_layer) {
			continue;
		}
		switch (r->type) {
		case MAP_ITEM_TILE:
			if (MAP_ItemExtent(m, r, &rExt, ncam) == 0 &&
			    xoffs+xd >= rExt.x && xoffs+xd < rExt.x+rExt.w &&
			    yoffs+yd >= rExt.y && yoffs+yd < rExt.y+rExt.h) {
				return (r);
			}
			break;
		default:
			break;
		}
	}
	return (NULL);
}

/* Locate a map item. */
MAP_Item *
MAP_ItemLocate(MAP *m, int xMap, int yMap, int ncam)
{
	MAP_Camera *cam = &m->cameras[ncam];
	int x = xMap/cam->tilesz;
	int y = yMap/cam->tilesz;
	int xoffs = xMap%cam->tilesz;
	int yoffs = yMap%cam->tilesz;
	MAP_Item *r;

	if (x < 0 || y < 0 || x >= (int)m->mapw || x >= (int)m->maph) {
		return (NULL);
	}
	if ((r = LocateItem(m, &m->map[y][x], xoffs, yoffs, 0, 0, ncam))
	    != NULL) {
		return (r);
	}

	if (y+1 < (int)m->maph) {
		if ((r = LocateItem(m, &m->map[y+1][x], xoffs, yoffs,
		    0, -cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	if (y-1 >= 0) {
		if ((r = LocateItem(m, &m->map[y-1][x], xoffs, yoffs,
		    0, +cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	if (x+1 < (int)m->mapw) {
		if ((r = LocateItem(m, &m->map[y][x+1], xoffs, yoffs,
		    -cam->tilesz, 0, ncam)) != NULL) {
			return (r);
		}
	}
	if (x-1 >= 0) {
		if ((r = LocateItem(m, &m->map[y][x-1], xoffs, yoffs,
		    +cam->tilesz, 0, ncam)) != NULL) {
			return (r);
		}
	}

	/* Check diagonal nodes. */
	if (x+1 < (int)m->mapw && y+1 < (int)m->maph) {
		if ((r = LocateItem(m, &m->map[y+1][x+1], xoffs, yoffs,
		    -cam->tilesz, -cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	if (x-1 >= 0 && y-1 >= 0) {
		if ((r = LocateItem(m, &m->map[y-1][x-1], xoffs, yoffs,
		    +cam->tilesz, +cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	if (x-1 >= 0 && y+1 < (int)m->maph) {
		if ((r = LocateItem(m, &m->map[y+1][x-1], xoffs, yoffs,
		    +cam->tilesz, -cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	if (x+1 < (int)m->mapw && y-1 >= 0) {
		if ((r = LocateItem(m, &m->map[y-1][x+1], xoffs, yoffs,
		    -cam->tilesz, +cam->tilesz, ncam)) != NULL) {
			return (r);
		}
	}
	return (NULL);
}

/*
 * Return the dimensions of a graphical item, and coordinates relative to
 * the origin of the the node.
 */
int
MAP_ItemExtent(MAP *m, MAP_Item *r, AG_Rect *rd, int cam)
{
	int tilesz = m->cameras[cam].tilesz;

	if ((r->type == MAP_ITEM_TILE &&
	     RG_LookupTile(r->r_tile.obj, r->r_tile.id, NULL) == -1)) {
		return (-1);
	}
	if (tilesz != MAPTILESZ) {
		rd->x = r->r_gfx.xcenter*tilesz/MAPTILESZ +
		        r->r_gfx.xmotion*tilesz/MAPTILESZ -
			r->r_gfx.xorigin*tilesz/MAPTILESZ;
		rd->y = r->r_gfx.ycenter*tilesz/MAPTILESZ +
		        r->r_gfx.ymotion*tilesz/MAPTILESZ -
			r->r_gfx.yorigin*tilesz/MAPTILESZ;
		rd->w = r->r_gfx.rs.w*tilesz/MAPTILESZ;
		rd->h = r->r_gfx.rs.h*tilesz/MAPTILESZ;
	} else {
		rd->x = r->r_gfx.xcenter + r->r_gfx.xmotion - r->r_gfx.xorigin;
		rd->y = r->r_gfx.ycenter + r->r_gfx.ymotion - r->r_gfx.yorigin;
		rd->w = r->r_gfx.rs.w;
		rd->h = r->r_gfx.rs.h;
	}
	return (0);
}

void
MAP_ItemDraw(MAP *m, MAP_Item *r, int x, int y, int cam)
{
	/* XXX TODO */
}

/* Create a new undo block at the current level. */
void
MAP_ModBegin(MAP *m)
{
	/* Destroy blocks at upper levels. */
	while (m->nBlks > m->curBlk+1)
		FreeModBlk(m, &m->blks[--m->nBlks]);

	m->blks = Realloc(m->blks, (++m->nBlks)*sizeof(MAP_Mod));
	InitModBlk(&m->blks[m->curBlk++]);
}

void
MAP_ModCancel(MAP *m)
{
	MAP_ModBlk *blk = &m->blks[m->curBlk];

	blk->cancel = 1;
}

void
MAP_ModEnd(MAP *m)
{
	MAP_ModBlk *blk = &m->blks[m->curBlk];
	
	if (blk->nMods == 0 || blk->cancel == 1) {
		FreeModBlk(m, blk);
		m->nBlks--;
		m->curBlk--;
	}
}

void
MAP_Undo(MAP *m)
{
	MAP_ModBlk *blk = &m->blks[m->curBlk];
	Uint i;

	if (m->curBlk-1 <= 0)
		return;

	for (i = 0; i < blk->nMods; i++) {
		MAP_Mod *mm = &blk->mods[i];

		switch (mm->type) {
		case AG_MAPMOD_NODECHG:
			{
				MAP_Node *n = &m->map[mm->mm_nodechg.y]
				                        [mm->mm_nodechg.x];
				MAP_Item *r;
				
				MAP_NodeRemoveAll(m, n, -1);
				TAILQ_FOREACH(r, &mm->mm_nodechg.node.nrefs,
				    nrefs) {
					MAP_NodeCopyItem(r, m, n, -1);
				}
			}
			break;
		default:
			break;
		}
	}
	FreeModBlk(m, blk);
	m->nBlks--;
	m->curBlk--;
}

void
MAP_Redo(MAP *m)
{
	/* TODO */
}

void
MAP_ModNodeChg(MAP *m, int x, int y)
{
	MAP_Node *node = &m->map[y][x];
	MAP_ModBlk *blk = &m->blks[m->nBlks-1];
	MAP_Mod *mm;
	MAP_Item *sr;
	Uint i;

	for (i = 0; i < blk->nMods; i++) {
		mm = &blk->mods[i];
		if (mm->type == AG_MAPMOD_NODECHG &&
		    mm->mm_nodechg.x == x &&
		    mm->mm_nodechg.y == y)
			return;
	}

	blk->mods = Realloc(blk->mods, (blk->nMods+1)*sizeof(MAP_Mod));
	mm = &blk->mods[blk->nMods++];
	mm->type = AG_MAPMOD_NODECHG;
	mm->mm_nodechg.x = x;
	mm->mm_nodechg.y = y;
	MAP_NodeInit(&mm->mm_nodechg.node);

	TAILQ_FOREACH(sr, &node->nrefs, nrefs)
		MAP_NodeCopyItem(sr, m, &mm->mm_nodechg.node, -1);
}

void
MAP_ModLayerAdd(MAP *m, int l)
{
}

#if 0
/* Break a surface into tile-sized fragments and generate a map. */
MAP *
AG_GenerateMapFromSurface(AG_Gfx *gfx, AG_Surface *sprite)
{
	char mapname[AG_OBJECT_NAME_MAX];
	int x, y, mx, my;
	Uint mw, mh;
	AG_Rect sd;
	MAP *fragmap;

	sd.w = MAPTILESZ;
	sd.h = MAPTILESZ;
	mw = sprite->w/MAPTILESZ + 1;
	mh = sprite->h/MAPTILESZ + 1;

	fragmap = Malloc(sizeof(MAP));
	AG_ObjectInit(fragmap, &mapClass);
	if (MAP_AllocNodes(fragmap, mw, mh) == -1)
		AG_FatalError(NULL);

	for (y = 0, my = 0; y < sprite->h; y += MAPTILESZ, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += MAPTILESZ, mx++) {
			AG_Surface *su;
			Uint32 saflags = sprite->flags & (AG_SRCALPHA);
			Uint32 sckflags = sprite->flags & (AG_SRCCOLORKEY);
			Uint8 salpha = sprite->format->alpha;
			Uint32 scolorkey = sprite->format->colorkey;
			MAP_Node *node = &fragmap->map[my][mx];
			Uint32 nsprite;
			int fw = MAPTILESZ;
			int fh = MAPTILESZ;

			if (sprite->w - x < MAPTILESZ)
				fw = sprite->w - x;
			if (sprite->h - y < MAPTILESZ)
				fh = sprite->h - y;

			/* Allocate a surface for the fragment. */
			su = AG_SurfaceRGBA(fw, fh,
			    sprite->format->BitsPerPixel,
			    (sprite->flags &
			     (AG_SRCALPHA|AG_SRCCOLORKEY)),
			    sprite->format->Rmask,
			    sprite->format->Gmask,
			    sprite->format->Bmask,
			    sprite->format->Amask);
			if (su == NULL) {
				AG_FatalError(NULL);
			}
			/* Copy the fragment as-is. */
			AG_SetAlpha(sprite, 0, 0);
			AG_SetColorKey(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			AG_SurfaceBlit(sprite, &sd, su, 0,0);
			nsprite = AG_GfxAddTile(gfx, su);
			AG_SetAlpha(sprite, saflags, salpha);
			AG_SetColorKey(sprite, sckflags, scolorkey);

			/*
			 * Enable alpha blending if there are any pixels
			 * with a non-opaque alpha channel on the surface.
			 */
			if (AG_HasTransparency(su))
				AG_SetAlpha(su, AG_SRCALPHA, su->format->alpha);

			/* Map the sprite as a NULL reference. */
			MAP_NodeAddTile(fragmap,node, NULL,nsprite);
		}
	}

	AG_GfxAddSubmap(gfx, fragmap);
	return (fragmap);
}
#endif

/* Create a new map view. */
static void
CreateView(AG_Event *_Nonnull event)
{
	MAP_View *omv = MAP_VIEW_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);
	MAP *map = omv->map;
	MAP_View *mv;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("View: %s"), OBJECT(map)->name);
	
	mv = MAP_ViewNew(win, map, 0, NULL, NULL);
	mv->cam = MAP_AddCamera(map, _("View"));
	MAP_ViewSizeHint(mv, 2, 2);
	AG_WidgetFocus(mv);
	
	AG_WindowAttach(pwin, win);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 50);
	AG_WindowShow(win);
}

static void
SelectTool(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	MAP_Tool *ntool = AG_PTR(2);

	MAP_ViewSelectTool(mv, ntool, mv->map);
	AG_WidgetFocus(mv);
}

static void
ResizeMap(AG_Event *_Nonnull event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_SELF();
	MAP *m = MAP_PTR(1);
	MAP_View *mv = MAP_VIEW_PTR(2);

	MAP_Resize(m, msb->xvalue, msb->yvalue);
	AG_PostEvent(mv, "map-resized", NULL);
}

/* Display the list of undo blocks. */
static void
PollUndoBlks(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP *m = MAP_PTR(1);
	Uint i, j;

	AG_TlistClear(tl);
	for (i = 0; i < m->nBlks; i++) {
		MAP_ModBlk *blk = &m->blks[i];
		AG_TlistItem *it;

		it = AG_TlistAdd(tl, NULL, "%sBlock %d (%d mods)",
		    i==m->curBlk ? "*" : "", i, blk->nMods);
		it->depth = 0;
		for (j = 0; j < blk->nMods; j++) {
			MAP_Mod *mod = &blk->mods[j];
			
			switch (mod->type) {
			case AG_MAPMOD_NODECHG:
				it = AG_TlistAdd(tl, NULL, "NODECHG (%d,%d)",
				    mod->mm_nodechg.x, mod->mm_nodechg.y);
				break;
			case AG_MAPMOD_LAYERADD:
				it = AG_TlistAdd(tl, NULL, "LAYERADD (%d)",
				    mod->mm_layeradd.nlayer);
				break;
			case AG_MAPMOD_LAYERDEL:
				it = AG_TlistAdd(tl, NULL, "LAYERDEL (%d)",
				    mod->mm_layerdel.nlayer);
				break;
			}
			it->depth = 1;
		}
	}
	AG_TlistRestore(tl);
}

static void
EditMapParameters(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);
	AG_Window *win;
	AG_MSpinbutton *msb;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	MAP *m = mv->map;

	if ((win = AG_WindowNewNamed(0, "MAP_Edit-Parameters-%s",
	    OBJECT(m)->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Map parameters: <%s>"), OBJECT(m)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Map settings"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(ntab, 0, "x", _("Map size: "));
		AG_MSpinbuttonSetRange(msb, 1, MAP_WIDTH_MAX);
		msb->xvalue = m->mapw;
		msb->yvalue = m->maph;
		AG_SetEvent(msb, "mspinbutton-return",
		    ResizeMap, "%p,%p", m, mv);
		
		msb = AG_MSpinbuttonNew(ntab, 0, ",", _("Origin position: "));
		AG_BindInt(msb, "xvalue", &m->origin.x);
		AG_BindInt(msb, "yvalue", &m->origin.y);
		AG_MSpinbuttonSetRange(msb, 0, MAP_WIDTH_MAX);

		AG_NumericalNewIntR(ntab, 0, NULL,
		    _("Origin layer: "), &m->origin.layer, 0, 255);
	}

	ntab = AG_NotebookAdd(nb, _("View"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(ntab, 0, ",", _("Node offset: "));
		AG_BindInt(msb, "xvalue", &mv->mx);
		AG_BindInt(msb, "yvalue", &mv->my);
		AG_MSpinbuttonSetRange(msb,
		   -MAP_WIDTH_MAX/2, MAP_WIDTH_MAX/2);
	
		/* XXX unsafe */
		msb = AG_MSpinbuttonNew(ntab, 0, ",", _("Camera position: "));
		AG_BindInt(msb, "xvalue", &AGMCAM(mv).x);
		AG_BindInt(msb, "yvalue", &AGMCAM(mv).y);
		
		AG_NumericalNewUintR(ntab, 0, NULL,
		    _("Zoom factor: "), &AGMCAM(mv).zoom, 1, 100);
		
		AG_NumericalNewInt(ntab, 0, "px",
		    _("Tile size: "), &AGMTILESZ(mv));
	
		msb = AG_MSpinbuttonNew(ntab, 0, ",",
		    _("Display offset (view): "));
		AG_BindInt(msb, "xvalue", &mv->xoffs);
		AG_BindInt(msb, "yvalue", &mv->yoffs);
		AG_MSpinbuttonSetRange(msb, -MAP_TILESZ_MAX, MAP_TILESZ_MAX);
		
		msb = AG_MSpinbuttonNew(ntab, 0, "x", _("Display area: "));
		AG_BindUint(msb, "xvalue", &mv->mw);
		AG_BindUint(msb, "yvalue", &mv->mh);
		AG_MSpinbuttonSetRange(msb, 1, MAP_WIDTH_MAX);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		AG_CheckboxNewInt(ntab, 0, _("Smooth scaling"),
		    &mapSmoothScaling);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Camera: %i"), &mv->cam);
#ifdef THREADS
		AG_LabelNewPolledMT(ntab, AG_LABEL_HFILL, &OBJECT(m)->lock,
		    _("Current layer: %i"), &OBJECT(m)->lock, &m->cur_layer);
#else
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Current layer: %i"), &m->cur_layer);
#endif
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Cursor position: %ix%i"), &mv->cx, &mv->cy);
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Mouse selection: %[ibool] (%i+%i,%i+%i)"), &mv->msel.set,
		    &mv->msel.x, &mv->msel.xoffs, &mv->msel.y, &mv->msel.yoffs);
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Effective selection: %[ibool] (%ix%i at %i,%i)"),
		    &mv->esel.set,
		    &mv->esel.w, &mv->esel.h, &mv->esel.x, &mv->esel.y);
	}

	ntab = AG_NotebookAdd(nb, _("Undo"), AG_BOX_VERT);
	{
		AG_Tlist *tl;

		tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollUndoBlks, "%p", mv->map);
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

#if 0
/* Scan VFS for objects we can use as library items. */
static void
PollLibsFind(AG_Tlist *_Nonnull tl, AG_Object *_Nonnull pob, int depth)
{
	AG_Object *cob;
	AG_TlistItem *it;
	
	it = AG_TlistAdd(tl, NULL, "%s%s", pob->name,
	    OBJECT_RESIDENT(pob) ? _(" (resident)") : "");
	it->p1 = pob;
	it->depth = depth;

	if (AG_OfClass(pob, "RG_Tileset:*")) {
		RG_Tileset *ts = (RG_Tileset *)pob;
		AG_TlistItem *sit;
		RG_Tile *tile;

		it->cat = "tileset";

		AG_MutexLock(&ts->lock);

		if (!TAILQ_EMPTY(&ts->tiles)) {
			it->flags |= AG_TLIST_HAS_CHILDREN;
		}
		if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
		    AG_TlistVisibleChildren(tl, it)) {
			TAILQ_FOREACH(tile, &ts->tiles, tiles) {
				if (tile->su == NULL) {
					continue;
				}
				sit = AG_TlistAdd(tl, NULL, "%s (%ux%u)",
				    tile->name, tile->su->w, tile->su->h);
				sit->depth = depth+1;
				sit->cat = "tile";
				sit->p1 = tile;
				AG_TlistSetIcon(tl, sit, tile->su);
			}
		}
		AG_MutexUnlock(&ts->lock);
	} else {
		it->cat = "object";
	}

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_ObjectRoot(pob) == pob)
			it->flags |= AG_TLIST_ITEM_EXPANDED;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			PollLibsFind(tl, cob, depth+1);
	}
}
static void
PollLibs(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *pob = AG_OBJECT_PTR(1);

	AG_TlistClear(tl);
	AG_LockLinkage();
	PollLibsFind(tl, pob, 0);
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}
#endif

/* Select a library item. */
static void
SelectLib(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	int state = AG_INT(3);
	MAP_Tool *t;

	if (state == 0) {
		if (mv->curtool != NULL &&
		    (mv->curtool->ops == &mapInsertOps ||
		     mv->curtool->ops == &mapGInsertOps))
		    	MAP_ViewSelectTool(mv, NULL, NULL);
	} else {
		if (strcmp(it->cat, "tile") == 0) {
			RG_Tile *tile = it->p1;
	
			if ((t = MAP_ViewFindTool(mv, "Insert")) != NULL) {
				struct map_insert_tool *ins =
				    (struct map_insert_tool*)t;

				ins->snap_mode = tile->snap_mode;
				ins->replace_mode = (ins->snap_mode ==
				                     RG_SNAP_TO_GRID);

				if (mv->curtool != NULL) {
					MAP_ViewSelectTool(mv, NULL, NULL);
				}
				MAP_ViewSelectTool(mv, t, mv->map);
				AG_WidgetFocus(mv);
			}
		} else if (strcmp(it->cat, "object") == 0 &&
		    AG_OfClass(it->p1, "MAP_Actor:*")) {
			if ((t = MAP_ViewFindTool(mv, "Ginsert")) != NULL) {
				if (mv->curtool != NULL) {
					MAP_ViewSelectTool(mv, NULL, NULL);
				}
				MAP_ViewSelectTool(mv, t, mv->map);
				AG_WidgetFocus(mv);
			}
		} else {
			MAP_ViewSelectTool(mv, NULL, NULL);
		}
	}
}

/* Scan VFS for items we can use as actors. */
static void
PollActors(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP_View *mv = MAP_VIEW_PTR(1);
	MAP *m = mv->map;
	AG_TlistItem *it;
	MAP_Actor *a;

	AG_TlistClear(tl);
	TAILQ_FOREACH(a, &m->actors, actors) {
		it = AG_TlistAdd(tl, NULL, "%s [%d,%d %d-%d]",
		    OBJECT(a)->name, a->g_map.x, a->g_map.y,
		    a->g_map.l0, a->g_map.l1);
		it->p1 = a;
		it->cat = "actor";
	}
	AG_TlistRestore(tl);
}

/* Display the current map layers. */
static void
PollLayers(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP *m = MAP_PTR(1);
	AG_TlistItem *it;
	Uint i;

	AG_TlistClear(tl);
	for (i = 0; i < m->nLayers; i++) {
		MAP_Layer *lay = &m->layers[i];

		it = AG_TlistAdd(tl, mapIconLayerEditor.s, "%s%s%s",
		    (i == m->cur_layer) ? "[*] " : "", lay->name,
		    lay->visible ? "" : _(" (hidden)"));
		it->p1 = lay;
		it->cat = "layer";
	}
	AG_TlistRestore(tl);
}

static void
SetLayerVisibility(AG_Event *_Nonnull event)
{
	MAP_Layer *lay = AG_PTR(1);
	int state = AG_INT(2);

	lay->visible = state;
}

/* Select a layer for edition. */
static void
SelectLayer(AG_Event *_Nonnull event)
{
	MAP *m = MAP_PTR(1);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);
	MAP_Layer *layer = ti->p1;
	Uint nlayer;

	for (nlayer = 0; nlayer < m->nLayers; nlayer++) {
		if (&m->layers[nlayer] == layer) {
			m->cur_layer = nlayer;
			break;
		}
	}
}

/*
 * Delete a layer. Unless this is the top layer, we have to update
 * the layer indices of all relevant items.
 */
static void
DeleteLayer(AG_Event *_Nonnull event)
{
	MAP *m = MAP_PTR(1);
	MAP_Layer *lay = AG_PTR(2);
	Uint i, x, y, nlayer;
	
	for (nlayer = 0; nlayer < m->nLayers; nlayer++) {
		if (&m->layers[nlayer] == lay)
			break;
	}
	if (nlayer == m->nLayers) {
		return;
	}
	if (--m->nLayers < 1) {
		m->nLayers = 1;
		return;
	}
	if (m->cur_layer <= (int)m->nLayers) {
		m->cur_layer = (int)m->nLayers-1;
	}
	for (i = nlayer; i <= m->nLayers; i++) {
		memcpy(&m->layers[i], &m->layers[i+1], sizeof(MAP_Layer));
	}
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r, *nr;

			for (r = TAILQ_FIRST(&node->nrefs);
			     r != TAILQ_END(&node->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->layer == nlayer) {
					TAILQ_REMOVE(&node->nrefs, r, nrefs);
					MAP_ItemDestroy(m, r);
					free(r);
				} else if (r->layer > nlayer) {
					r->layer--;
				}
			}
		}
	}
}

/* Destroy all items on a given layer. */
static void
ClearLayer(AG_Event *_Nonnull event)
{
	MAP *m = MAP_PTR(1);
	MAP_Layer *lay = AG_PTR(2);
	Uint x, y, nlayer;
	
	for (nlayer = 0; nlayer < m->nLayers; nlayer++) {
		if (&m->layers[nlayer] == lay)
			break;
	}
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r, *nr;

			for (r = TAILQ_FIRST(&node->nrefs);
			     r != TAILQ_END(&node->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->layer == nlayer) {
					TAILQ_REMOVE(&node->nrefs, r, nrefs);
					MAP_ItemDestroy(m, r);
					free(r);
				}
			}
		}
	}
}

/* Move a layer (and its associated items) up or down the stack. */
static void
MoveLayer(AG_Event *_Nonnull event)
{
	char tmp[MAP_LAYER_NAME_MAX];
	MAP *m = MAP_PTR(1);
	MAP_Layer *lay1 = AG_PTR(2), *lay2;
	int movedown = AG_INT(3);
	AG_Tlist *tlLayers = AG_TLIST_PTR(4);
	Uint l1, l2;
	Uint x, y;

	for (l1 = 0; l1 < m->nLayers; l1++) {
		if (&m->layers[l1] == lay1)
			break;
	}
	if (l1 == m->nLayers) {
		return;
	}
	if (movedown) {
		l2 = l1+1;
		if (l2 >= m->nLayers) return;
	} else {
		if (((int)l1 - 1) < 0) return;
		l2 = l1-1;
	}
	lay1 = &m->layers[l1];
	lay2 = &m->layers[l2];
	Strlcpy(tmp, lay1->name, sizeof(tmp));
	Strlcpy(lay1->name, lay2->name, sizeof(lay1->name));
	Strlcpy(lay2->name, tmp, sizeof(lay2->name));

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer == l1) {
					r->layer = l2;
				} else if (r->layer == l2) {
					r->layer = l1;
				}
			}
		}
	}
	AG_TlistSelectPtr(tlLayers, lay2);
}

/* Create a new layer. */
static void
PushLayer(AG_Event *_Nonnull event)
{
	char name[MAP_LAYER_NAME_MAX];
	MAP *m = MAP_PTR(1);
	AG_Textbox *tb = AG_TEXTBOX_PTR(2);
	
	AG_TextboxCopyString(tb, name, sizeof(name));

	if (MAP_PushLayer(m, name) != 0) {
		AG_TextMsgFromError();
	} else {
		AG_TextboxPrintf(tb, NULL);
	}
}

#if 0
static void
EditItemProps(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	int xoffs = AG_INT(4);
	int yoffs = AG_INT(5);
	MAP_Item *r;
	AG_Window *pwin, *win;
	AG_MSpinbutton *msb;

	if ((r = MAP_ItemLocate(mv->map, mv->mouse.xmap, mv->mouse.ymap,
	    mv->cam)) == NULL) {
		return;
	}

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Map item"));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 1);

	AG_LabelNew(win, 0, _("Type: %s"),
	    (r->type == MAP_ITEM_TILE) ? _("Tile") :
	    (r->type == MAP_ITEM_WARP) ? _("Warp point") : "?");
	msb = AG_MSpinbuttonNew(win, 0, ",", _("Centering: "));
	AG_BindSint16(msb, "xvalue", &r->r_gfx.xcenter);
	AG_BindSint16(msb, "yvalue", &r->r_gfx.ycenter);
	msb = AG_MSpinbuttonNew(win, 0, ",", _("Motion: "));
	AG_BindSint16(msb, "xvalue", &r->r_gfx.xmotion);
	AG_BindSint16(msb, "yvalue", &r->r_gfx.ymotion);
	msb = AG_MSpinbuttonNew(win, 0, ",", _("Origin: "));
	AG_BindSint16(msb, "xvalue", &r->r_gfx.xorigin);
	AG_BindSint16(msb, "yvalue", &r->r_gfx.yorigin);
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	msb = AG_MSpinbuttonNew(win, 0, ",", _("Source coords: "));
	AG_BindSint16(msb, "xvalue", &r->r_gfx.rs.x);
	AG_BindSint16(msb, "yvalue", &r->r_gfx.rs.y);
	msb = AG_MSpinbuttonNew(win, 0, "x", _("Source dims: "));
	AG_BindSint16(msb, "xvalue", &r->r_gfx.rs.w);
	AG_BindSint16(msb, "yvalue", &r->r_gfx.rs.h);
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	AG_NumericalNewUint8(win, 0, NULL, _("Layer: "), &r->layer);
	AG_NumericalNewUint8(win, 0, NULL, _("Friction: "), &r->friction);

	if ((pwin = AG_WidgetParentWindow(mv)) != NULL) {
		AG_WindowAttach(pwin, win);
	}
	AG_WindowShow(win);
}
#endif

/* Set the attribute edition mode. */
static void
EditPropMode(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	int flag = AG_INT(2);

	if (flag != 0) {
		mv->mode = MAP_VIEW_EDIT_ATTRS;
		mv->edit_attr = flag;
	} else {
		mv->mode = MAP_VIEW_EDIT;
	}
}

static void
Undo(AG_Event *_Nonnull event)
{
	MAP_Undo(MAP_PTR(1));
}

static void
Redo(AG_Event *_Nonnull event)
{
	MAP_Redo(MAP_PTR(1));
}

static void
CenterViewToOrigin(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);

	AGMCAM(mv).x = mv->map->origin.x*AGMTILESZ(mv) - AGMTILESZ(mv)/2;
	AGMCAM(mv).y = mv->map->origin.y*AGMTILESZ(mv) - AGMTILESZ(mv)/2;
	MAP_ViewUpdateCamera(mv);
}

/* Detach an actor object. */
static void
DetachActor(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	MAP *m = MAP_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	     strcmp(it->cat, "actor") == 0) {
		MAP_Actor *a = it->p1;
	
		TAILQ_REMOVE(&m->actors, a, actors);
		MAP_DetachActor(m, a);
	}
}

/* Control an actor object. */
static void
SelectActor(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	MAP_View *mv = MAP_VIEW_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	     strcmp(it->cat, "actor") == 0) {
		MAP_Actor *a = it->p1;
	
		MAP_ViewControl(mv, _("Player 1"), a);
	}
}

/* Remove all items referencing any element of the given tileset. */
static void
RemoveAllRefsToTileset(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	MAP_View *mv = MAP_VIEW_PTR(2);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	RG_Tileset *ts = it->p1;
	MAP *m = mv->map;
	Uint x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *n = &m->map[y][x];
			MAP_Item *r, *r2;

			for (r = TAILQ_FIRST(&n->nrefs);
			     r != TAILQ_END(&n->nrefs);
			     r = r2) {
				r2 = TAILQ_NEXT(r, nrefs);
				if (r->type == MAP_ITEM_TILE &&
				    r->r_tile.obj == ts)
					MAP_NodeDelItem(m, n, r);
			}
		}
	}
}

/* Remove all items referencing a given tile. */
static void
RemoveAllRefsToTile(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	MAP_View *mv = MAP_VIEW_PTR(2);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	RG_Tile *tile = it->p1;
	MAP *m = mv->map;
	Uint x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *n = &m->map[y][x];
			MAP_Item *r, *r2;
			RG_Tile *ntile;

			for (r = TAILQ_FIRST(&n->nrefs);
			     r != TAILQ_END(&n->nrefs);
			     r = r2) {
				r2 = TAILQ_NEXT(r, nrefs);
				if (RG_LookupTile(r->r_tile.obj, r->r_tile.id,
				    &ntile) == 0 && (ntile == tile))
					MAP_NodeDelItem(m, n, r);
			}
		}
	}
}

/* Generate the menu that pops up when clicking on a layer. */
static void
CreateLayerMenu(AG_Event *_Nonnull event)
{
	MAP *m             = MAP_PTR(1);
	AG_Tlist *tlLayers = AG_TLIST_PTR(2);
	AG_MenuItem *mi    = AG_MENU_ITEM_PTR(3);
	MAP_Layer *layer;

	if ((layer = AG_TlistSelectedItemPtr(tlLayers)) == NULL) {
		return;
	}
	AG_MenuAction(mi,
	    layer->visible ? _("Hide layer") : _("Show layer"), NULL,
	    SetLayerVisibility, "%p,%i", layer, !layer->visible);
	AG_MenuAction(mi, _("Delete layer"), agIconTrash.s,
	    DeleteLayer, "%p,%p", m, layer);
	AG_MenuAction(mi, _("Clear layer"), agIconTrash.s,
	    ClearLayer, "%p,%p", m, layer);
	AG_MenuSeparator(mi);
	AG_MenuActionKb(mi, _("Move layer up"), agIconUp.s,
	    AG_KEY_U, AG_KEYMOD_SHIFT,
	    MoveLayer, "%p,%p,%i", m, layer, 0, tlLayers); 
	AG_MenuActionKb(mi, _("Move layer down"), agIconDown.s,
	    AG_KEY_D, AG_KEYMOD_SHIFT,
	    MoveLayer, "%p,%p,%i", m, layer, 1, tlLayers); 
}

static void *_Nullable
Edit(void *_Nonnull p)
{
	MAP *m = p;
	AG_Window *win;
	AG_Toolbar *toolbar;
	AG_Statusbar *statbar;
	AG_Scrollbar *hbar, *vbar;
	MAP_View *mv;
	AG_Menu *menu;
	AG_MenuItem *pitem;
	AG_Box *hBox, *vBox;
	AG_Pane *hPane, *vPane;
	Uint flags = MAP_VIEW_GRID;

	if ((OBJECT(m)->flags & AG_OBJECT_READONLY) == 0)
		flags |= MAP_VIEW_EDIT;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(m)->name);

	toolbar = AG_ToolbarNew(NULL, AG_TOOLBAR_VERT, 2, 0);
	statbar = AG_StatusbarNew(NULL, 0);
	
	mv = MAP_ViewNew(NULL, m, flags, toolbar, statbar);
	MAP_ViewSizeHint(mv, 2, 2);
#if 0
	AG_SetEvent(mv, "mapview-dblclick", EditItemProps, NULL);
#endif

	menu = AG_MenuNew(win, AG_MENU_HFILL);
	pitem = AG_MenuNode(menu->root, _("File"), NULL);
	{
		AG_MenuActionKb(pitem, _("Close map"), agIconClose.s,
		    AG_KEY_W, AG_KEYMOD_CTRL, AGWINCLOSE(win));
	}
	
	pitem = AG_MenuNode(menu->root, _("Edit"), NULL);
	{
		AG_MenuAction(pitem, _("Undo"), NULL, Undo, "%p", m);
		AG_MenuAction(pitem, _("Redo"), NULL, Redo, "%p", m);

		AG_MenuSeparator(pitem);

		AG_MenuAction(pitem, _("Map parameters..."), mapIconSettings.s,
		    EditMapParameters, "%p,%p", mv, win);
	}
	
	pitem = AG_MenuNode(menu->root, _("Attributes"), NULL);
	{
		AG_MenuAction(pitem, _("None"), NULL,
		    EditPropMode, "%p,%i", mv, 0);

		AG_MenuAction(pitem, _("Walkability"), mapIconWalkable.s,
		    EditPropMode, "%p,%i", mv, MAP_ITEM_BLOCK);
		AG_MenuAction(pitem, _("Climbability"), mapIconClimbable.s,
		    EditPropMode, "%p,%i", mv, MAP_ITEM_CLIMBABLE);
		AG_MenuAction(pitem, _("Jumpability"), mapIconJumpable.s,
		    EditPropMode, "%p,%i", mv, MAP_ITEM_JUMPABLE);
		AG_MenuAction(pitem, _("Slippery"), mapIconSlippery.s,
		    EditPropMode, "%p,%i", mv, MAP_ITEM_SLIPPERY);
	}

	pitem = AG_MenuNode(menu->root, _("View"), NULL);
	{
		extern int mapViewAnimatedBg;

		AG_MenuAction(pitem, _("Create view..."), mapIconNewView.s,
		    CreateView, "%p, %p", mv, win);
		AG_MenuAction(pitem, _("Center around origin"), mapIconOrigin.s,
		    CenterViewToOrigin, "%p", mv);

		AG_MenuSeparator(pitem);

		AG_MenuIntFlags(pitem, _("Show grid"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_GRID, 0);
		AG_MenuIntFlags(pitem, _("Show background"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_NO_BG, 1);
		AG_MenuIntBool(pitem, _("Animate background"), mapIconGrid.s,
		    &mapViewAnimatedBg, 0);
		AG_MenuIntFlags(pitem, _("Show map origin"), mapIconOrigin.s,
		    &mv->flags, MAP_VIEW_SHOW_ORIGIN, 0);
		AG_MenuIntFlags(pitem, _("Show element offsets"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_SHOW_OFFSETS, 0);
	}
	
	hPane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	AG_PaneSetDivisionPacking(hPane, 1, AG_BOX_HORIZ);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Tlist *tl;
		AG_MenuItem *mi;

		vPane = AG_PaneNewVert(hPane->div[0], AG_PANE_EXPAND);
		nb = AG_NotebookNew(vPane->div[0], AG_NOTEBOOK_EXPAND);
		ntab = AG_NotebookAdd(nb, _("Library"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND);
#if 0
			AG_SetEvent(tl, "tlist-poll", PollLibs, "%p", agWorld);
#endif
			AG_SetEvent(tl, "tlist-changed", SelectLib, "%p", mv);
			mv->lib_tl = tl;
			WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);

			mi = AG_TlistSetPopup(mv->lib_tl, "tileset");
			{
				AG_MenuAction(mi, _("Remove all references to"),
				    agIconTrash.s,
				    RemoveAllRefsToTileset, "%p,%p",
				    mv->lib_tl, mv); 
			}
			mi = AG_TlistSetPopup(mv->lib_tl, "tile");
			{
				AG_MenuAction(mi, _("Remove all references to"),
				    agIconTrash.s,
				    RemoveAllRefsToTile, "%p,%p",
				    mv->lib_tl, mv); 
			}
		}
		ntab = AG_NotebookAdd(nb, _("Objects"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND);
			AG_SetEvent(tl, "tlist-poll", PollActors, "%p", mv);
//			AG_SetEvent(tl, "tlist-changed", select_obj, "%p", mv);
			mv->objs_tl = tl;
			WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
			
			mi = AG_TlistSetPopup(mv->objs_tl, "actor");
			{
				AG_MenuAction(mi, _("Control actor"),
				    mapIconActor.s,
				    SelectActor, "%p,%p", mv->objs_tl, mv); 

				AG_MenuAction(mi, _("Detach actor"),
				    agIconTrash.s,
				    DetachActor, "%p,%p", mv->objs_tl, m); 
			}
		}
		ntab = AG_NotebookAdd(nb, _("Layers"), AG_BOX_VERT);
		{
			AG_Textbox *tb;

			mv->layers_tl = AG_TlistNew(ntab, AG_TLIST_POLL|
			                                  AG_TLIST_EXPAND);
			AG_TlistSetItemHeight(mv->layers_tl, MAPTILESZ);
			AG_SetEvent(mv->layers_tl, "tlist-poll",
			    PollLayers, "%p", m);
			AG_SetEvent(mv->layers_tl, "tlist-dblclick",
			    SelectLayer, "%p", m);

			mi = AG_TlistSetPopup(mv->layers_tl, "layer");
			AG_MenuSetPollFn(mi, CreateLayerMenu, "%p,%p",
			    m, mv->layers_tl);

			hBox = AG_BoxNew(ntab, AG_BOX_HORIZ, AG_BOX_HFILL);
			tb = AG_TextboxNewS(hBox, 0, _("Name: "));
			AG_SetEvent(tb, "textbox-return",
			    PushLayer, "%p, %p", m, tb);
			AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Push"),
			    PushLayer, "%p, %p", m, tb);
		}
		
		AG_SeparatorNew(hPane->div[0], AG_SEPARATOR_HORIZ);
		
		vbar = AG_ScrollbarNew(hPane->div[1], AG_SCROLLBAR_VERT, 0);
		vBox = AG_BoxNew(hPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
		{
			AG_ObjectAttach(vBox, mv);
			hbar = AG_ScrollbarNew(vBox, AG_SCROLLBAR_HORIZ, 0);
		}
		AG_ObjectAttach(hPane->div[1], toolbar);
	}

	pitem = AG_MenuNode(menu->root, _("Tools"), NULL);
	{
		const MAP_ToolOps *ops[] = {
			&mapNodeselOps,
			&mapRefselOps,
			&mapFillOps,
			&mapEraserOps,
			&mapFlipOps,
			&mapInvertOps,
			&mapInsertOps,
			&mapGInsertOps
		};
		const int nops = sizeof(ops) / sizeof(ops[0]);
		MAP_Tool *t;
		int i;

		for (i = 0; i < nops; i++) {
			t = MAP_ViewRegTool(mv, ops[i], m);
			t->pane = (void *)vPane->div[1];
			AG_MenuAction(pitem, _(ops[i]->desc),
			    (ops[i]->icon!=NULL) ? ops[i]->icon->s : NULL,
			    SelectTool, "%p, %p", mv, t);
		}
	}
	
	MAP_ViewUseScrollbars(mv, hbar, vbar);
	AG_ObjectAttach(win, statbar);

	AG_PaneMoveDividerPct(hPane, 40);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 40);

	AG_LabelTextS(mv->status,
	    _("Select a tool or double-click on an element to insert."));

	AG_WidgetFocus(mv);
	return (win);
}

AG_ObjectClass mapClass = {
	"MAP",
	sizeof(MAP),
	{ 11, 0 },
	Init,
	Reset,
	Destroy,
	Load,
	Save,
	Edit
};
