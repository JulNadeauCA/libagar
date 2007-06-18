/*	Public domain	*/

#ifndef _AGAR_RG_TILESET_H_
#define _AGAR_RG_TILESET_H_

struct rg_tileset;
struct rg_tile;
struct rg_sketch;
struct rg_pixmap;
struct rg_feature;
struct rg_anim;
struct rg_texture;

#ifdef _AGAR_INTERNAL
#include <rg/transform.h>
#include <vg/vg.h>
#include <rg/tile.h>
#include <rg/feature.h>
#include <rg/pixmap.h>
#include <rg/sketch.h>
#include <rg/animation.h>
#include <rg/texture.h>
#include <rg/prim.h>
#else
#include <agar/rg/transform.h>
#include <agar/vg/vg.h>
#include <agar/rg/tile.h>
#include <agar/rg/feature.h>
#include <agar/rg/pixmap.h>
#include <agar/rg/sketch.h>
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
	char template[RG_TEMPLATE_NAME_MAX];
	SDL_PixelFormat *fmt;
	SDL_Surface *icon;
	int flags;

	RG_Tile **tiletbl;		/* Tile ID mappings */
	Uint	ntiletbl;
	RG_Anim **animtbl;		/* Animation ID mappings */
	Uint	nanimtbl;

	TAILQ_HEAD(, rg_tile) tiles;
	TAILQ_HEAD(, rg_sketch) sketches;
	TAILQ_HEAD(, rg_pixmap) pixmaps;
	TAILQ_HEAD(, rg_feature) features;
	TAILQ_HEAD(, rg_anim) animations;
	TAILQ_HEAD(, rg_texture) textures;
} RG_Tileset;

__BEGIN_DECLS
void	 RG_InitSubsystem(void);
void	 RG_TilesetInit(void *, const char *);
void	 RG_TilesetReinit(void *);
void	 RG_TilesetDestroy(void *);
int	 RG_TilesetLoad(void *, AG_Netbuf *);
int	 RG_TilesetSave(void *, AG_Netbuf *);
void	*RG_TilesetEdit(void *);

__inline__ RG_Tile	*RG_TilesetFindTile(RG_Tileset *, const char *);
__inline__ RG_Sketch	*RG_TilesetFindSketch(RG_Tileset *, const char *);
__inline__ RG_Pixmap	*RG_TilesetFindPixmap(RG_Tileset *, const char *);
__inline__ RG_Anim	*RG_TilesetFindAnim(RG_Tileset *, const char *);
RG_Pixmap		*RG_TilesetResvPixmap(const char *, const char *);
RG_Tile			*RG_TilesetResvTile(const char *, const char *);

__inline__ int		 RG_LookupTile(RG_Tileset *, Uint32, RG_Tile **);
__inline__ int		 RG_LookupAnim(RG_Tileset *, Uint32, RG_Anim **);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILESET_H_ */
