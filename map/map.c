/*
 * Copyright (c) 2001-2022 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/file_dlg.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_icons.h>
#include <agar/map/map.h>
#include <agar/map/icons_data.h>
#include <agar/map/insert.h>

#include <string.h>
#include <stdlib.h>

int mapSmoothScaling = 0;

MAP_ItemClass *mapItemClasses[MAP_ITEM_LAST] = {
	&mapTileClass,				/* MAP_ITEM_TILE */
	&mapImgClass,				/* MAP_ITEM_IMG */
	&mapLinkClass,				/* MAP_ITEM_LINK */
	NULL					/* MAP_ITEM_PRIVATE */
};

static const MAP_ToolOps *mapToolOps[] = {
	&mapInsertObjOps,
	&mapInsertOps,
	&mapNodeselOps,
	&mapFillOps,
	&mapEraserOps,
	&mapFlipOps,
	&mapInvertOps,
	NULL
};

void
MAP_InitSubsystem(void)
{
	Uint i;

	AG_RegisterNamespace("MAP", "MAP_", "https://libagar.org/");
	AG_RegisterClass(&mapClass);
	AG_RegisterClass(&mapObjectClass);
	AG_RegisterClass(&mapViewClass);

	mapIcon_Init();

	for (i = MAP_ITEM_PRIVATE; i <= MAP_ITEM_LAST; i++)
		mapItemClasses[i] = NULL;
}

void
MAP_DestroySubsystem(void)
{
	AG_UnregisterClass(&mapClass);
	AG_UnregisterClass(&mapObjectClass);
	AG_UnregisterClass(&mapViewClass);
}

void
MAP_NodeInit(MAP_Node *node)
{
	node->flags = MAP_NODE_VALID;
	node->nLocs = 0;
	node->locs = NULL;
	TAILQ_INIT(&node->items);
}

void
MAP_NodeDestroy(MAP *map, MAP_Node *node)
{
	MAP_Item *mi, *miNext;
	Uint i;

	for (mi = TAILQ_FIRST(&node->items);
	     mi != TAILQ_END(&node->items);
	     mi = miNext) {
		miNext = TAILQ_NEXT(mi, items);
		MAP_ItemDestroy(map, mi);
		free(mi);
	}
	for (i = 0; i < node->nLocs; i++) {
		Free(node->locs[i].neigh);
	}
	Free(node->locs);

	node->flags &= ~(MAP_NODE_VALID);
}

void
MAP_ItemInit(void *obj, enum map_item_type type)
{
	MAP_Item *mi = MAPITEM(obj);

	mi->type = type;
	mi->flags = MAP_ITEM_VALID;
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

	if ((mi->flags & MAP_ITEM_VALID) == 0) {
		AG_FatalError("MAP_ItemDestroy: Invalid item");
	}
	mi->flags &= ~(MAP_ITEM_VALID);

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

/*
 * Allocate and initialize the two-dimensional array of MAP_Node's
 * to (w x h) nodes.
 */
int
MAP_AllocNodes(MAP *map, Uint w, Uint h)
{
	Uint x, y;
	MAP_Node **mapNew;
	
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError(_("%ux%u nodes exceed the limit of %ux%u."),
		    w,h, MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	if ((mapNew = TryMalloc(h * sizeof(MAP_Node *))) == NULL) {
		return (-1);
	}
	for (y = 0; y < h; y++) {
		if ((mapNew[y] = TryMalloc(w * sizeof(MAP_Node))) == NULL) {
			for (--y; y >= 0; y--) {
				free(mapNew[y]);
			}
			free(mapNew);
			return (-1);
		}
	}
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++)
			MAP_NodeInit(&mapNew[y][x]);
	}

	AG_ObjectLock(map);
	map->map = mapNew;
	map->w = w;
	map->h = h;
	AG_ObjectUnlock(map);

	return (0);
}

/* Release the node map. */
void
MAP_FreeNodes(MAP *map)
{
	Uint x,y;
	MAP_Node *node;

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
	map->cameras = Realloc(map->cameras, sizeof(MAP_Camera));
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
		AG_SetError(_("%ux%u nodes exceed the limit of %ux%u."),
		    w,h, MAP_WIDTH_MAX, MAP_HEIGHT_MAX);
		return (-1);
	}

	AG_ObjectInitStatic(&mapTmp, &mapClass);

	AG_ObjectLock(map);

	if (MAP_AllocNodes(&mapTmp, map->w, map->h) == -1) {
		goto fail;
	}
	for (y = 0; y < map->h && y < h; y++) {
		for (x = 0; x < map->w && x < w; x++)
			MAP_NodeCopy(&mapTmp, &mapTmp.map[y][x], -1,
			             &map->map[y][x], -1);
	}

	/* Resize the map and restore the original nodes. */
	MAP_FreeNodes(map);
	if (MAP_AllocNodes(map, w,h) == -1) {
		goto fail;
	}
	for (y = 0; y < mapTmp.h && y < map->h; y++) {
		for (x = 0; x < mapTmp.w && x < map->w; x++)
			MAP_NodeCopy(map, &map->map[y][x], -1,
			             &mapTmp.map[y][x], -1);
	}

	/* Clamp the origin point. */
	if (map->xOrigin >= (int)w) { map->xOrigin = (int)w - 1; }
	if (map->yOrigin >= (int)h) { map->yOrigin = (int)h - 1; }

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

	if ((cam->tilesz = cam->zoom*MAP_TILESZ_DEF/100) > MAP_TILESZ_MAX)
		cam->tilesz = MAP_TILESZ_MAX;

	AG_ObjectUnlock(map);
}

/* Create a new map. */
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

/* Initialize a new layer. */
void
MAP_InitLayer(MAP_Layer *lay, const char *name)
{
	Strlcpy(lay->name, name, sizeof(lay->name));
	lay->visible = 1;
	lay->xinc = 1;
	lay->yinc = 1;
	lay->alpha = 255;
}

/* Initialize a new camera. */
void
MAP_InitCamera(MAP_Camera *cam, const char *name)
{
	Strlcpy(cam->name, name, sizeof(cam->name));
	cam->x = 0;
	cam->y = 0;
	cam->flags = 0;
	cam->alignment = MAP_CENTER;
	cam->zoom = 100;
	cam->tilesz = MAP_TILESZ_DEF;
	cam->pixsz = 1;
}

/* Create a new camera. */
int
MAP_AddCamera(MAP *map, const char *name)
{
	int rv;

	AG_ObjectLock(map);

	map->cameras = Realloc(map->cameras, (map->nCameras + 1) *
	                                     sizeof(MAP_Camera));
	MAP_InitCamera(&map->cameras[map->nCameras], name);
	rv = map->nCameras++;

	AG_ObjectUnlock(map);

	return (rv);
}

static void
InitRevision(MAP_Revision *_Nonnull rev)
{
	rev->changes = Malloc(sizeof(MAP_Change));
	rev->nChanges = 0;
	rev->cancel = 0;
}

static void
FreeRevision(MAP *_Nonnull map, MAP_Revision *_Nonnull rev)
{
	Uint i;

	for (i = 0; i < rev->nChanges; i++) {
		MAP_Change *mm = &rev->changes[i];
	
		switch (mm->type) {
		case MAP_CHANGE_NODECHG:
			MAP_NodeDestroy(map, &mm->mm_nodechg.node);
			break;
		default:
			break;
		}
	}
	free(rev->changes);
}

/* Initialize undo levels. */
void
MAP_InitHistory(MAP *map)
{
	map->nChanges = 0;

	map->undo = Malloc(sizeof(MAP_Revision));
	map->redo = Malloc(sizeof(MAP_Revision));
	map->nUndo = 0;
	map->nRedo = 0;
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
	map->nObjs = 0;
	map->objs = Malloc(sizeof(MAP_Object *));
	map->pLibs = NULL;
	map->pMaps = NULL;

	MAP_InitLayer(&map->layers[0], _("Layer 0"));
	MAP_InitCamera(&map->cameras[0], _("Camera 0"));
	MAP_InitHistory(map);

	MAP_AllocNodes(map, 32,32);
	map->xOrigin = 16;
	map->yOrigin = 16;
}

