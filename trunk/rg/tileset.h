/*	Public domain	*/

#ifndef _AGAR_RG_TILESET_H_
#define _AGAR_RG_TILESET_H_

struct rg_tileset;
struct rg_tile;
/* struct rg_sketch; */
struct rg_pixmap;
struct rg_feature;
struct rg_anim;
struct rg_texture;

#ifdef _AGAR_INTERNAL
#include <rg/transform.h>
/* #include <vg/vg.h> */
#include <rg/tile.h>
#include <rg/feature.h>
#include <rg/pixmap.h>
/* #include <rg/sketch.h> */
#include <rg/animation.h>
#include <rg/texture.h>
#include <rg/prim.h>
#else
#include <agar/rg/transform.h>
/* #include <agar/vg/vg.h> */
#include <agar/rg/tile.h>
#include <agar/rg/feature.h>
#include <agar/rg/pixmap.h>
/* #include <agar/rg/sketch.h> */
#include <agar/rg/animation.h>
#include <agar/rg/texture.h>
#include <agar/rg/prim.h>
#endif

#include "begin_code.h"

#ifndef RG_TILESZ
#define RG_TILESZ 32
#endif
#define RG_TEMPLATE_NAME_MAX	24
#define RG_TILESET_NAME_MAX	32	/* MAX({TILE,SKETCH,PIXMAP}_NAME_MAX) */
#define RG_TILE_ID_MAX		(0xffffffff-1)
#define RG_TILE_ID_MINREUSE	(0xffff)
#define RG_ANIM_ID_MAX		(0xffffffff-1)
#define RG_ANIM_ID_MINREUSE	(0xffff)

typedef struct rg_tileset {
	struct ag_object obj;
	AG_Mutex lock;
	char tmpl[RG_TEMPLATE_NAME_MAX];
	SDL_PixelFormat *fmt;
	SDL_Surface *icon;
	int flags;

	RG_Tile **tiletbl;		/* Tile ID mappings */
	Uint	ntiletbl;
	RG_Anim **animtbl;		/* Animation ID mappings */
	Uint	nanimtbl;

	AG_TAILQ_HEAD(, rg_tile) tiles;
#if 0
	AG_TAILQ_HEAD(, rg_sketch) sketches;
#endif
	AG_TAILQ_HEAD(, rg_pixmap) pixmaps;
	AG_TAILQ_HEAD(, rg_feature) features;
	AG_TAILQ_HEAD(, rg_anim) animations;
	AG_TAILQ_HEAD(, rg_texture) textures;
} RG_Tileset;

#ifdef DEBUG
#define RGTILE(ts,id) RG_GetTile((ts),(id))
#define RGANIM(ts,id) RG_GetAnim((ts),(id))
#else
#define RGTILE(ts,id) (ts)->tiletbl[(id)]
#define RGANIM(ts,id) (ts)->animtbl[(id)]
#endif

__BEGIN_DECLS
extern AG_ObjectClass rgTilesetClass;

void	 	 RG_InitSubsystem(void);
void		 RG_DestroySubsystem(void);

RG_Tileset	*RG_TilesetNew(void *, const char *, Uint);
RG_Tile		*RG_TilesetFindTile(RG_Tileset *, const char *);
#if 0
RG_Sketch	*RG_TilesetFindSketch(RG_Tileset *, const char *);
#endif
RG_Pixmap	*RG_TilesetFindPixmap(RG_Tileset *, const char *);
RG_Anim		*RG_TilesetFindAnim(RG_Tileset *, const char *);
RG_Pixmap	*RG_TilesetResvPixmap(void *, const char *, const char *);
RG_Tile		*RG_TilesetResvTile(void *, const char *, const char *);

static __inline__ int
RG_LookupTile(RG_Tileset *ts, Uint32 id, RG_Tile **t)
{
	if (id >= ts->ntiletbl || ts->tiletbl[id] == NULL) {
		AG_SetError("%s: no such tile: %u", AGOBJECT(ts)->name,
		    (Uint)id);
		return (-1);
	}
	*t = ts->tiletbl[id];
	return (0);
}
static __inline__ int
RG_LookupAnim(RG_Tileset *ts, Uint32 id, RG_Anim **anim)
{
	if (id >= ts->nanimtbl || ts->animtbl[id] == NULL) {
		AG_SetError("%s: no such anim: %u", AGOBJECT(ts)->name,
		    (Uint)id);
		return (-1);
	}
	*anim = ts->animtbl[id];
	return (0);
}
static __inline__ RG_Tile *
RG_GetTile(RG_Tileset *ts, Uint32 id)
{
	RG_Tile *t;
	if (RG_LookupTile(ts, id, &t) == -1) {
		AG_FatalError("%s", AG_GetError());
	}
	return (t);
}
static __inline__ RG_Anim *
RG_GetAnim(RG_Tileset *ts, Uint32 id)
{
	RG_Anim *a;
	if (RG_LookupAnim(ts, id, &a) == -1) {
		AG_FatalError("%s", AG_GetError());
	}
	return (a);
}

__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILESET_H_ */
