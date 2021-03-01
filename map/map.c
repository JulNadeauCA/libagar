/*
 * Copyright (c) 2001-2021 Julien Nadeau Carriere <vedge@csoft.net>
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

MAP_ItemClass *mapItemClasses[MAP_ITEM_LAST] = {
	&mapTileClass,     /* MAP_ITEM_TILE */
	&mapImgClass,      /* MAP_ITEM_IMG */
	&mapLinkClass      /* MAP_ITEM_LINK */
};

void
MAP_InitSubsystem(void)
{
	Uint i;

	AG_RegisterNamespace("MAP", "MAP_", "http://libagar.org/");
	AG_RegisterClass(&mapClass);
	AG_RegisterClass(&mapActorClass);
	AG_RegisterClass(&mapEditorClass);
	AG_RegisterClass(&mapEditorPseudoClass);
	AG_RegisterClass(&mapViewClass);

	mapIcon_Init();

	for (i = MAP_ITEM_PRIVATE; i <= MAP_ITEM_LAST; i++)
		mapItemClasses[i] = NULL;
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
	TAILQ_INIT(&node->items);
}

void
MAP_NodeDestroy(MAP *map, MAP_Node *node)
{
	MAP_Item *mi, *miNext;

	for (mi = TAILQ_FIRST(&node->items);
	     mi != TAILQ_END(&node->items);
	     mi = miNext) {
		miNext = TAILQ_NEXT(mi, items);
		MAP_ItemDestroy(map, mi);
		free(mi);
	}
}

void
MAP_ItemInit(void *obj, enum map_item_type type)
{
	MAP_Item *mi = MAPITEM(obj);

	mi->type = type;
	mi->flags = 0;
	mi->layer = 0;
	mi->p = NULL;
	RG_TransformChainInit(&mi->transforms);

#ifdef AG_DEBUG
	if (type >= MAP_ITEM_LAST)
		AG_FatalError("No such item type");
#endif
	if (mapItemClasses[type]->init != NULL)
		mapItemClasses[type]->init(mi);
}