static void
Destroy(void *_Nonnull obj)
{
	MAP *map = obj;
	Uint i;

	for (i = 0; i < map->nUndo; i++) {
		FreeRevision(map, &map->undo[i]);
	}
	free(map->undo);

	free(map->layers);
	free(map->cameras);
	free(map->objs);
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

	AG_ObjectLock(map);

	if (map->nLayers+1 > MAP_LAYERS_MAX) {
		AG_SetError(_("%d layers exceed the limit of %d."),
		    map->nLayers+1, MAP_LAYERS_MAX);
		goto fail;
	}
	map->layers = Realloc(map->layers, (map->nLayers+1)*sizeof(MAP_Layer));
	MAP_InitLayer(&map->layers[map->nLayers], layname);
	map->nLayers++;

	AG_ObjectUnlock(map);
	return (0);
fail:
	AG_ObjectUnlock(map);
	return (-1);
}

/* Remove the last layer. */
void
MAP_PopLayer(MAP *map)
{
	AG_ObjectLock(map);

	if (--map->nLayers < 1)
		map->nLayers = 1;
	
	AG_ObjectUnlock(map);
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
 * Copy map items and objects from mapSrc:nodeSrc to map:node.
 *
 * Copy only those items/objects on layerSrc (or -1 = all layers).
 * Set destination layer to layerDst (or -1 = original layer in mapSrc).
 *
 * Both source and destination maps must be locked.
 */
void
MAP_NodeCopy(MAP *map, MAP_Node *node, int layerDst,
    const MAP_Node *nodeSrc, int layerSrc)
{
	MAP_Item *miSrc;
	Uint i;

	TAILQ_FOREACH(miSrc, &nodeSrc->items, items) {
		if (layerSrc != -1 &&
		    layerSrc != miSrc->layer) {
			continue;
		}
		MAP_DuplicateItem(map,node,layerDst, miSrc);
	}
	for (i = 0; i < nodeSrc->nLocs; i++) {
		const MAP_Location *locSrc = &nodeSrc->locs[i];

		if (layerSrc != -1 &&
		    layerSrc != locSrc->layer) {
			continue;
		}
		MAP_DuplicateLocation(map,node,layerDst, locSrc);
	}
}

/*
 * Duplicate the map item mi onto map:node.
 * Both source and destination maps must be locked.
 */
MAP_Item *
MAP_DuplicateItem(MAP *map, MAP_Node *node, int layer, const MAP_Item *mi)
{
	MAP_Item *miDup;
	RG_Transform *xf, *xfDup;

	/* Allocate a new node item with the same data. */
	miDup = (MAP_Item *)mapItemClasses[mi->type]->duplicate(map, node,
	    (const void *)mi);
	miDup->flags = mi->flags;
	miDup->layer = (layer == -1) ? mi->layer : layer;
	miDup->z = mi->z;
	miDup->h = mi->h;

	TAILQ_FOREACH(xf, &mi->transforms, transforms) {
		xfDup = Malloc(sizeof(RG_Transform));
		RG_TransformInit(xfDup, xf->type, xf->nArgs, xf->args);
		TAILQ_INSERT_TAIL(&miDup->transforms, xfDup, transforms);
	}
	return (miDup);
}

/*
 * Duplicate the MAP_Object location loc onto map:node.
 * Both source and destination maps must be locked.
 */
MAP_Location *
MAP_DuplicateLocation(MAP *map, MAP_Node *node, int layer, const MAP_Location *loc)
{
	MAP_Location *locDup;

	node->locs = Realloc(node->locs, (node->nLocs + 1)*sizeof(MAP_Location));
	locDup = &node->locs[node->nLocs++];
	memcpy(locDup, loc, sizeof(MAP_Location));
	locDup->neigh = NULL;
	locDup->nNeigh = 0;

	if (layer != -1)
		locDup->layer = layer;

	return (locDup);
}

/*
 * Unlink a MAP_Item and release its allocated memory.
 * The map must be locked.
 */
void
MAP_NodeDelItem(MAP *map, MAP_Node *node, MAP_Item *mi)
{
	TAILQ_REMOVE(&node->items, mi, items);
	MAP_ItemDestroy(map, mi);
	free(mi);
}

/*
 * Remove a MAP_Object location from a node by pointer reference.
 * Return 1 on success or 0 if the location does not exist on node.
 * The map must be locked.
 */
int
MAP_NodeDelLocation(MAP *map, MAP_Node *node, MAP_Location *loc)
{
	Uint i;

	for (i = 0; i < node->nLocs; i++) {
		if (&node->locs[i] == loc)
			break;
	}
	if (i == node->nLocs) {
		return (0);
	}
	if (i < node->nLocs - 1) {
		Free(loc->neigh);
	
		memmove(loc, &node->locs[i+1],
		        (node->nLocs - i - 1) * sizeof(MAP_Location));
	}
	node->nLocs--;
	return (1);
}

/*
 * Remove a MAP_Object location from a node by index.
 * Return 1 on success or 0 if the location index is invalid.
 * The map must be locked.
 */
int
MAP_NodeDelLocationAtIndex(MAP *map, MAP_Node *node, int idx)
{
	if (idx < 0 || idx >= node->nLocs) {
		return (0);
	}
	if (idx < node->nLocs - 1) {
		MAP_Location *loc = &node->locs[idx];

		Free(loc->neigh);

		memmove(&node->locs[idx], &node->locs[idx+1],
			(node->nLocs - idx - 1) * sizeof(MAP_Location));
	}
	node->nLocs--;
	return (1);
}

/*
 * Remove all MAP_Items on the given layer (or -1 = on any layer).
 * The map must be locked.
 */
void
MAP_NodeClear(MAP *map, MAP_Node *node, int layer)
{
	MAP_Item *mi, *miNext;
	Uint i;

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

	if (layer == -1) {				/* Fast path */
		for (i = 0; i < node->nLocs; i++) {
			Free(node->locs[i].neigh);
		}
		Free(node->locs);
		node->locs = NULL;
		node->nLocs = 0;
	} else {					/* Selective */
clear_locs:
		for (i = 0; i < node->nLocs; i++) {
			if (node->locs[i].layer == layer &&
			    MAP_NodeDelLocationAtIndex(map, node, i))
				goto clear_locs;
		}
	}
}

/*
 * Move node items from layer1 to layer2.
 * The map must be locked.
 */
void
MAP_NodeSwapLayers(MAP *map, MAP_Node *node, int layer1, int layer2)
{
	MAP_Item *mi;
	Uint i;

	TAILQ_FOREACH(mi, &node->items, items) {
		if (mi->layer == layer1)
			mi->layer = layer2;
	}
	for (i = 0; i < node->nLocs; i++) {
		MAP_Location *loc = &node->locs[i];

		if (loc->layer == layer1)
			loc->layer = layer2;
	}
}

/*
 * Move a node item to the upper layer.
 * The map must be locked.
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
 * The map must be locked.
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
 * The map must be locked.
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
 * The map must be locked.
 */
void
MAP_NodeMoveItemToHead(MAP_Node *node, MAP_Item *mi)
{
	if (mi != TAILQ_FIRST(&node->items)) {
		TAILQ_REMOVE(&node->items, mi, items);
		TAILQ_INSERT_HEAD(&node->items, mi, items);
	}
}

/* Clear the map's history buffer. */
void
MAP_ClearHistory(MAP *map)
{
	Uint i;

	for (i = 0; i < map->nUndo; i++) {
		FreeRevision(map, &map->undo[i]);
	}
	free(map->undo);

	for (i = 0; i < map->nRedo; i++) {
		FreeRevision(map, &map->redo[i]);
	}
	free(map->redo);

	MAP_InitHistory(map);
}

static void
Reset(void *_Nonnull obj)
{
	MAP *map = obj;

	if (map->map != NULL)
		MAP_FreeNodes(map);
	if (map->layers != NULL)
		FreeLayers(map);
	if (map->cameras != NULL)
		FreeCameras(map);

	MAP_ClearHistory(map);
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
	Uint flags, layer;
	float z, h;

	if ((type = (enum map_item_type)AG_ReadUint8(ds)) >= MAP_ITEM_LAST) {
		AG_SetError("Bad item type: %d", type);
		return (-1);
	}
	miClass = mapItemClasses[type];

	if ((layer = (int)AG_ReadUint8(ds)) >= MAP_LAYERS_MAX) {
		AG_SetError("Item%d invalid layer %d", (int)type, layer);
		return (-1);
	}

	flags = (Uint)AG_ReadUint16(ds);
	if ((flags & MAP_ITEM_VALID) == 0) {
		AG_SetError("Item%d invalid flags 0x%x", (int)type, flags);
		return (-1);
	}

	z = AG_ReadFloat(ds);
	h = AG_ReadFloat(ds);
	if (h < 0.0f) {
		AG_SetError("Item%d invalid height %f", (int)type, h);
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
	mi->z = z;
	mi->h = h;

	if (miClass->load != NULL) {
		if (miClass->load(map, mi, ds) == -1)
			return (-1);
	}
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

/*
 * Load a map node.
 * The map must be locked.
 */
int
MAP_NodeLoad(MAP *map, AG_DataSource *ds, MAP_Node *node)
{
	Uint flags, nLocs, nItems, i;
	MAP_Location *locsNew;

	flags = (Uint)AG_ReadUint32(ds);
	if ((flags & MAP_NODE_VALID) == 0) {
		AG_SetError("Node flags invalid (0x%x)", flags);
		return (-1);
	}
	if ((nLocs = (Uint)AG_ReadUint16(ds)) > MAP_NODE_ITEMS_MAX) {
		AG_SetError("Node nLocs invalid (%u)", nLocs);
		return (-1);
	}
	if ((nItems = (Uint)AG_ReadUint16(ds)) > MAP_NODE_ITEMS_MAX) {
		AG_SetError("Node nItems invalid (%u)", nItems);
		return (-1);
	}

	if ((locsNew = TryMalloc(nLocs * sizeof(MAP_Location))) == NULL) {
		return (-1);
	}
	for (i = 0; i < nLocs; i++) {
		MAP_Location *loc = &locsNew[i];
		MAP_Object *mo;
		Uint id;

		id = (Uint)AG_ReadUint32(ds);
		if (id > MAP_OBJECT_ID_MAX) {
			AG_SetError("Node Loc#%u ID invalid (%u)", i, id);
			free(locsNew);
			return (-1);
		}
		AGOBJECT_FOREACH_CLASS(mo, map, map_object, "MAP_Object:*") {
			if (mo->id == id)
				break;
		}
		if (mo == NULL) {
			AG_SetError("Node Loc#%u no such Object #%u", i, id);
			free(locsNew);
			return (-1);
		}
		Debug(map, "Node Loc#%u resolved ID%u => %s\n", i, id, AGOBJECT(mo)->name);
		loc->obj = mo;
		loc->flags = (Uint)AG_ReadUint16(ds);
		loc->x = (int)AG_ReadSint32(ds);
		loc->y = (int)AG_ReadSint32(ds);
		loc->layer = (Uint)AG_ReadUint8(ds);
		loc->z = AG_ReadFloat(ds);
		loc->h = AG_ReadFloat(ds);
		loc->neigh = NULL;		/* Will compute later */
		loc->nNeigh = 0;

		mo->locs = Realloc(mo->locs, (mo->nLocs + 1) * sizeof(MAP_Location *));
		mo->locs[mo->nLocs++] = loc;
	}

	node->flags = flags;
	node->locs = locsNew;
	node->nLocs = nLocs;

	for (i = 0; i < nItems; i++) {
		if (MAP_ItemLoad(map, node, ds) == -1) {
			MAP_NodeDestroy(map, node);
			MAP_NodeInit(node);
			return (-1);
		}
	}
	return (0);
}


static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
    const AG_Version *_Nonnull ver)
{
	MAP *map = obj;
	Uint32 w, h, origin_x, origin_y;
	Uint i, x, y;
	
	map->flags = (Uint)AG_ReadUint32(ds) & MAP_SAVED_FLAGS;
	w = AG_ReadUint32(ds);
	h = AG_ReadUint32(ds);
	if (w > MAP_WIDTH_MAX || h > MAP_HEIGHT_MAX) {
		AG_SetError("Invalid map size (%u x %u)", w,h);
		return (-1);
	}
	origin_x = AG_ReadUint32(ds);
	origin_y = AG_ReadUint32(ds);
	if (origin_x > MAP_WIDTH_MAX || origin_y > MAP_HEIGHT_MAX) {
		AG_SetError("Invalid origin point (%u,%u)", origin_x, origin_y);
		return (-1);
	}
	map->w = (Uint)w;
	map->h = (Uint)h;
	map->xOrigin = (int)origin_x;
	map->yOrigin = (int)origin_y;

	/* Layers */
	if ((map->nLayers = (Uint)AG_ReadUint32(ds)) > MAP_LAYERS_MAX ||
	     map->nLayers < 1) {
		AG_SetError("Invalid layer count (%u)", map->nLayers);
		return (-1);
	}
	if ((map->layers = TryRealloc(map->layers, map->nLayers *
	                              sizeof(MAP_Layer))) == NULL) {
		return (-1);
	}
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
	
	/* Cameras */
	if ((map->nCameras = (Uint)AG_ReadUint32(ds)) > MAP_CAMERAS_MAX ||
	     map->nCameras < 1) {
		AG_SetError("Invalid camera count (%u)", map->nLayers);
		return (-1);
	}
	if ((map->cameras = TryRealloc(map->cameras, map->nCameras *
	                               sizeof(MAP_Camera))) == NULL) {
		return (-1);
	}
	for (i = 0; i < map->nCameras; i++) {
		MAP_Camera *cam = &map->cameras[i];

		AG_CopyString(cam->name, ds, sizeof(cam->name));
		cam->flags = (int)AG_ReadUint32(ds);
		if (i > 0 || map->flags & MAP_SAVE_CAM0POS) {
			cam->x = (int)AG_ReadSint32(ds);
			cam->y = (int)AG_ReadSint32(ds);
		}
		cam->alignment = (enum map_camera_alignment)AG_ReadUint8(ds);
		
		if (i > 0 || map->flags & MAP_SAVE_CAM0ZOOM) {
			cam->zoom = (Uint)AG_ReadUint16(ds);
			cam->tilesz = (Uint)AG_ReadUint16(ds);
			cam->pixsz = cam->tilesz / MAP_TILESZ_DEF;
		} else {
			cam->zoom = 100;
			cam->tilesz = MAP_TILESZ_DEF;
			cam->pixsz = 1;
		}

		if (i == 0 && (map->flags & MAP_SAVE_CAM0POS) == 0) {
			cam->x = map->xOrigin * cam->tilesz - cam->tilesz/2;
			cam->y = map->yOrigin * cam->tilesz - cam->tilesz/2;
		}
	}

	/* Map objects */
	map->nObjs = (Uint)AG_ReadUint32(ds);
	if ((map->objs = TryRealloc(map->objs, map->nObjs *
	                            sizeof(MAP_Object *))) == NULL) {
		return (-1);
	}
	for (i = 0; i < map->nObjs; i++) {
		char name[AG_OBJECT_NAME_MAX];
		char hier[AG_OBJECT_HIER_MAX];
		AG_ObjectClass *C;
		MAP_Object *mo;
		Uint32 skipSize;

		AG_CopyString(name, ds, sizeof(name));
		AG_CopyString(hier, ds, sizeof(hier));
		skipSize = AG_ReadUint32(ds);
#ifdef AG_DEBUG
		Debug(map, "Loading object "
		    AGSI_YEL "%s" AGSI_RST " (<%s>, %u bytes)\n",
		    name, hier, (Uint)skipSize);
#else
		(void)skipSize;
#endif

		if ((C = AG_LoadClass(hier)) == NULL)
			return (-1);

		if ((mo = AG_TryMalloc(C->size)) == NULL) {
			return (-1);
		}
		AG_ObjectInit(mo, C);
		AG_ObjectSetNameS(mo, name);
		if (AG_ObjectUnserialize(mo, ds) == -1) {
			AG_ObjectDestroy(mo);
			return (-1);
		}
		AG_ObjectAttach(map, mo);
		map->objs[i] = mo;
	}

	/* Populated nodes */
	if (MAP_AllocNodes(map, map->w, map->h) == -1) {
		return (-1);
	}
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			if (MAP_NodeLoad(map, ds, &map->map[y][x]) == -1)
				return (-1);
		}
	}
	return (0);
}

