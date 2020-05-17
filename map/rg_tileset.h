/*	Public domain	*/

#ifndef _AGAR_RG_TILESET_H_
#define _AGAR_RG_TILESET_H_

#include <agar/gui/surface.h>

struct rg_tileset;
struct rg_tile;
/* struct rg_sketch; */
struct rg_pixmap;
struct rg_feature;
struct rg_texture;

#include <agar/map/rg_transform.h>
/* #include <agar/vg/vg.h> */
#include <agar/map/rg_tile.h>
#include <agar/map/rg_feature.h>
#include <agar/map/rg_pixmap.h>
/* #include <agar/map/rg_sketch.h> */
#include <agar/map/rg_texture.h>
#include <agar/map/rg_prim.h>

#include <agar/map/begin.h>

#ifndef RG_TILESZ
#define RG_TILESZ 16
#endif
#define RG_TEMPLATE_NAME_MAX	24
#define RG_TILESET_NAME_MAX	32	/* MAX({TILE,SKETCH,PIXMAP}_NAME_MAX) */
#define RG_TILE_ID_MAX		(0xffffffff-1)
#define RG_TILE_ID_MINREUSE	(0xffff)

typedef struct rg_tileset {
	struct ag_object obj;
	_Nonnull_Mutex AG_Mutex lock;
	char tmpl[RG_TEMPLATE_NAME_MAX];
	AG_PixelFormat *_Nonnull fmt;	/* Reference pixel format */
	AG_Surface *_Nonnull icon;	/* Thumbnail */
	int flags;

	Uint                         nTileTbl;
	RG_Tile *_Nullable *_Nullable tileTbl;	/* Tile ID mappings */

	AG_TAILQ_HEAD_(rg_tile) tiles;
#if 0
	AG_TAILQ_HEAD_(rg_sketch) sketches;
#endif
	AG_TAILQ_HEAD_(rg_pixmap) pixmaps;
	AG_TAILQ_HEAD_(rg_feature) features;
	AG_TAILQ_HEAD_(rg_texture) textures;
} RG_Tileset;

#define RGTILESET(obj)            ((RG_Tileset *)(obj))
#define RGCTILESET(obj)           ((const RG_Tileset *)(obj))
#define RG_TILESET_SELF()          RGTILESET( AG_OBJECT(0,"RG_Tileset:*") )
#define RG_TILESET_PTR(n)          RGTILESET( AG_OBJECT((n),"RG_Tileset:*") )
#define RG_TILESET_NAMED(n)        RGTILESET( AG_OBJECT_NAMED((n),"RG_Tileset:*") )
#define RG_CONST_TILESET_SELF()   RGCTILESET( AG_CONST_OBJECT(0,"RG_Tileset:*") )
#define RG_CONST_TILESET_PTR(n)   RGCTILESET( AG_CONST_OBJECT((n),"RG_Tileset:*") )
#define RG_CONST_TILESET_NAMED(n) RGCTILESET( AG_CONST_OBJECT_NAMED((n),"RG_Tileset:*") )

#ifdef AG_DEBUG
# define RGTILE(ts,id) RG_GetTile((ts),(id))
#else
# define RGTILE(ts,id) (ts)->tileTbl[(id)]
#endif

__BEGIN_DECLS
extern AG_ObjectClass rgTilesetClass;

void RG_InitSubsystem(void);
void RG_DestroySubsystem(void);

RG_Tileset *_Nonnull RG_TilesetNew(void *_Nullable, const char *_Nullable, Uint);
RG_Tile *_Nullable RG_TilesetFindTile(RG_Tileset *_Nonnull, const char *_Nonnull);
#if 0
RG_Sketch *_Nullable RG_TilesetFindSketch(RG_Tileset *_Nonnull, const char *_Nonnull);
#endif
RG_Pixmap *_Nullable RG_TilesetFindPixmap(RG_Tileset *_Nonnull, const char *_Nonnull);

RG_Pixmap *_Nullable RG_TilesetResvPixmap(void *_Nonnull, const char *_Nonnull,
                                          const char *_Nonnull);
RG_Tile   *_Nullable RG_TilesetResvTile(void *_Nonnull, const char *_Nonnull,
                                        const char *_Nonnull);

static __inline__ AG_Surface *_Nonnull
RG_SurfaceStd(RG_Tileset *_Nonnull ts, Uint w, Uint h, Uint flags)
{
	return AG_SurfaceRGBA(w, h, ts->fmt->BitsPerPixel, flags,
	    ts->fmt->Rmask, ts->fmt->Gmask, ts->fmt->Bmask, ts->fmt->Amask);
}

static __inline__ int
RG_LookupTile(RG_Tileset *_Nonnull ts, Uint32 id,
    RG_Tile *_Nullable *_Nullable t)
{
	if (id >= ts->nTileTbl || ts->tileTbl[id] == NULL) {
		AG_SetError("%s: no such tile: %u", AGOBJECT(ts)->name,
		    (Uint)id);
		return (-1);
	}
	if (t != NULL) {
		*t = ts->tileTbl[id];
	}
	return (0);
}
static __inline__ RG_Tile *_Nonnull
RG_GetTile(RG_Tileset *_Nonnull ts, Uint32 id)
{
	RG_Tile *t;
	if (RG_LookupTile(ts, id, &t) == -1) {
		AG_FatalError(NULL);
	}
	return (t);
}
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_TILESET_H_ */