void
MAP_ItemDestroy(MAP *map, MAP_Item *mi)
{
	MAP_ItemClass *miClass = mapItemClasses[mi->type];

	if (miClass->destroy != NULL)
		miClass->destroy(mi);

	RG_TransformChainDestroy(&mi->transforms);
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
MAP_AllocNodes(MAP *map, Uint w, Uint h)
{
	Uint x, y;
	
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError(_("%ux%u nodes exceed %ux%u."), w, h,
		    MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	AG_ObjectLock(map);
	map->w = w;
	map->h = h;
	map->map = Malloc(h * sizeof(MAP_Node *));
	for (y = 0; y < h; y++) {
		map->map[y] = Malloc(w * sizeof(MAP_Node));
		for (x = 0; x < w; x++)
			MAP_NodeInit(&map->map[y][x]);
	}
	AG_ObjectUnlock(map);
	return (0);
}

/* Release the node map. */
void
MAP_FreeNodes(MAP *map)
{
	Uint x,y;
	MAP_Node *node;

	AG_ObjectLock(map);
	if (map->map != NULL) {
		for (y = 0; y < map->h; y++) {
			for (x = 0; x < map->w; x++) {
				node = &map->map[y][x];
				MAP_NodeDestroy(map, node);
			}
			free(map->map[y]);
		}
		free(map->map);
		map->map = NULL;
	}
	AG_ObjectUnlock(map);
}

static void
FreeLayers(MAP *_Nonnull map)
{
	map->layers = Realloc(map->layers, sizeof(MAP_Layer));
	map->nLayers = 1;
	MAP_InitLayer(&map->layers[0], _("Layer 0"));
}

static void
FreeCameras(MAP *_Nonnull map)
{
	map->cameras = Realloc(map->cameras , sizeof(MAP_Camera));
	map->nCameras = 1;
	MAP_InitCamera(&map->cameras[0], _("Camera 0"));
}

/* Resize a map, initializing new nodes and destroying any excess ones. */
int
MAP_Resize(MAP *map, Uint w, Uint h)
{
	MAP mapTmp;
	Uint x, y;

	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError(_("%ux%u nodes exceed %ux%u."), w, h,
		    MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	AG_ObjectLock(map);

	/* Save the nodes to a temporary map, to preserve dependencies. */
	AG_ObjectInitStatic(&mapTmp, &mapClass);

	if (MAP_AllocNodes(&mapTmp, map->w, map->h) == -1) {
		goto fail;
	}
	for (y = 0; y < map->h && y < h; y++) {
		for (x = 0; x < map->w && x < w; x++)
			MAP_NodeCopy(map,      &map->map[y][x], -1,
			            &mapTmp, &mapTmp.map[y][x], -1);
	}

	/* Resize the map, restore the original nodes. */
	MAP_FreeNodes(map);
	if (MAP_AllocNodes(map, w,h) == -1) {
		goto fail;
	}
	for (y = 0; y < mapTmp.h && y < map->h; y++) {
		for (x = 0; x < mapTmp.w && x < map->w; x++)
			MAP_NodeCopy(&mapTmp, &mapTmp.map[y][x], -1,
			              map,      &map->map[y][x], -1);
	}

	if (map->xOrigin >= (int)w) { map->xOrigin = (int)w-1; }
	if (map->yOrigin >= (int)h) { map->yOrigin = (int)h-1; }

	AG_ObjectUnlock(map);
	AG_ObjectDestroy(&mapTmp);
	return (0);
fail:
	AG_ObjectUnlock(map);
	AG_ObjectDestroy(&mapTmp);
	return (-1);
}

/* Set the display scaling factor. */
void
MAP_SetZoom(MAP *map, int ncam, Uint zoom)
{
	MAP_Camera *cam = &map->cameras[ncam];

	AG_ObjectLock(map);
	cam->zoom = zoom;
	if ((cam->tilesz = cam->zoom*MAPTILESZ/100) > MAP_TILESZ_MAX) {
		cam->tilesz = MAP_TILESZ_MAX;
	}
	AG_ObjectUnlock(map);
}

MAP *
MAP_New(void *parent, const char *name)
{
	MAP *map;

	map = Malloc(sizeof(MAP));
	AG_ObjectInit(map, &mapClass);
	AG_ObjectSetNameS(map, name);
	AG_ObjectAttach(parent, map);
	return (map);
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
MAP_AddCamera(MAP *map, const char *name)
{
	map->cameras = Realloc(map->cameras,
	    (map->nCameras + 1) * sizeof(MAP_Camera));
	MAP_InitCamera(&map->cameras[map->nCameras], name);
	return (map->nCameras++);
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
FreeModBlk(MAP *_Nonnull map, MAP_ModBlk *_Nonnull blk)
{
	Uint i;

	for (i = 0; i < blk->nMods; i++) {
		MAP_Mod *mm = &blk->mods[i];
	
		switch (mm->type) {
		case AG_MAPMOD_NODECHG:
			MAP_NodeDestroy(map, &mm->mm_nodechg.node);
			break;
		default:
			break;
		}
	}
	free(blk->mods);
}

void
MAP_InitModBlks(MAP *map)
{
	map->blks = Malloc(sizeof(MAP_ModBlk));
	map->nBlks = 1;
	map->curBlk = 0;
	map->nMods = 0;

	InitModBlk(&map->blks[0]);
}

static void
Init(void *_Nonnull obj)
{
	MAP *map = obj;

	map->flags = 0;
	map->w = 0;
	map->h = 0;
	map->layerCur = 0;
	map->xOrigin = 0;
	map->yOrigin = 0;
	map->layerOrigin = 0;
	map->map = NULL;
	map->layers = Malloc(sizeof(MAP_Layer));
	map->nLayers = 1;
	map->cameras = Malloc(sizeof(MAP_Camera));
	map->nCameras = 1;
	TAILQ_INIT(&map->actors);
	
	MAP_InitLayer(&map->layers[0], _("Layer 0"));
	MAP_InitCamera(&map->cameras[0], _("Camera 0"));
	MAP_InitModBlks(map);

	if (!mapEditorInited) {
		mapEditorInited = 1;
		MAP_EditorInit();
	}
	MAP_AllocNodes(map, mapDefaultWidth, mapDefaultHeight);
	map->xOrigin = mapDefaultWidth >> 1;
	map->yOrigin = mapDefaultHeight >> 1;
}

/* Create a new layer. */
int
MAP_PushLayer(MAP *map, const char *name)
{
	char layname[MAP_LAYER_NAME_MAX];

	if (name[0] == '\0') {
		Snprintf(layname, sizeof(layname), _("Layer %u"), map->nLayers);
	} else {
		Strlcpy(layname, name, sizeof(layname));
	}

	if (map->nLayers+1 > MAP_LAYERS_MAX) {
		AG_SetErrorS(_("Too many layers."));
		return (-1);
	}
	map->layers = Realloc(map->layers, (map->nLayers+1)*sizeof(MAP_Layer));
	MAP_InitLayer(&map->layers[map->nLayers], layname);
	map->nLayers++;
	return (0);
}

/* Remove the last layer. */
void
MAP_PopLayer(MAP *map)
{
	if (--map->nLayers < 1)
		map->nLayers = 1;
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

/*
 * Copy node items from mapSrc:nodeSrc to mapDst:nodeDst.
 *
 * Copy only those items on layerSrc (or -1 = all layers).
 * Set destination layer to layerDst (or -1 = original layer in mapSrc).
 */
void
MAP_NodeCopy(MAP *mapSrc, MAP_Node *nodeSrc, int layerSrc, MAP *mapDst,
    MAP_Node *nodeDst, int layerDst)
{
	MAP_Item *miSrc;

	AG_ObjectLock(mapSrc);
	if (mapDst != mapSrc)
		AG_ObjectLock(mapDst);

	TAILQ_FOREACH(miSrc, &nodeSrc->items, items) {
		if (layerSrc != -1 &&
		    miSrc->layer != layerSrc) {
			continue;
		}
		MAP_NodeDuplicate(miSrc, mapDst, nodeDst, layerDst);
	}
	
	if (mapDst != mapSrc) {
		AG_ObjectUnlock(mapDst);
	}
	AG_ObjectUnlock(mapSrc);
}

/*
 * Duplicate a given map item onto mapDst:nodeDst.
 * Both source and destination maps must be locked.
 */
MAP_Item *
MAP_NodeDuplicate(const MAP_Item *miSrc, MAP *mapDst, MAP_Node *nodeDst,
    int layerDst)
{
	MAP_Item *miDup;
	RG_Transform *xf, *xfDup;

	/* Allocate a new node item with the same data. */
	miDup = (MAP_Item *)mapItemClasses[miSrc->type]->duplicate(mapDst,
	                                               nodeDst,
	                                               (const void *)miSrc);
	miDup->flags = miSrc->flags;
	miDup->layer = (layerDst == -1) ? miSrc->layer : layerDst;

	TAILQ_FOREACH(xf, &miSrc->transforms, transforms) {
		xfDup = Malloc(sizeof(RG_Transform));
		RG_TransformInit(xfDup, xf->type, xf->nArgs, xf->args);
		TAILQ_INSERT_TAIL(&miDup->transforms, xfDup, transforms);
	}
	return (miDup);
}

/* Unlink a node item and free it. */
void
MAP_NodeDelItem(MAP *map, MAP_Node *node, MAP_Item *mi)
{
	AG_ObjectLock(map);

	TAILQ_REMOVE(&node->items, mi, items);
	MAP_ItemDestroy(map, mi);

	AG_ObjectUnlock(map);
	
	free(mi);
}

/* Remove all references associated with the given layer. */
void
MAP_NodeRemoveAll(MAP *map, MAP_Node *node, int layer)
{
	MAP_Item *mi, *miNext;

	AG_ObjectLock(map);

	for (mi = TAILQ_FIRST(&node->items);
	     mi != TAILQ_END(&node->items);
	     mi = miNext) {
		miNext = TAILQ_NEXT(mi, items);
		if (layer != -1 &&
		    layer != mi->layer) {
			continue;
		}
		TAILQ_REMOVE(&node->items, mi, items);
		MAP_ItemDestroy(map, mi);
		free(mi);
	}

	AG_ObjectUnlock(map);
}

/* Move all references from a layer to another. */
void
MAP_NodeSwapLayers(MAP *map, MAP_Node *node, int layer1, int layer2)
{
	MAP_Item *mi;

	AG_ObjectLock(map);

	TAILQ_FOREACH(mi, &node->items, items) {
		if (mi->layer == layer1)
			mi->layer = layer2;
	}

	AG_ObjectUnlock(map);
}

/*
 * Move a node item to the upper layer.
 * The parent map must be locked.
 */
void
MAP_NodeMoveItemUp(MAP_Node *node, MAP_Item *mi)
{
	MAP_Item *miNext = TAILQ_NEXT(mi, items);

	if (miNext != NULL) {
		TAILQ_REMOVE(&node->items, mi, items);
		TAILQ_INSERT_AFTER(&node->items, miNext, mi, items);
	}
}

/*
 * Move a node item to the lower layer.
 * The parent map must be locked.
 */
void
MAP_NodeMoveItemDown(MAP_Node *node, MAP_Item *mi)
{
	MAP_Item *miPrev = TAILQ_PREV(mi, map_itemq, items);

	if (miPrev != NULL) {
		TAILQ_REMOVE(&node->items, mi, items);
		TAILQ_INSERT_BEFORE(miPrev, mi, items);
	}
}

/*
 * Move a node item to the tail of the queue.
 * The parent map must be locked.
 */
void
MAP_NodeMoveItemToTail(MAP_Node *node, MAP_Item *mi)
{
	if (mi != TAILQ_LAST(&node->items, map_itemq)) {
		TAILQ_REMOVE(&node->items, mi, items);
		TAILQ_INSERT_TAIL(&node->items, mi, items);
	}
}

/*
 * Move a node item to the head of the queue.
 * The parent map must be locked.
 */
void
MAP_NodeMoveItemToHead(MAP_Node *node, MAP_Item *mi)
{
	if (mi != TAILQ_FIRST(&node->items)) {
		TAILQ_REMOVE(&node->items, mi, items);
		TAILQ_INSERT_HEAD(&node->items, mi, items);
	}
}

static void
Reset(void *_Nonnull obj)
{
	MAP *map = obj;
	Uint i;

	if (map->map != NULL)
		MAP_FreeNodes(map);
	if (map->layers != NULL)
		FreeLayers(map);
	if (map->cameras != NULL)
		FreeCameras(map);
	
	for (i = 0; i < map->nBlks; i++) {
		FreeModBlk(map, &map->blks[i]);
	}
	free(map->blks);
	MAP_InitModBlks(map);
}

static void
Destroy(void *_Nonnull obj)
{
	MAP *map = obj;

	free(map->layers);
	free(map->cameras);
}

/*
 * Load a map item (creating a new item under node).
 * The map must be locked.
 */
int
MAP_ItemLoad(MAP *map, MAP_Node *node, AG_DataSource *ds)
{
	MAP_Item *mi;
	MAP_ItemClass *miClass;
	enum map_item_type type;
	Uint flags;
	int layer;

	if ((type = (enum map_item_type)AG_ReadUint32(ds)) >= MAP_ITEM_LAST) {
		AG_SetError("Bad item type: %d", type);
		return (-1);
	}
	miClass = mapItemClasses[type];

	flags = (Uint)AG_ReadUint32(ds);
	if ((layer = (int)AG_ReadUint8(ds)) >= MAP_LAYERS_MAX) {
		AG_SetError("Bad layer#: %d", layer);
		return (-1);
	}

	if ((mi = TryMalloc(miClass->size)) == NULL) {
		return (-1);
	}
	if (miClass->init != NULL) {
		miClass->init(mi);
	}
	mi->flags = flags;
	mi->layer = layer;

	if (miClass->load != NULL)
		if (miClass->load(map, mi, ds) == -1)
			return (-1);
	
	/* Transform chain */
	if (RG_TransformChainLoad(ds, &mi->transforms) == -1)
		goto fail;

	return (0);
fail:
	if (mi != NULL) {
		MAP_ItemDestroy(map, mi);
		free(mi);
	}
	return (-1);
}

int
MAP_NodeLoad(MAP *map, AG_DataSource *ds, MAP_Node *node)
{
	Uint i, nItems;
	
	if ((nItems = (Uint)AG_ReadUint16(ds)) > MAP_NODE_ITEMS_MAX) {
		AG_SetErrorS("nItems > max");
		return (-1);
	}
	for (i = 0; i < nItems; i++) {
		if (MAP_ItemLoad(map, node, ds) == -1) {
			MAP_NodeDestroy(map, node);
			MAP_NodeInit(node);
			return (-1);
		}
	}
	return (0);
}

void
MAP_AttachActor(MAP *map, MAP_Actor *a)
{
	if (a->g_map.x < 0 || a->g_map.x >= (int)map->w ||
	    a->g_map.y < 0 || a->g_map.y >= (int)map->h)  {
		Debug(map, "Bad coordinates %d,%d (not attaching %s)\n",
		    a->g_map.x, a->g_map.y, OBJECT(a)->name);
		return;
	}

	AG_ObjectLock(a);

	MAP_PageIn(map, "ACTOR-", a);

	a->type = AG_ACTOR_MAP;
	a->parent = map;
	a->g_map.x0 = a->g_map.x;
	a->g_map.y0 = a->g_map.y;
	a->g_map.x1 = a->g_map.x;
	a->g_map.y1 = a->g_map.y;
	
	if (MAP_ACTOR_OPS(a)->map != NULL) {
		MAP_ACTOR_OPS(a)->map(a, map);
	}
	AG_ObjectUnlock(a);
}

void
MAP_DetachActor(MAP *map, MAP_Actor *a)
{
	AG_ObjectLock(a);

	if (AG_OfClass(map, "MAP:*")) {
		MAP_ActorUnmapTile(a);
		MAP_PageOut(map, "ACTOR-", a);
	}
	a->parent = NULL;

	AG_ObjectUnlock(a);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
    const AG_Version *_Nonnull ver)
{
	MAP *map = obj;
	Uint32 w, h, origin_x, origin_y;
	Uint i, x, y;
	MAP_Actor *a;
	
	map->flags = (Uint)AG_ReadUint32(ds) & AG_MAP_SAVED_FLAGS;
	w = AG_ReadUint32(ds);
	h = AG_ReadUint32(ds);
	origin_x = AG_ReadUint32(ds);
	origin_y = AG_ReadUint32(ds);
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX ||
	    origin_x > MAP_WIDTH_MAX || origin_y > MAP_HEIGHT_MAX) {
		AG_SetErrorS("Invalid map geometry");
		goto fail;
	}
	map->w = (Uint)w;
	map->h = (Uint)h;
	map->xOrigin = (int)origin_x;
	map->yOrigin = (int)origin_y;
	
	/* Read the layer information. */
	if ((map->nLayers = (Uint)AG_ReadUint32(ds)) > MAP_LAYERS_MAX) {
		AG_SetErrorS("Too many layers");
		goto fail;
	}
	if (map->nLayers < 1) {
		AG_SetErrorS("Missing layer0");
		goto fail;
	}
	map->layers = Realloc(map->layers, map->nLayers * sizeof(MAP_Layer));
	for (i = 0; i < map->nLayers; i++) {
		MAP_Layer *lay = &map->layers[i];

		AG_CopyString(lay->name, ds, sizeof(lay->name));
		lay->visible = (int)AG_ReadUint8(ds);
		lay->xinc = (int)AG_ReadSint16(ds);
		lay->yinc = (int)AG_ReadSint16(ds);
		lay->alpha = AG_ReadUint8(ds);
	}
	map->layerCur = (int)AG_ReadUint8(ds);
	map->layerOrigin = (int)AG_ReadUint8(ds);
	
	/* Read the camera information. */
	if ((map->nCameras = (Uint)AG_ReadUint32(ds)) > MAP_CAMERAS_MAX) {
		AG_SetErrorS("Too many cameras");
		goto fail;
	}
	if (map->nCameras < 1) {
		AG_SetErrorS("Missing camera0");
		goto fail;
	}
	map->cameras = Realloc(map->cameras, map->nCameras * sizeof(MAP_Camera));
	for (i = 0; i < map->nCameras; i++) {
		MAP_Camera *cam = &map->cameras[i];

		AG_CopyString(cam->name, ds, sizeof(cam->name));
		cam->flags = (int)AG_ReadUint32(ds);
		if (i > 0 || map->flags & AG_MAP_SAVE_CAM0POS) {
			cam->x = (int)AG_ReadSint32(ds);
			cam->y = (int)AG_ReadSint32(ds);
		}
		cam->alignment = (enum map_camera_alignment)AG_ReadUint8(ds);
		
		if (i > 0 || map->flags & AG_MAP_SAVE_CAM0ZOOM) {
			cam->zoom = (Uint)AG_ReadUint16(ds);
			cam->tilesz = (Uint)AG_ReadUint16(ds);
			cam->pixsz = cam->tilesz / MAPTILESZ;
		} else {
			cam->zoom = 100;
			cam->tilesz = MAPTILESZ;
			cam->pixsz = 1;
		}

		if (i == 0 && (map->flags & AG_MAP_SAVE_CAM0POS) == 0) {
			cam->x = map->xOrigin * cam->tilesz - cam->tilesz/2;
			cam->y = map->yOrigin * cam->tilesz - cam->tilesz/2;
		}
	}

	/* Allocate and load the nodes. */
	if (MAP_AllocNodes(map, map->w, map->h) == -1) {
		goto fail;
	}
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			if (MAP_NodeLoad(map, ds, &map->map[y][x]) == -1)
				goto fail;
		}
	}

	/* Attach the actor objects. */
	TAILQ_FOREACH(a, &map->actors, actors) {
		Debug(map, "Attaching actor %s at %d,%d layer %d\n",
		     OBJECT(a)->name, a->g_map.x, a->g_map.y,
		     a->g_map.l0);
		Debug(a, "Attached to map %s at %d,%d layer %d\n",
		     OBJECT(map)->name, a->g_map.x, a->g_map.y,
		     a->g_map.l0);
		MAP_AttachActor(map, a);
	}
	return (0);
fail:
	return (-1);
}

/* Save a map item. The parent map must be locked. */
void
MAP_ItemSave(MAP *map, MAP_Item *mi, AG_DataSource *ds)
{
	const MAP_ItemClass *miClass = mapItemClasses[mi->type];

	AG_WriteUint32(ds, (Uint32)mi->type);
	AG_WriteUint32(ds, (Uint32)mi->flags);
	AG_WriteUint8(ds, (Uint8)mi->layer);

	RG_TransformChainSave(ds, &mi->transforms);

	if (miClass->save != NULL)
		miClass->save(map, mi, ds);
}

void
MAP_NodeSave(MAP *map, AG_DataSource *ds, MAP_Node *node)
{
	MAP_Item *mi;
	AG_Offset nItemsOffs;
	Uint nItems = 0;

	nItemsOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	TAILQ_FOREACH(mi, &node->items, items) {
		if (mi->flags & MAP_ITEM_NOSAVE) {
			continue;
		}
		MAP_ItemSave(map, mi, ds);
		nItems++;
	}
	AG_WriteUint16At(ds, (Uint16)nItems, nItemsOffs);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	MAP *map = obj;
	Uint i, x,y;
	
	AG_WriteUint32(ds, (Uint32)(map->flags & AG_MAP_SAVED_FLAGS));

	AG_WriteUint32(ds, (Uint32)map->w);
	AG_WriteUint32(ds, (Uint32)map->h);
	AG_WriteUint32(ds, (Uint32)map->xOrigin);
	AG_WriteUint32(ds, (Uint32)map->yOrigin);

	/* Layers */
	AG_WriteUint32(ds, (Uint32)map->nLayers);
	for (i = 0; i < map->nLayers; i++) {
		const MAP_Layer *lay = &map->layers[i];

		AG_WriteString(ds, lay->name);
		AG_WriteUint8(ds, (Uint8)lay->visible);
		AG_WriteSint16(ds, (Sint16)lay->xinc);
		AG_WriteSint16(ds, (Sint16)lay->yinc);
		AG_WriteUint8(ds, lay->alpha);
	}
	AG_WriteUint8(ds, map->layerCur);
	AG_WriteUint8(ds, map->layerOrigin);

	/* Cameras */
	AG_WriteUint32(ds, (Uint32)map->nCameras);
	for (i = 0; i < map->nCameras; i++) {
		MAP_Camera *cam = &map->cameras[i];

		AG_WriteString(ds, cam->name);
		AG_WriteUint32(ds, (Uint32)cam->flags);
		if (i == 0 && (map->flags & AG_MAP_SAVE_CAM0POS)) {
			AG_WriteSint32(ds, (Sint32)cam->x);
			AG_WriteSint32(ds, (Sint32)cam->y);
		}
		AG_WriteUint8(ds, (Uint8)cam->alignment);
		if (i == 0 && (map->flags & AG_MAP_SAVE_CAM0ZOOM)) {
			AG_WriteUint16(ds, (Uint16)cam->zoom);
			AG_WriteUint16(ds, (Uint16)cam->tilesz);
		}
	}

	/* Nodes */
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++)
			MAP_NodeSave(map, ds, &map->map[y][x]);
	}
	return (0);
}

/* XXX 1.4 */
#if 0
/* Render surface s, scaled to rx,ry pixels. */
static void
BlitSurfaceScaled(MAP *_Nonnull map, AG_Surface *_Nonnull s, AG_Rect *_Nonnull rs,
    int rx, int ry, int cam)
{
	int x, y, sx, sy;
	Uint8 r1, g1, b1, a1;
	Uint32 c;
	int tilesz = map->cameras[cam].tilesz;
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
LocateItem(MAP *_Nonnull map, MAP_Node *_Nonnull node, int xOffs, int yOffs,
    int xd, int yd, int ncam)
{
	AG_Rect rExt;
	MAP_Item *mi;

	TAILQ_FOREACH_REVERSE(mi, &node->items, map_itemq, items) {
		MAP_ItemClass *miClass;

		if (mi->layer != map->layerCur) {
			continue;
		}
		miClass = mapItemClasses[mi->type];
		if (miClass->extent != NULL) {
			miClass->extent(map, mi, &rExt, ncam);
			if (xOffs+xd >= rExt.x && xOffs+xd < rExt.x+rExt.w &&
			    yOffs+yd >= rExt.y && yOffs+yd < rExt.y+rExt.h)
				return (mi);
		}
	}
	return (NULL);
}

/* Locate a map item. */
MAP_Item *
MAP_ItemLocate(MAP *map, int xMap, int yMap, int ncam)
{
	MAP_Camera *cam = &map->cameras[ncam];
	MAP_Item *mi;
	const int tileSz = cam->tilesz;
	int x,y, xOffs,yOffs;

	if (ncam < 0 || ncam >= map->nCameras ||
 	    xMap < 0 || yMap < 0 || xMap >= map->w || yMap >= map->h) {
		return (NULL);
	}
	x = xMap / tileSz;
	y = yMap / tileSz;
	xOffs = xMap % tileSz;
	yOffs = yMap % tileSz;

	if ((mi = LocateItem(map, &map->map[y][x], xOffs,yOffs, 0,0, ncam))
	    != NULL) {
		return (mi);
	}

	if (y+1 < map->h) {
		if ((mi = LocateItem(map, &map->map[y+1][x], xOffs, yOffs,
		    0, -cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	if (y-1 >= 0) {
		if ((mi = LocateItem(map, &map->map[y-1][x], xOffs, yOffs,
		    0, +cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	if (x+1 < (int)map->w) {
		if ((mi = LocateItem(map, &map->map[y][x+1], xOffs, yOffs,
		    -cam->tilesz, 0, ncam)) != NULL)
			return (mi);
	}
	if (x-1 >= 0) {
		if ((mi = LocateItem(map, &map->map[y][x-1], xOffs, yOffs,
		    +cam->tilesz, 0, ncam)) != NULL)
			return (mi);
	}

	/* Check diagonal nodes. */
	if (x+1 < map->w && y+1 < map->h) {
		if ((mi = LocateItem(map, &map->map[y+1][x+1], xOffs, yOffs,
		    -cam->tilesz, -cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	if (x-1 >= 0 && y-1 >= 0) {
		if ((mi = LocateItem(map, &map->map[y-1][x-1], xOffs, yOffs,
		    +cam->tilesz, +cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	if (x-1 >= 0 && y+1 < map->h) {
		if ((mi = LocateItem(map, &map->map[y+1][x-1], xOffs, yOffs,
		    +cam->tilesz, -cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	if (x+1 < map->w && y-1 >= 0) {
		if ((mi = LocateItem(map, &map->map[y-1][x+1], xOffs, yOffs,
		    -cam->tilesz, +cam->tilesz, ncam)) != NULL)
			return (mi);
	}
	return (NULL);
}

/*
 * Return the dimensions of a graphical item, and coordinates relative to
 * the origin of the the node.
 */
int
MAP_ItemExtent(MAP *map, const MAP_Item *mi, AG_Rect *rd, int ncam)
{
	MAP_ItemClass *miClass = mapItemClasses[mi->type];

	if (miClass->extent != NULL) {
		miClass->extent(map, (void *)mi, rd, ncam);
		return (0);
	} else {
		return (-1);
	}
}

void
MAP_ItemDraw(MAP *map, MAP_Item *mi, int x, int y, int cam)
{
	/* XXX TODO */
}

/* Create a new undo block at the current level. */
void
MAP_ModBegin(MAP *map)
{
	/* Destroy blocks at upper levels. */
	while (map->nBlks > map->curBlk+1)
		FreeModBlk(map, &map->blks[--map->nBlks]);

	map->blks = Realloc(map->blks, (++map->nBlks) * sizeof(MAP_Mod));
	InitModBlk(&map->blks[map->curBlk++]);
}

void
MAP_ModCancel(MAP *map)
{
	MAP_ModBlk *blk = &map->blks[map->curBlk];

	blk->cancel = 1;
}

void
MAP_ModEnd(MAP *map)
{
	MAP_ModBlk *blk = &map->blks[map->curBlk];
	
	if (blk->nMods == 0 || blk->cancel == 1) {
		FreeModBlk(map, blk);
		map->nBlks--;
		map->curBlk--;
	}
}

void
MAP_Undo(MAP *map)
{
	MAP_ModBlk *blk = &map->blks[map->curBlk];
	Uint i;

	if ((map->curBlk - 1) <= 0)
		return;

	for (i = 0; i < blk->nMods; i++) {
		MAP_Mod *mm = &blk->mods[i];

		switch (mm->type) {
		case AG_MAPMOD_NODECHG:
			{
				MAP_Node *node = &map->map[mm->mm_nodechg.y]
				                          [mm->mm_nodechg.x];
				MAP_Item *mi;
				
				MAP_NodeRemoveAll(map, node, -1);

				TAILQ_FOREACH(mi, &mm->mm_nodechg.node.items,
				    items) {
					MAP_NodeDuplicate(mi, map, node, -1);
				}
			}
			break;
		default:
			break;
		}
	}
	FreeModBlk(map, blk);
	map->nBlks--;
	map->curBlk--;
}

void
MAP_Redo(MAP *map)
{
	/* TODO */
}

void
MAP_ModNodeChg(MAP *map, int x, int y)
{
	MAP_Node *node = &map->map[y][x];
	MAP_ModBlk *blk = &map->blks[map->nBlks - 1];
	MAP_Mod *mm;
	MAP_Item *mi;
	Uint i;

	for (i = 0; i < blk->nMods; i++) {
		mm = &blk->mods[i];
		if (mm->type == AG_MAPMOD_NODECHG &&
		    mm->mm_nodechg.x == x &&
		    mm->mm_nodechg.y == y)
			return;
	}

	blk->mods = Realloc(blk->mods, (blk->nMods + 1)*sizeof(MAP_Mod));
	mm = &blk->mods[blk->nMods++];
	mm->type = AG_MAPMOD_NODECHG;
	mm->mm_nodechg.x = x;
	mm->mm_nodechg.y = y;
	MAP_NodeInit(&mm->mm_nodechg.node);

	TAILQ_FOREACH(mi, &node->items, items)
		MAP_NodeDuplicate(mi, map, &mm->mm_nodechg.node, -1);
}

void
MAP_ModLayerAdd(MAP *map, int l)
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

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("View: %s"), OBJECT(map)->name);
	
	mv = MAP_ViewNew(win, map, 0, NULL, NULL);
	mv->cam = MAP_AddCamera(map, _("View"));
	MAP_ViewSizeHint(mv, 10,10);
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
	MAP *map = MAP_PTR(1);
	MAP_View *mv = MAP_VIEW_PTR(2);

	MAP_Resize(map, msb->xvalue, msb->yvalue);
	AG_PostEvent(mv, "map-resized", NULL);
}

/* Display the list of undo blocks. */
static void
PollUndoBlks(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP *map = MAP_PTR(1);
	Uint i, j;

	AG_TlistClear(tl);
	for (i = 0; i < map->nBlks; i++) {
		const MAP_ModBlk *blk = &map->blks[i];
		AG_TlistItem *it;

		it = AG_TlistAdd(tl, NULL, "%sBlock %d (%d mods)",
		    (i == map->curBlk) ? "*" : "", i, blk->nMods);
		it->depth = 0;
		for (j = 0; j < blk->nMods; j++) {
			const MAP_Mod *mod = &blk->mods[j];
			
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
	MAP *map = mv->map;
	const Uint msbFlags = AG_MSPINBUTTON_HFILL;

	if ((win = AG_WindowNewNamed(0, "MAP_Edit-Parameters-%s",
	    OBJECT(map)->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Map parameters: <%s>"), OBJECT(map)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAdd(nb, _("Map settings"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(ntab, msbFlags, "x", _("Map size: "));
		AG_MSpinbuttonSetRange(msb, 1, MAP_WIDTH_MAX);
		msb->xvalue = map->w;
		msb->yvalue = map->h;
		AG_SetEvent(msb, "mspinbutton-return",
		    ResizeMap, "%p,%p", map, mv);
		
		msb = AG_MSpinbuttonNew(ntab, msbFlags, ",", _("Origin position: "));
		AG_BindInt(msb, "xvalue", &map->xOrigin);
		AG_BindInt(msb, "yvalue", &map->yOrigin);
		AG_MSpinbuttonSetRange(msb, 0, MAP_WIDTH_MAX);

		AG_NumericalNewIntR(ntab, AG_NUMERICAL_HFILL, NULL,
		    _("Origin layer: "), &map->layerOrigin, 0, 255);
	}

	ntab = AG_NotebookAdd(nb, _("View"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(ntab, msbFlags, ",", _("Node offset: "));
		AG_BindInt(msb, "xvalue", &mv->mx);
		AG_BindInt(msb, "yvalue", &mv->my);
		AG_MSpinbuttonSetRange(msb,
		   -MAP_WIDTH_MAX/2, MAP_WIDTH_MAX/2);
	
		/* XXX unsafe */
		msb = AG_MSpinbuttonNew(ntab, msbFlags, ",", _("Camera position: "));
		AG_BindInt(msb, "xvalue", &AGMCAM(mv).x);
		AG_BindInt(msb, "yvalue", &AGMCAM(mv).y);
		
		AG_NumericalNewUintR(ntab, AG_NUMERICAL_HFILL, NULL,
		    _("Zoom factor: "), &AGMCAM(mv).zoom, 1, 100);
		
		AG_NumericalNewInt(ntab, AG_NUMERICAL_HFILL, "px",
		    _("Tile size: "), &AGMTILESZ(mv));
	
		msb = AG_MSpinbuttonNew(ntab, msbFlags, ",", _("Visible offset: "));
		AG_BindInt(msb, "xvalue", &mv->xOffs);
		AG_BindInt(msb, "yvalue", &mv->yOffs);
		AG_MSpinbuttonSetRange(msb, -MAP_TILESZ_MAX, MAP_TILESZ_MAX);
		
		msb = AG_MSpinbuttonNew(ntab, msbFlags, "x", _("Visible area (nodes): "));
		AG_BindUint(msb, "xvalue", &mv->mw);
		AG_BindUint(msb, "yvalue", &mv->mh);
		AG_MSpinbuttonSetRange(msb, 1, MAP_WIDTH_MAX);

		msb = AG_MSpinbuttonNew(ntab, msbFlags, "x", _("Visible area (px): "));
		AG_BindUint(msb, "xvalue", &mv->wVis);
		AG_BindUint(msb, "yvalue", &mv->hVis);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		AG_CheckboxNewInt(ntab, 0, _("Smooth scaling"),
		    &mapSmoothScaling);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Camera: %i"), &mv->cam);
#ifdef THREADS
		AG_LabelNewPolledMT(ntab, AG_LABEL_HFILL, &OBJECT(m)->lock,
		    _("Current layer: %i"), &OBJECT(m)->lock, &m->layerCur);
#else
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Current layer: %i"), &map->layerCur);
#endif
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Cursor position: [%i, %i]"), &mv->cx, &mv->cy);
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Mouse selection: %i (%i+%i,%i+%i)"),
		    &mv->msel.set,
		    &mv->msel.x, &mv->msel.xOffs, &mv->msel.y, &mv->msel.yOffs);
		AG_LabelNewPolled(ntab, AG_LABEL_HFILL,
		    _("Effective selection: %i (%ix%i @ %i,%i)"),
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
	const int state = AG_INT(3);
	MAP_Tool *tool;

	if (state == 0) {
		if (mv->curtool != NULL &&
		    (mv->curtool->ops == &mapInsertOps ||
		     mv->curtool->ops == &mapGInsertOps))
		    	MAP_ViewSelectTool(mv, NULL, NULL);
	} else {
		if (strcmp(it->cat, "tile") == 0) {
			RG_Tile *tile = it->p1;

			/* XXX hack */
			if ((tool = MAP_ViewFindTool(mv, "Insert")) != NULL) {
				MAP_InsertTool *insTool = (MAP_InsertTool *)tool;

				insTool->snap_mode = tile->snap_mode;
				insTool->replace_mode = (insTool->snap_mode ==
				                         RG_SNAP_TO_GRID);

				if (mv->curtool != NULL) {
					MAP_ViewSelectTool(mv, NULL, NULL);
				}
				MAP_ViewSelectTool(mv, tool, mv->map);
				AG_WidgetFocus(mv);
			}
		} else if (strcmp(it->cat, "object") == 0 &&
		    AG_OfClass(it->p1, "MAP_Actor:*")) {
			if ((tool = MAP_ViewFindTool(mv, "Ginsert")) != NULL) {
				if (mv->curtool != NULL) {
					MAP_ViewSelectTool(mv, NULL, NULL);
				}
				MAP_ViewSelectTool(mv, tool, mv->map);
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
	MAP *map = mv->map;
	AG_TlistItem *it;
	MAP_Actor *a;

	AG_TlistClear(tl);
	TAILQ_FOREACH(a, &map->actors, actors) {
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
	MAP *map = MAP_PTR(1);
	AG_TlistItem *it;
	Uint i;

	AG_TlistClear(tl);
	for (i = 0; i < map->nLayers; i++) {
		MAP_Layer *lay = &map->layers[i];

		it = AG_TlistAdd(tl, mapIconLayerEditor.s, "%s%s%s",
		    (i == map->layerCur) ? "[*] " : "", lay->name,
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
	const int state = AG_INT(2);

	lay->visible = state;
}

/* Select a layer for edition. */
static void
SelectLayer(AG_Event *_Nonnull event)
{
	MAP *map = MAP_PTR(1);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);
	const MAP_Layer *layer = ti->p1;
	Uint i;

	for (i = 0; i < map->nLayers; i++) {
		if (&map->layers[i] == layer) {
			map->layerCur = i;
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
	MAP *map = MAP_PTR(1);
	MAP_Layer *lay = AG_PTR(2);
	Uint i, x,y, nlayer;
	
	for (nlayer = 0; nlayer < map->nLayers; nlayer++) {
		if (&map->layers[nlayer] == lay)
			break;
	}
	if (nlayer == map->nLayers) {
		return;
	}
	if (--map->nLayers < 1) {
		map->nLayers = 1;
		return;
	}
	if (map->layerCur <= (int)map->nLayers) {
		map->layerCur = (int)map->nLayers - 1;
	}
	for (i = nlayer; i <= map->nLayers; i++) {
		memcpy(&map->layers[i], &map->layers[i+1], sizeof(MAP_Layer));
	}
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi, *miNext;

			for (mi = TAILQ_FIRST(&node->items);
			     mi != TAILQ_END(&node->items);
			     mi = miNext) {
				miNext = TAILQ_NEXT(mi, items);
				if (mi->layer == nlayer) {
					TAILQ_REMOVE(&node->items, mi, items);
					MAP_ItemDestroy(map, mi);
					free(mi);
				} else if (mi->layer > nlayer) {
					mi->layer--;
				}
			}
		}
	}
}

/* Destroy all items on a given layer. */
static void
ClearLayer(AG_Event *_Nonnull event)
{
	MAP *map = MAP_PTR(1);
	const MAP_Layer *lay = AG_PTR(2);
	Uint x,y, i;
	
	for (i = 0; i < map->nLayers; i++) {
		if (&map->layers[i] == lay)
			break;
	}
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi, *miNext;

			for (mi = TAILQ_FIRST(&node->items);
			     mi != TAILQ_END(&node->items);
			     mi = miNext) {
				miNext = TAILQ_NEXT(mi, items);
				if (mi->layer == i) {
					TAILQ_REMOVE(&node->items, mi, items);
					MAP_ItemDestroy(map, mi);
					free(mi);
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
	MAP *map = MAP_PTR(1);
	MAP_Layer *lay1 = AG_PTR(2), *lay2;
	const int moveDown = AG_INT(3);
	AG_Tlist *tlLayers = AG_TLIST_PTR(4);
	Uint l1, l2;
	Uint x, y;

	for (l1 = 0; l1 < map->nLayers; l1++) {
		if (&map->layers[l1] == lay1)
			break;
	}
	if (l1 == map->nLayers) {
		return;
	}
	if (moveDown) {
		l2 = l1+1;
		if (l2 >= map->nLayers) return;
	} else {
		if (((int)l1 - 1) < 0) return;
		l2 = l1-1;
	}
	lay1 = &map->layers[l1];
	lay2 = &map->layers[l2];
	Strlcpy(tmp, lay1->name, sizeof(tmp));
	Strlcpy(lay1->name, lay2->name, sizeof(lay1->name));
	Strlcpy(lay2->name, tmp, sizeof(lay2->name));

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi;

			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer == l1) {
					mi->layer = l2;
				} else if (mi->layer == l2) {
					mi->layer = l1;
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
	MAP *map = MAP_PTR(1);
	AG_Textbox *tb = AG_TEXTBOX_PTR(2);
	
	AG_TextboxCopyString(tb, name, sizeof(name));

	if (MAP_PushLayer(map, name) != 0) {
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
	int xOffs = AG_INT(4);
	int yOffs = AG_INT(5);
	MAP_Item *mi;
	AG_Window *pwin, *win;
	AG_MSpinbutton *msb;

	if ((mi = MAP_ItemLocate(mv->map, mv->mouse.xmap, mv->mouse.ymap,
	    mv->cam)) == NULL) {
		return;
	}

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Map item"));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 1);

	AG_LabelNew(win, 0, _("Type: %s"), mapItemClasses[mi->type]->name);

	msb = AG_MSpinbuttonNew(win, 0, ",", _("Centering: "));
	AG_BindInt(msb, "xvalue", &mi->data.tile.xCenter);
	AG_BindInt(msb, "yvalue", &mi->data.tile.yCenter);

	msb = AG_MSpinbuttonNew(win, 0, ",", _("Motion: "));
	AG_BindInt(msb, "xvalue", &mi->data.tile.xMotion);
	AG_BindInt(msb, "yvalue", &mi->data.tile.yMotion);

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	msb = AG_MSpinbuttonNew(win, 0, ",", _("Source X,Y: "));
	AG_BindInt(msb, "xvalue", &mi->data.tile.rs.x);
	AG_BindInt(msb, "yvalue", &mi->data.tile.rs.y);

	msb = AG_MSpinbuttonNew(win, 0, "x", _("Source W,H: "));
	AG_BindInt(msb, "xvalue", &mi->data.tile.rs.w);
	AG_BindInt(msb, "yvalue", &mi->data.tile.rs.h);

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	AG_NumericalNewUint8(win, 0, NULL, _("Layer: "), &mi->layer);

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
	const Uint flag = AG_UINT(2);

	if (flag != 0) {
		MAP_ViewStatus(mv, _("Editing attribute 0x%x"), flag);
		mv->mode = MAP_VIEW_EDIT_ATTRS;
		mv->edit_attr = flag;
	} else {
		MAP_ViewStatus(mv, _("Editing map"));
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
	const int tileSz = AGMTILESZ(mv);

	AGMCAM(mv).x = mv->map->xOrigin * tileSz - tileSz/2;
	AGMCAM(mv).y = mv->map->yOrigin * tileSz - tileSz/2;

	MAP_ViewUpdateCamera(mv);
}

/* Detach an actor object. */
static void
DetachActor(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	MAP *map = MAP_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	     strcmp(it->cat, "actor") == 0) {
		MAP_Actor *a = it->p1;
	
		TAILQ_REMOVE(&map->actors, a, actors);
		MAP_DetachActor(map, a);
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
	MAP *map = mv->map;
	Uint x,y;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi, *miNext;

			for (mi = TAILQ_FIRST(&node->items);
			     mi != TAILQ_END(&node->items);
			     mi = miNext) {
				miNext = TAILQ_NEXT(mi, items);
				if (mi->type == MAP_ITEM_TILE &&
				    MAPTILE(mi)->obj == ts)
					MAP_NodeDelItem(map, node, mi);
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
	MAP *map = mv->map;
	Uint x,y;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi, *miNext;
			RG_Tile *ntile;

			for (mi = TAILQ_FIRST(&node->items);
			     mi != TAILQ_END(&node->items);
			     mi = miNext) {
				miNext = TAILQ_NEXT(mi, items);

				if (mi->type == MAP_ITEM_TILE &&
				    RG_LookupTile(MAPTILE(mi)->obj,
				                  MAPTILE(mi)->id,
						  &ntile) == 0 &&
				    (ntile == tile)) {
					MAP_NodeDelItem(map, node, mi);
				}
			}
		}
	}
}

/* Generate the menu that pops up when clicking on a layer. */
static void
CreateLayerMenu(AG_Event *_Nonnull event)
{
	MAP *map = MAP_PTR(1);
	AG_Tlist *tlLayers = AG_TLIST_PTR(2);
	AG_MenuItem *menu = AG_MENU_ITEM_PTR(3);
	MAP_Layer *layer;

	if ((layer = AG_TlistSelectedItemPtr(tlLayers)) == NULL) {
		return;
	}
	AG_MenuAction(menu,
	    layer->visible ? _("Hide layer") : _("Show layer"), NULL,
	    SetLayerVisibility, "%p,%i", layer, !layer->visible);

	AG_MenuAction(menu, _("Delete layer"), agIconTrash.s,
	    DeleteLayer, "%p,%p", map, layer);

	AG_MenuAction(menu, _("Clear layer"), agIconTrash.s,
	    ClearLayer, "%p,%p", map, layer);

	AG_MenuSeparator(menu);

	AG_MenuActionKb(menu, _("Move layer up"), agIconUp.s,
	    AG_KEY_U, AG_KEYMOD_SHIFT,
	    MoveLayer, "%p,%p,%i", map, layer, 0, tlLayers);

	AG_MenuActionKb(menu, _("Move layer down"), agIconDown.s,
	    AG_KEY_D, AG_KEYMOD_SHIFT,
	    MoveLayer, "%p,%p,%i", map, layer, 1, tlLayers);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	MAP *map = obj;
	AG_Window *win;
	AG_Toolbar *toolbar;
	AG_Statusbar *statbar;
	MAP_View *mv;
	AG_Menu *menuWin;
	AG_MenuItem *menu;
	AG_Box *hBox, *vBox;
	AG_Pane *hPane, *vPane;
	Uint flags = MAP_VIEW_GRID;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(map)->name);

	if ((OBJECT(map)->flags & AG_OBJECT_READONLY) == 0)
		flags |= MAP_VIEW_EDIT;
	
	toolbar = AG_ToolbarNew(NULL, AG_TOOLBAR_VERT, 2, 0);
	statbar = AG_StatusbarNew(NULL, 0);
	
	mv = MAP_ViewNew(NULL, map, flags, toolbar, statbar);
	MAP_ViewSizeHint(mv, 10,10);
#if 0
	AG_SetEvent(mv, "mapview-dblclick", EditItemProps, NULL);
#endif

	menuWin = AG_MenuNew(win, AG_MENU_HFILL);
	menu = AG_MenuNode(menuWin->root, _("File"), NULL);
	{
		AG_MenuActionKb(menu, _("Close map"), agIconClose.s,
		    AG_KEY_W, AG_KEYMOD_CTRL, AGWINCLOSE(win));
	}
	
	menu = AG_MenuNode(menuWin->root, _("Edit"), NULL);
	{
		AG_MenuAction(menu, _("Undo"), NULL, Undo, "%p", map);
		AG_MenuAction(menu, _("Redo"), NULL, Redo, "%p", map);

		AG_MenuSeparator(menu);

		AG_MenuAction(menu, _("Map parameters..."), mapIconSettings.s,
		    EditMapParameters, "%p,%p", mv, win);
	}
	
	menu = AG_MenuNode(menuWin->root, _("Attributes"), NULL);
	{
		AG_MenuAction(menu, _("None"), NULL,
		    EditPropMode, "%p,%u", mv, 0);

		AG_MenuAction(menu, _("Walkability"), mapIconWalkable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_BLOCK);
		AG_MenuAction(menu, _("Climbability"), mapIconClimbable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_CLIMBABLE);
		AG_MenuAction(menu, _("Jumpability"), mapIconJumpable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_JUMPABLE);
		AG_MenuAction(menu, _("Slippery"), mapIconSlippery.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_SLIPPERY);
	}

	menu = AG_MenuNode(menuWin->root, _("View"), NULL);
	{
		AG_MenuAction(menu, _("Create view..."), mapIconNewView.s,
		    CreateView, "%p,%p", mv, win);
		AG_MenuAction(menu, _("Center around origin"), mapIconOrigin.s,
		    CenterViewToOrigin, "%p", mv);

		AG_MenuSeparator(menu);

		AG_MenuUintFlags(menu, _("Show grid"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_GRID, 0);
		AG_MenuUintFlags(menu, _("Show background"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_NO_BG, 1);
		AG_MenuUintFlags(menu, _("Show map origin"), mapIconOrigin.s,
		    &mv->flags, MAP_VIEW_SHOW_ORIGIN, 0);
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
				    DetachActor, "%p,%p", mv->objs_tl, map); 
			}
		}
		ntab = AG_NotebookAdd(nb, _("Layers"), AG_BOX_VERT);
		{
			AG_Textbox *tb;

			mv->layers_tl = AG_TlistNew(ntab, AG_TLIST_POLL|
			                                  AG_TLIST_EXPAND);
			AG_TlistSetItemHeight(mv->layers_tl, MAPTILESZ);
			AG_SetEvent(mv->layers_tl, "tlist-poll",
			    PollLayers, "%p", map);
			AG_SetEvent(mv->layers_tl, "tlist-dblclick",
			    SelectLayer, "%p", map);

			mi = AG_TlistSetPopup(mv->layers_tl, "layer");
			AG_MenuSetPollFn(mi, CreateLayerMenu, "%p,%p",
			    map, mv->layers_tl);

			hBox = AG_BoxNew(ntab, AG_BOX_HORIZ, AG_BOX_HFILL);
			tb = AG_TextboxNewS(hBox, 0, _("Name: "));
			AG_SetEvent(tb, "textbox-return",
			    PushLayer, "%p, %p", map, tb);
			AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Push"),
			    PushLayer, "%p, %p", map, tb);
		}
		
		AG_SeparatorNew(hPane->div[0], AG_SEPARATOR_HORIZ);
		
		vBox = AG_BoxNew(hPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
		{
			AG_ObjectAttach(vBox, mv);
		}

		AG_ObjectAttach(hPane->div[1], toolbar);
	}

	menu = AG_MenuNode(menuWin->root, _("Tools"), NULL);
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
		MAP_Tool *tool;
		int i;

		for (i = 0; i < nops; i++) {
			tool = MAP_ViewRegTool(mv, ops[i], map);
			tool->pane = (void *)vPane->div[1];
			AG_MenuAction(menu, _(ops[i]->desc),
			    (ops[i]->icon!=NULL) ? ops[i]->icon->s : NULL,
			    SelectTool, "%p,%p", mv, tool);
		}
	}
	
	AG_ObjectAttach(win, statbar);

	AG_PaneMoveDividerPct(hPane, 40);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 50, 50);

	AG_LabelTextS(mv->status,
	    _("Select a tool or double-click on an element to insert."));

	AG_WidgetFocus(mv);
	return (win);
}

AG_ObjectClass mapClass = {
	"MAP",
	sizeof(MAP),
	{ 12, 0 },
	Init,
	Reset,
	Destroy,
	Load,
	Save,
	Edit
};