/* Save a map item. The parent map must be locked. */
void
MAP_ItemSave(MAP *map, MAP_Item *mi, AG_DataSource *ds)
{
	const MAP_ItemClass *miClass = mapItemClasses[mi->type];

	AG_WriteUint8(ds, (Uint8)mi->type);
	AG_WriteUint8(ds, (Uint8)mi->layer);
	AG_WriteUint16(ds, (Uint16)mi->flags);
	AG_WriteFloat(ds, mi->z);
	AG_WriteFloat(ds, mi->h);

	RG_TransformChainSave(ds, &mi->transforms);

	if (miClass->save != NULL)
		miClass->save(map, mi, ds);
}

void
MAP_NodeSave(MAP *map, AG_DataSource *ds, MAP_Node *node)
{
	MAP_Item *mi;
	AG_Offset nItemsOffs;
	Uint i, nItems = 0;

	AG_WriteUint32(ds, node->flags);
	AG_WriteUint16(ds, (Uint16)node->nLocs);
	nItemsOffs = AG_Tell(ds);
	AG_WriteUint16(ds, 0);

	/* Map object locations */
	for (i = 0; i < node->nLocs; i++) {
		const MAP_Location *loc = &node->locs[i];

		AG_WriteUint32(ds, (Uint32)loc->obj->id);
		AG_WriteUint16(ds, (Uint16)loc->flags);
		AG_WriteSint32(ds, (Sint32)loc->x);
		AG_WriteSint32(ds, (Sint32)loc->y);
		AG_WriteUint8(ds, (Uint8)loc->layer);
		AG_WriteFloat(ds, loc->z);
		AG_WriteFloat(ds, loc->h);
	}

	/* Static map items */
	TAILQ_FOREACH(mi, &node->items, items) {
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
	
	AG_WriteUint32(ds, (Uint32)(map->flags & MAP_SAVED_FLAGS));
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
		if (i == 0 && (map->flags & MAP_SAVE_CAM0POS)) {
			AG_WriteSint32(ds, (Sint32)cam->x);
			AG_WriteSint32(ds, (Sint32)cam->y);
		}
		AG_WriteUint8(ds, (Uint8)cam->alignment);
		if (i == 0 && (map->flags & MAP_SAVE_CAM0ZOOM)) {
			AG_WriteUint16(ds, (Uint16)cam->zoom);
			AG_WriteUint16(ds, (Uint16)cam->tilesz);
		}
	}

	/* Map objects */
	AG_WriteUint32(ds, map->nObjs);
	for (i = 0; i < map->nObjs; i++) {
		MAP_Object *mo = map->objs[i];
		AG_Offset skipSizeOffs;
		
		if (!AG_OBJECT_VALID(mo) || !AG_OfClass(mo, "MAP_Object:*"))
			continue;

		Debug(map, "Saving Object %u in slot %u: "
		           AGSI_YEL "%s" AGSI_RST " (<%s>)\n",
		    mo->id, i, OBJECT(mo)->name, OBJECT_CLASS(mo)->name);

		AG_WriteString(ds, OBJECT(mo)->name);

		if (OBJECT_CLASS(mo)->pvt.libs[0] != '\0') {       /* DSOs? */
			char s[AG_OBJECT_TYPE_MAX];

			Strlcpy(s, OBJECT_CLASS(mo)->hier, sizeof(s));
			Strlcat(s, "@", sizeof(s));
			Strlcat(s, OBJECT_CLASS(mo)->pvt.libs, sizeof(s));
			AG_WriteString(ds, s);
		} else {
			AG_WriteString(ds, OBJECT_CLASS(mo)->hier);
		}

		skipSizeOffs = AG_Tell(ds);
		AG_WriteUint32(ds, 0);

		if (AG_ObjectSerialize(mo, ds) == -1) {
			return (-1);
		}
		AG_WriteUint32At(ds, AG_Tell(ds) - skipSizeOffs, skipSizeOffs);
	}

	/* Populated nodes */
	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++)
			MAP_NodeSave(map, ds, &map->map[y][x]);
	}
	return (0);
}

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

/* Create a new revision. */
void
MAP_BeginRevision(MAP *map)
{
	Debug(map, "New undo level #%d (redo=%d->0)\n", map->nUndo+1, map->nRedo);

	if (map->nRedo > 0) {
		int i;

		for (i = 0; i < map->nRedo; i++) {
			FreeRevision(map, &map->redo[i]);
		}
		map->nRedo = 0;
	}

	map->undo = Realloc(map->undo, (map->nUndo + 1)*sizeof(MAP_Revision));
	InitRevision(&map->undo[map->nUndo++]);
}

/* Abort the current undo level. */
void
MAP_AbortRevision(MAP *map)
{
	MAP_Revision *rev = &map->undo[map->nUndo - 1];

#ifdef AG_DEBUG
	if (map->nUndo == 0) AG_FatalError("AbortRevision");
#endif
	Debug(map, "Aborting undo level #%d\n", map->nUndo - 1);
	rev->cancel = 1;
}

/* Commit the current revision and start a new one. */
void
MAP_CommitRevision(MAP *map)
{
	MAP_Revision *rev;

	if (map->nUndo == 0) {
		MAP_BeginRevision(map);
	}
	rev = &map->undo[map->nUndo - 1];

	if (rev->nChanges == 0 || rev->cancel) {
		Debug(map, "Aborting undo level #%d (%d changes)\n",
		    map->nUndo - 1, rev->nChanges);
		FreeRevision(map, rev);
		map->nUndo--;
	} else {
		Debug(map, "Committing undo level #%d (%d changes)\n",
		    map->nUndo - 1, rev->nChanges);
	}
}

/*
 * Revert the changes effected by the given revision.
 * The map must be locked.
 */
void
MAP_UndoRevision(MAP *map, MAP_Revision *rev, MAP_Revision *undoRedo,
    Uint nUndoRedo)
{
	Uint i;

	for (i = 0; i < rev->nChanges; i++) {
		MAP_Change *chg = &rev->changes[i];
	
		switch (chg->type) {
		case MAP_CHANGE_NODECHG:
		{
			MAP_Node *node = &map->map[chg->mm_nodechg.y]
			                          [chg->mm_nodechg.x];

			Debug(map, "Undo(#%d): Reverting node at "
			           "[" AGSI_BOLD "%d,%d" AGSI_RST "]\n",
			    map->nUndo - 1,
			    chg->mm_nodechg.x,
			    chg->mm_nodechg.y);
			
			MAP_NodeRevision(map,
			    chg->mm_nodechg.x,
			    chg->mm_nodechg.y,
			    undoRedo, nUndoRedo);

			MAP_NodeClear(map, node, -1);
			MAP_NodeCopy(map, node, -1, &chg->mm_nodechg.node, -1);
			break;
		}
		default:
			break;
		}
	}
}

/* Revert map to the state prior to the last revision. */
void
MAP_Undo(MAP *map)
{
	MAP_Revision *revUndo, *revRedo;

	if (map->nUndo < 1) {
		return;
	}
	Debug(map, "Undo (undo level #%d)\n", map->nUndo - 1);

	revUndo = &map->undo[map->nUndo - 1];

	/* Record changes made by MAP_UndoRevision() onto the Redo stack. */
	map->redo = Realloc(map->redo, (map->nRedo + 1)*sizeof(MAP_Change));
	revRedo = &map->redo[map->nRedo++];
	InitRevision(revRedo);

	MAP_UndoRevision(map, revUndo, map->redo, map->nRedo);

	if (revRedo->nChanges == 0) {                /* No changes recorded */
		FreeRevision(map, revRedo);
		map->nRedo--;
	}

	FreeRevision(map, revUndo);
	map->nUndo--;
}

/* Revert the last undone change. */
void
MAP_Redo(MAP *map)
{
	MAP_Revision *revRedo, *revUndo;

	if (map->nRedo < 1)
		return;

	Debug(map, "Redo (undo level #%d, redo level #%d)\n",
	    map->nUndo - 1,
	    map->nRedo - 1);

	revRedo = &map->redo[map->nRedo - 1];

	/* Record changes made by MAP_UndoRevision() back onto the Undo stack. */
	map->undo = Realloc(map->undo, (map->nUndo + 1)*sizeof(MAP_Change));
	revUndo = &map->undo[map->nUndo++];
	InitRevision(revUndo);

	MAP_UndoRevision(map, revRedo, map->undo, map->nUndo);

	if (revUndo->nChanges == 0) {                /* No changes recorded */
		FreeRevision(map, revUndo);
		map->nUndo--;
	}

	FreeRevision(map, revRedo);
	map->nRedo--;
}

/*
 * Record the state of the node at map:[x,y] on the given Undo (or Redo) stack.
 *
 * If a copy of the node already exists at the current undo level then this
 * is a no-op.
 */
void
MAP_NodeRevision(MAP *map, int x, int y, MAP_Revision *undoRedo, Uint nUndoRedo)
{
	MAP_Node *node = &map->map[y][x], *nodeSave;
	MAP_Revision *rev;
	MAP_Change *chg;
	MAP_Item *mi;
	Uint i;

	if (nUndoRedo < 1)
		return;

	Debug(map, "Undo/Redo(#%d): Saving node at [" AGSI_BOLD "%d,%d" AGSI_RST "]\n",
	    nUndoRedo - 1, x,y);

	rev = &undoRedo[nUndoRedo - 1];

	for (i = 0; i < rev->nChanges; i++) {
		chg = &rev->changes[i];
		if (chg->type == MAP_CHANGE_NODECHG &&
		    chg->mm_nodechg.x == x &&
		    chg->mm_nodechg.y == y)
			return;				/* Existing copy */
	}

	rev->changes = Realloc(rev->changes, (rev->nChanges + 1) *
	                                     sizeof(MAP_Change));
	chg = &rev->changes[rev->nChanges++];
	chg->type = MAP_CHANGE_NODECHG;
	chg->mm_nodechg.x = x;
	chg->mm_nodechg.y = y;
	nodeSave = &chg->mm_nodechg.node;
	MAP_NodeInit(nodeSave);

	/* Map Items */
	TAILQ_FOREACH(mi, &node->items, items)
		MAP_DuplicateItem(map,nodeSave,-1, mi);

	/* Object Locations */
	nodeSave->nLocs = node->nLocs;
	nodeSave->locs = Malloc(node->nLocs * sizeof(MAP_Location));
	memcpy(nodeSave->locs, node->locs, node->nLocs * sizeof(MAP_Location));
	for (i = 0; i < nodeSave->nLocs; i++) {
		MAP_Location *locSave = &nodeSave->locs[i];

		locSave->neigh = NULL;
		locSave->nNeigh = 0;
	}
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

	sd.w = MAP_TILESZ_DEF;
	sd.h = MAP_TILESZ_DEF;
	mw = sprite->w/MAP_TILESZ_DEF + 1;
	mh = sprite->h/MAP_TILESZ_DEF + 1;

	fragmap = Malloc(sizeof(MAP));
	AG_ObjectInit(fragmap, &mapClass);
	if (MAP_AllocNodes(fragmap, mw, mh) == -1)
		AG_FatalError(NULL);

	for (y = 0, my = 0; y < sprite->h; y += MAP_TILESZ_DEF, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += MAP_TILESZ_DEF, mx++) {
			AG_Surface *su;
			Uint32 saflags = sprite->flags & (AG_SRCALPHA);
			Uint32 sckflags = sprite->flags & (AG_SRCCOLORKEY);
			Uint8 salpha = sprite->format->alpha;
			Uint32 scolorkey = sprite->format->colorkey;
			MAP_Node *node = &fragmap->map[my][mx];
			Uint32 nsprite;
			int fw = MAP_TILESZ_DEF;
			int fh = MAP_TILESZ_DEF;

			if (sprite->w - x < MAP_TILESZ_DEF)
				fw = sprite->w - x;
			if (sprite->h - y < MAP_TILESZ_DEF)
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
	AG_Window *winParent = AG_WINDOW_PTR(2);
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
	
	AG_WindowAttach(winParent, win);
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

static void
PollHistoryBuffer_Changes(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it,
    MAP *_Nonnull map, MAP_Revision *_Nonnull rev)
{
	static const char *chgTypeNames[] = {
		N_("Edit Node"),
		N_("Add Layer"),
		N_("Remove Layer"),
	};
	Uint j;

	for (j = 0; j < rev->nChanges; j++) {
		MAP_Change *chg = &rev->changes[j];
		AG_TlistItem *it;
			
		switch (chg->type) {
		case MAP_CHANGE_NODECHG:
			it = AG_TlistAdd(tl, NULL,
			    "%s (" AGSI_BOLD "%d,%d" AGSI_RST ")",
			    chgTypeNames[MAP_CHANGE_NODECHG],
			    chg->mm_nodechg.x,
			    chg->mm_nodechg.y);
			break;
		case MAP_CHANGE_LAYERADD:
		case MAP_CHANGE_LAYERDEL:
			it = AG_TlistAdd(tl, NULL, "%s (%d)",
			    chgTypeNames[chg->type],
			    chg->data.layermod.nlayer);
			break;
		default:
			it = AG_TlistAddS(tl, NULL, chgTypeNames[chg->type]);
			break;
		}
		it->p1 = chg;
		it->depth = 1;
	}
}

static void
PollHistoryBuffer(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP *map = MAP_PTR(1);
	AG_TlistItem *it;
	MAP_Revision *rev;
	Uint i;

	AG_TlistClear(tl);
	AG_ObjectLock(map);

	for (i = 0; i < map->nUndo; i++) {
		rev = &map->undo[i];
		it = AG_TlistAdd(tl, agIconDown.s,
		    _("%s" "Undo Level %d (%d changes)"),
		    (i == map->nUndo - 1) ? AGSI_BOLD : "",
		    i, rev->nChanges);
		it->flags |= AG_TLIST_HAS_CHILDREN;
		it->depth = 0;
		it->p1 = rev;

		if (AG_TlistVisibleChildren(tl, it))
			PollHistoryBuffer_Changes(tl, it, map, rev);
	}
	if (map->nUndo == 0) {
		it = AG_TlistAddS(tl, NULL, _("(Empty undo buffer)"));
		it->flags |= AG_TLIST_NO_SELECT;
	}

	for (i = 0; i < map->nRedo; i++) {
		rev = &map->redo[i];
		it = AG_TlistAdd(tl, agIconUp.s,
		    _("%s" "Redo Level %d (%d changes)"),
		    (i == map->nRedo - 1) ? AGSI_BOLD : "",
		    i, rev->nChanges);
		it->flags |= AG_TLIST_HAS_CHILDREN;
		it->depth = 0;
		it->p1 = rev;

		if (AG_TlistVisibleChildren(tl, it))
			PollHistoryBuffer_Changes(tl, it, map, rev);
	}
	if (map->nRedo == 0) {
		it = AG_TlistAddS(tl, NULL, _("(Empty redo buffer)"));
		it->flags |= AG_TLIST_NO_SELECT;
	}

	AG_TlistRestore(tl);
	AG_ObjectUnlock(map);
}

static void
EditMapParameters(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	AG_Window *winParent = AG_WINDOW_PTR(2);
	AG_Window *win;
	AG_MSpinbutton *msb;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	MAP *map = mv->map;
	const Uint msbFlags = AG_MSPINBUTTON_HFILL;

	if ((win = AG_WindowNew(0)) == NULL) {
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
		AG_BindInt(msb, "xvalue", &MAP_CAM(mv).x);
		AG_BindInt(msb, "yvalue", &MAP_CAM(mv).y);
		
		AG_NumericalNewUintR(ntab, AG_NUMERICAL_HFILL, NULL,
		    _("Zoom factor: "), &MAP_CAM(mv).zoom, 1, 100);
		
		AG_NumericalNewInt(ntab, AG_NUMERICAL_HFILL, "px",
		    _("Tile size: "), &MAP_TILESZ(mv));
	
		msb = AG_MSpinbuttonNew(ntab, msbFlags, ",", _("Visible offset: "));
		AG_BindInt(msb, "xvalue", &mv->xOffs);
		AG_BindInt(msb, "yvalue", &mv->yOffs);
		AG_MSpinbuttonSetRange(msb, -MAP_TILESZ_MAX,
		                             MAP_TILESZ_MAX);
		
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

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
EditHistoryBuffer(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	AG_Window *winParent = AG_WINDOW_PTR(2);
	AG_Window *win;
	MAP *map = mv->map;
	AG_Tlist *tl;

	if ((win = AG_WindowNew(0)) == NULL)
		return;

	tl = AG_TlistNew(win, AG_TLIST_POLL | AG_TLIST_EXPAND);
	AG_TlistSetRefresh(tl, 125);
	AG_TlistSizeHint(tl, "<Level XXXXX (XXXX changes)>", 25);
	AG_SetEvent(tl, "tlist-poll", PollHistoryBuffer, "%p", mv->map);

	AG_WindowSetCaption(win, _("%s - History Buffer"), OBJECT(map)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

/* Scan VFS for objects we can use as library items. */
static void
PollLibObjectsFind(AG_Tlist *_Nonnull tl, AG_Object *_Nonnull obj, int depth)
{
	AG_Object *child;
	AG_TlistItem *it;
	
	if (AG_OfClass(obj, "RG_Tileset:*")) {
		RG_Tileset *ts = (RG_Tileset *)obj;
		AG_TlistItem *itTile;
		RG_Tile *tile;

		it = AG_TlistAddS(tl, rgIconTileset.s, obj->name);
		it->depth = depth;
		it->p1 = obj;
		it->cat = "tileset";

		AG_MutexLock(&ts->lock);

		if (!TAILQ_EMPTY(&ts->tiles)) {
			it->flags |= AG_TLIST_HAS_CHILDREN;
		}
		if (AG_TlistVisibleChildren(tl, it)) {
			TAILQ_FOREACH(tile, &ts->tiles, tiles) {
				if (tile->su == NULL) {
					continue;
				}
				itTile = AG_TlistAdd(tl, tile->su,
				    _("Tile %s (%ux%u)"),
				    tile->name, tile->su->w, tile->su->h);
				itTile->depth = depth+1;
				itTile->p1 = tile;
				itTile->cat = "tile";
			}
		}
		AG_MutexUnlock(&ts->lock);
	} else {
		it = AG_TlistAddS(tl, rgIconObject.s, obj->name);
		it->depth = depth;
		it->p1 = obj;
		it->cat = "object";
	}

	if (!TAILQ_EMPTY(&obj->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if (AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(child, &obj->children, cobjs)
			PollLibObjectsFind(tl, child, depth+1);   /* Recurse */
	}
}
static void
PollLibObjects(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP *m = MAP_PTR(1);

	if (m->pLibs == NULL)
		return;

	AG_TlistBegin(tl);

	AG_ObjectLock(m->pLibs);
	PollLibObjectsFind(tl, m->pLibs, 0);
	AG_ObjectUnlock(m->pLibs);

	AG_TlistEnd(tl);
}

/* Select a library object. */
static void
SelectLibObject(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	MAP_Tool *tool;
	const int state = AG_INT(3);

	if (state == 0) {
		if (mv->curtool != NULL &&
		    (mv->curtool->ops == &mapInsertOps ||
		     mv->curtool->ops == &mapInsertObjOps))
		    	MAP_ViewSelectTool(mv, NULL, NULL);
	} else {
		if (strcmp(it->cat, "tile") == 0) {
			RG_Tile *tile = it->p1;

			if ((tool = MAP_ViewFindTool(mv, "Insert")) != NULL) {
				MAP_InsertTool *insTool = (MAP_InsertTool *)tool;

				/* XXX */
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
		    AG_OfClass(it->p1, "MAP_Object:*")) {
			if ((tool = MAP_ViewFindTool(mv, "InsertObj")) != NULL) {
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

static void
PollObjects(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	MAP_View *mv = MAP_VIEW_PTR(1);
	MAP *map = mv->map;
	AG_TlistItem *it;
	MAP_Object *mo;

	AG_TlistClear(tl);

	AGOBJECT_FOREACH_CLASS(mo, map, map_object, "MAP_Object:*") {
		it = AG_TlistAdd(tl, NULL, "%s", OBJECT(mo)->name);
		it->p1 = mo;
		it->cat = "object";
	}

	AG_TlistRestore(tl);
}

static void
SelectObject(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	const int selected = AG_INT(3);
	MAP_Object *mo = it->p1;

	if (!AG_OBJECT_VALID(mo) || !AG_OfClass(mo, "MAP_Object:*"))
		return;

	if (selected) {
		MAP_ViewStatus(mv, _("Selected %s"), OBJECT(mo)->name);
		mo->flags |= MAP_OBJECT_SELECTED;
	} else {
		MAP_ViewStatus(mv, _("Unselected %s"), OBJECT(mo)->name);
		mo->flags &= ~(MAP_OBJECT_SELECTED);
	}

	AG_Redraw(mv);
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
		MAP_Layer *layer = &map->layers[i];

		it = AG_TlistAdd(tl, mapIconLayerEditor.s, "%s%s%s",
		    (i == map->layerCur) ? AGSI_BOLD : "", layer->name,
		    layer->visible ? "" : _(" (hidden)"));
		it->p1 = layer;
		it->cat = "layer";
	}
	AG_TlistRestore(tl);
}

static void
SetLayerVisibility(AG_Event *_Nonnull event)
{
	MAP_Layer *layer = AG_PTR(1);
	const int state = AG_INT(2);
	AG_Tlist *tlLayers = AG_TLIST_PTR(3);

	layer->visible = state;
	AG_TlistRefresh(tlLayers);
}

/* Select a layer for edition. */
static void
SelectLayer(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
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
	AG_TlistRefresh(tl);
}

/*
 * Remove a layer and reassign items to the previous layer.
 * The first layer cannot be deleted.
 */
static void
DeleteLayer(AG_Event *_Nonnull event)
{
	MAP *map = MAP_PTR(1);
	MAP_Layer *pLayer = AG_PTR(2);
	AG_Tlist *tlLayers = AG_TLIST_PTR(3);
	Uint i, x,y, layer;
	
	for (layer = 0; layer < map->nLayers; layer++) {
		if (&map->layers[layer] == pLayer)
			break;
	}
	if (layer == map->nLayers) {		/* No matching layer */
		return;
	}
	if ((map->nLayers - 1) < 1) {		/* Minimum of 1 layer */
		return;
	}
	map->nLayers--;

	if (layer < (map->nLayers - 1))
		memmove(&map->layers[layer], &map->layers[layer+1],
		        (map->nLayers - layer - 1)*sizeof(MAP_Layer));

	if (map->layerCur <= (int)map->nLayers)
		map->layerCur = (int)map->nLayers - 1;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi;

			MAP_NodeClear(map, node, layer);

			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer > layer)
					mi->layer--;
			}
			for (i = 0; i < node->nLocs; i++) {
				MAP_Location *loc = &node->locs[i];

				if (loc->layer > layer)
					loc->layer--;
			}
		}
	}
	AG_TlistRefresh(tlLayers);
}

/* Destroy all items on a given layer. */
static void
ClearLayer(AG_Event *_Nonnull event)
{
	MAP *map = MAP_PTR(1);
	const MAP_Layer *pLayer = AG_PTR(2);
	Uint x,y, layer;
	
	for (layer = 0; layer < map->nLayers; layer++) {
		if (&map->layers[layer] == pLayer)
			break;
	}
	for (y = 0; y < map->h; y++)
		for (x = 0; x < map->w; x++)
			MAP_NodeClear(map, &map->map[y][x], layer);
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
			Uint i;

			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer == l1) {
					mi->layer = l2;
				} else if (mi->layer == l2) {
					mi->layer = l1;
				}
			}
			for (i = 0; i < node->nLocs; i++) {
				MAP_Location *loc = &node->locs[i];

				if (loc->layer == l1) {
					loc->layer = l2;
				} else if (loc->layer == l2) {
					loc->layer = l1;
				}
			}
		}
	}
	AG_TlistSelectPtr(tlLayers, lay2);
}

/* Create a new layer. */
static void
NewLayer(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	
	if (MAP_PushLayer(mv->map, "") != 0) {
		AG_TextMsgFromError();
		return;
	}
	AG_TlistRefresh(mv->layers_tl);
}

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
	const int tileSz = MAP_TILESZ(mv);

	MAP_CAM(mv).x = mv->map->xOrigin * tileSz - tileSz/2;
	MAP_CAM(mv).y = mv->map->yOrigin * tileSz - tileSz/2;

	MAP_ViewUpdateCamera(mv);
}

/* Detach a map object. */
static void
DetachObject(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	     strcmp(it->cat, "object") == 0) {
#ifdef AG_DEBUG
		MAP *map = MAP_PTR(2);
		MAP_Object *mo = it->p1;

		Debug(map, "Detach map object %s...\n", OBJECT(mo)->name);
#endif
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
LayerPopupMenu(AG_Event *_Nonnull event)
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
	    SetLayerVisibility, "%p,%i,%p", layer, !layer->visible, tlLayers);

	AG_MenuAction(menu, _("Delete layer"), agIconTrash.s,
	    DeleteLayer, "%p,%p,%p", map, layer, tlLayers);

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

/* Create a new MAP_Object under Library. */
static void
CreateObject(AG_Event *_Nonnull event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_Object *pLibsRoot = AG_OBJECT_PTR(1);
	AG_ObjectClass *cls = AG_PTR(2);
	AG_Textbox *tbName = AG_TEXTBOX_PTR(3);
	AG_Window *winDlg = AG_WINDOW_PTR(4);
	MAP_View *mvParent = MAP_VIEW_PTR(5);
	MAP_Object *moLib;
	void *obj;

	AG_TextboxCopyString(tbName, name, sizeof(name));
	AG_ObjectDetach(winDlg);

	if ((moLib = obj = TryMalloc(cls->size)) == NULL) {
		AG_TextMsgFromError();
		return;
	}

	AG_ObjectInit(obj, cls);
	if (name[0] == '\0') {
		AG_ObjectGenName(pLibsRoot, cls, name, sizeof(name));
	}
	AG_ObjectSetNameS(obj, name);
	AG_ObjectAttach(pLibsRoot, obj);
	AG_ObjectUnlinkDatafiles(obj);

	AG_PostEvent(obj, "edit-create", NULL);

	if (mvParent->lib_tl)
		AG_TlistRefresh(mvParent->lib_tl);
	
	if (AG_ObjectSave(moLib) == -1)
		AG_TextMsgFromError();
}

static void
CreateObjectDlg(AG_Event *_Nonnull event)
{
	AG_Window *win;
	AG_Object *pLibsRoot = AG_OBJECT_PTR(1);
	AG_ObjectClass *cls = AG_PTR(2);
	AG_Window *winParent = AG_WINDOW_PTR(3);
	MAP_View *mv = MAP_VIEW_PTR(4);
	AG_Box *bo;
	AG_Textbox *tb;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("New %s object"), cls->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNew(bo, 0, _("Class: %s"), cls->hier);
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL, _("Name: "));
		AG_WidgetFocus(tb);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"),
		    CreateObject, "%p%p%p%p%p", pLibsRoot, cls, tb, win, mv);
		AG_SetEvent(tb, "textbox-return",
		    CreateObject, "%p%p%p%p%p", pLibsRoot, cls, tb, win, mv);

		AG_ButtonNewFn(bo, 0, _("Cancel"),
		    AGWINDETACH(win));
	}

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
GenNewObjectMenu(AG_MenuItem *_Nonnull mParent, AG_ObjectClass *_Nonnull cls,
    AG_Object *_Nonnull pLibsRoot, MAP_View *_Nonnull mv,
    AG_Window *_Nonnull winParent, int depth)
{
	AG_MenuItem *mn;

	mn = AG_MenuNode(mParent, (depth == 0) ? _("New Library Object") :
	                                         cls->name, NULL);

	AG_MenuAction(mn, AG_Printf(_("New %s..."), cls->name), NULL,
	    CreateObjectDlg, "%p%p%p%p%p", pLibsRoot, cls, winParent, mv);

	if (!TAILQ_EMPTY(&cls->pvt.sub)) {		/* Subclasses exist? */
		AG_ObjectClass *clsSub;

		AG_MenuSeparator(mn);

		TAILQ_FOREACH(clsSub, &cls->pvt.sub, pvt.subclasses) {
			GenNewObjectMenu(mn, clsSub, pLibsRoot, mv, winParent,
			    depth+1);
		}
	}

}

static void
OpenMapAGM(AG_Event *event)
{
	MAP *mapParent = MAP_PTR(1), *map;
	const char *path = AG_STRING(2);
	AG_Window *win;

	if (path[0] == '\0')
		return;

	if ((map = AG_ObjectNew(mapParent->pMaps, NULL, &mapClass)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	map->pLibs = mapParent->pLibs;			/* Inherit pLibs */

	if (AG_ObjectLoadFromFile(map, path) == -1) {
		AG_TextError("%s: %s", path, AG_GetError());
		return;
	}
	if ((win = mapClass.edit(map)) == NULL) {
		AG_TextMsgFromError();
		AG_ObjectDestroy(map);
		return;
	}
	AG_WindowShow(win);
	
	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Loaded " AGSI_YEL "%s" AGSI_RST " from "
		       AGSI_LEAGUE_GOTHIC "%s" AGSI_RST "OK."),
	    OBJECT(map)->name, path);
}

static void
OpenDlg(AG_Event *event)
{
	MAP *map = MAP_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Open Map or Map Object"));
	fd = AG_FileDlgNewMRU(win, "map-import",
	                      AG_FILEDLG_CLOSEWIN | AG_FILEDLG_LOAD |
	                      AG_FILEDLG_EXPAND | AG_FILEDLG_MASK_EXT);
	AG_FileDlgAddType(fd, _("Agar-Map File"), ".agm", OpenMapAGM, "%p", map);
	AG_FileDlgAddType(fd, _("Agar-Map File"), ".MAP", OpenMapAGM, "%p", map);
	AG_WindowShow(win);
}

static void
SaveMap(AG_Event *event)
{
	MAP *map = MAP_PTR(1);

	if (AG_ObjectSave(map) == -1) {
		AG_TextMsgFromError();
	} else {
		char path[AG_PATHNAME_MAX];

		if (AG_ObjectCopyFilename(map, path, sizeof(path)) == -1) {
			path[0] = '?';
			path[1] = '\0';
		}
		AG_TextTmsg(AG_MSG_INFO, 4000,
		    _("Saved " AGSI_YEL "%s" AGSI_RST " to default ("
			       AGSI_LEAGUE_GOTHIC "%s" AGSI_RST ") OK."),
		    OBJECT(map)->name, path);
	}
}

static void
SaveMapAGM(AG_Event *event)
{
	MAP *map = MAP_PTR(1);
	const char *path = AG_STRING(2);

	if (path[0] == '\0') {
		return;
	}
	if (AG_ObjectSaveToFile(map, path) == -1) {
		AG_TextError("%s: %s", path, AG_GetError());
		return;
	}
	AG_TextTmsg(AG_MSG_INFO, 4000,
	    _("Saved " AGSI_YEL "%s" AGSI_RST " to "
		       AGSI_LEAGUE_GOTHIC "%s" AGSI_RST " OK."),
	    OBJECT(map)->name, path);
}

static void
SaveMapDlg(AG_Event *event)
{
	MAP *map = MAP_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Save %s to..."), OBJECT(map)->name);
	fd = AG_FileDlgNewMRU(win, "map-import",
	                      AG_FILEDLG_CLOSEWIN | AG_FILEDLG_SAVE |
	                      AG_FILEDLG_EXPAND | AG_FILEDLG_MASK_EXT);
	AG_FileDlgAddType(fd, _("Agar-Map File"), ".agm", SaveMapAGM, "%p", map);
	AG_FileDlgAddType(fd, _("Agar-Map File"), ".MAP", SaveMapAGM, "%p", map);
	AG_FileDlgSetFilename(fd, "%s.agm", OBJECT(map)->name);
	AG_WindowShow(win);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	MAP *map = obj;
	AG_Window *win;
	AG_Toolbar *toolbar;
	AG_Statusbar *statbar;
	MAP_View *mv;
	AG_Menu *menu;
	AG_MenuItem *mi;
	AG_Box *vBox;
	AG_Pane *hPane, *vPane;
	Uint flags = MAP_VIEW_GRID;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("%s - Agar Map Editor"), OBJECT(map)->name);

	if ((OBJECT(map)->flags & AG_OBJECT_READONLY) == 0)
		flags |= MAP_VIEW_EDIT;
	
	toolbar = AG_ToolbarNew(NULL, AG_TOOLBAR_VERT, 2, AG_TOOLBAR_STICKY);
	statbar = AG_StatusbarNew(NULL, 0);
	
	mv = MAP_ViewNew(NULL, map, flags, toolbar, statbar);
	MAP_ViewSizeHint(mv, 10,10);
#if 0
	AG_SetEvent(mv, "mapview-dblclick", EditItemProps, NULL);
#endif

	menu = AG_MenuNew(win, AG_MENU_HFILL);
	mi = AG_MenuNode(menu->root, _("File"), NULL);
	{
		GenNewObjectMenu(mi, AGCLASS(&mapObjectClass),
		    map->pLibs, mv, win, 0);

		AG_MenuActionKb(mi, _("Open..."), agIconLoad.s,
		    AG_KEY_O, AG_KEYMOD_CTRL,
		    OpenDlg, "%p", map);

		AG_MenuSeparator(mi);

		AG_MenuActionKb(mi, _("Save..."), agIconSave.s,
		    AG_KEY_S, AG_KEYMOD_CTRL,
		    SaveMap, "%p", map);
		AG_MenuActionKb(mi, _("Save As..."), agIconSave.s,
		    AG_KEY_S, AG_KEYMOD_SHIFT | AG_KEYMOD_CTRL,
		    SaveMapDlg, "%p", map);

		AG_MenuSeparator(mi);

		AG_MenuActionKb(mi, _("Close map"), agIconClose.s,
		    AG_KEY_W, AG_KEYMOD_CTRL, AGWINCLOSE(win));
	}
	mi = AG_MenuNode(menu->root, _("Edit"), NULL);
	{
		AG_MenuAction(mi, _("Undo"), NULL, Undo, "%p", map);
		AG_MenuAction(mi, _("Redo"), NULL, Redo, "%p", map);
		AG_MenuAction(mi, _("History Buffer"), agIconUp.s,
		    EditHistoryBuffer, "%p,%p", mv, win);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Map Parameters..."), mapIconSettings.s,
		    EditMapParameters, "%p,%p", mv, win);
	}
	mi = AG_MenuNode(menu->root, _("Attributes"), NULL);
	{
		AG_MenuAction(mi, _("None"), NULL,
		    EditPropMode, "%p,%u", mv, 0);

		AG_MenuAction(mi, _("Walkability"), mapIconWalkable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_BLOCK);
		AG_MenuAction(mi, _("Climbability"), mapIconClimbable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_CLIMBABLE);
		AG_MenuAction(mi, _("Jumpability"), mapIconJumpable.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_JUMPABLE);
		AG_MenuAction(mi, _("Slippery"), mapIconSlippery.s,
		    EditPropMode, "%p,%u", mv, MAP_ITEM_SLIPPERY);
	}
	mi = AG_MenuNode(menu->root, _("View"), NULL);
	{
		AG_MenuAction(mi, _("Create View..."), mapIconNewView.s,
		    CreateView, "%p,%p", mv, win);
		AG_MenuAction(mi, _("Center Around Origin"), mapIconOrigin.s,
		    CenterViewToOrigin, "%p", mv);

		AG_MenuSeparator(mi);

		AG_MenuUintFlags(mi, _("Show Grid"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_GRID, 0);
		AG_MenuUintFlags(mi, _("Show Background"), mapIconGrid.s,
		    &mv->flags, MAP_VIEW_NO_BG, 1);
		AG_MenuUintFlags(mi, _("Show Origin"), mapIconOrigin.s,
		    &mv->flags, MAP_VIEW_SHOW_ORIGIN, 0);
	}
	
	hPane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	AG_PaneSetDivisionPacking(hPane, 1, AG_BOX_HORIZ);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Tlist *tl;

		vPane = AG_PaneNewVert(hPane->div[0], AG_PANE_EXPAND);
		nb = AG_NotebookNew(vPane->div[0], AG_NOTEBOOK_EXPAND);
		ntab = AG_NotebookAdd(nb, _("Library"), AG_BOX_VERT);
		{
			tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND |
			                       AG_TLIST_EXPAND_NODES);
			AG_TlistSetRefresh(tl, -1);
			AG_SetEvent(tl, "tlist-poll", PollLibObjects, "%p", map);
			AG_SetEvent(tl, "tlist-changed", SelectLibObject, "%p", mv);
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
			tl = AG_TlistNew(ntab, AG_TLIST_POLL | AG_TLIST_EXPAND |
			                       AG_TLIST_MULTI);
			AG_SetEvent(tl, "tlist-poll", PollObjects, "%p", mv);
			AG_SetEvent(tl, "tlist-changed", SelectObject, "%p", mv);
			mv->objs_tl = tl;
			WIDGET(tl)->flags &= ~(AG_WIDGET_FOCUSABLE);
			
			mi = AG_TlistSetPopup(mv->objs_tl, "object");
			{
				AG_MenuAction(mi, _("Detach object"),
				    agIconTrash.s,
				    DetachObject, "%p,%p", mv->objs_tl, map); 
			}
		}
		ntab = AG_NotebookAdd(nb, _("Layers"), AG_BOX_VERT);
		{
			mv->layers_tl = AG_TlistNew(ntab, AG_TLIST_POLL|
			                                  AG_TLIST_EXPAND);
			AG_TlistSetItemHeight(mv->layers_tl, MAP_TILESZ_DEF);
			AG_SetEvent(mv->layers_tl, "tlist-poll",
			    PollLayers, "%p", map);
			AG_SetEvent(mv->layers_tl, "tlist-dblclick",
			    SelectLayer, "%p", map);

			mi = AG_TlistSetPopup(mv->layers_tl, "layer");
			AG_MenuSetPollFn(mi, LayerPopupMenu, "%p,%p",
			    map, mv->layers_tl);

			AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("New Layer"),
			    NewLayer, "%p", mv);
		}
		
		AG_SeparatorNew(hPane->div[0], AG_SEPARATOR_HORIZ);
		
		vBox = AG_BoxNew(hPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
		{
			AG_ObjectAttach(vBox, mv);
		}

		AG_ObjectAttach(hPane->div[1], toolbar);
		AG_PaneMoveDividerPct(vPane, 30);
	}

	mi = AG_MenuNode(menu->root, _("Tools"), NULL);
	{
		const MAP_ToolOps **pTool;

		for (pTool = mapToolOps;
		    *pTool != NULL;
		     pTool++) {
			const MAP_ToolOps *toolOps = *pTool;
			MAP_Tool *tool;

			tool = MAP_ViewRegTool(mv, toolOps, map);
			tool->pane = (void *)vPane->div[1];
			AG_MenuAction(mi, _(toolOps->desc),
			    (toolOps->icon != NULL) ? toolOps->icon->s : NULL,
			    SelectTool, "%p,%p", mv, tool);
		}
	}
	
	AG_ObjectAttach(win, statbar);

	AG_PaneMoveDividerPct(hPane, 20);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 50, 50);

	AG_LabelTextS(mv->status,
	    _("Select a tool or double-click on an element to insert."));

	AG_WidgetFocus(mv);
	return (win);
}

AG_ObjectClass mapClass = {
	"MAP",
	sizeof(MAP),
	{ 12, 1 },
	Init,
	Reset,
	Destroy,
	Load,
	Save,
	Edit
};
