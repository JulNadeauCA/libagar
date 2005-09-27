/*	$Csoft: tileset.h,v 1.13 2005/09/19 01:25:19 vedge Exp $	*/
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

#include <engine/vg/vg.h>
#include <engine/rg/tile.h>
#include <engine/rg/feature.h>
#include <engine/rg/pixmap.h>
#include <engine/rg/sketch.h>
#include <engine/rg/animation.h>
#include <engine/rg/texture.h>
#include <engine/rg/prim.h>

#include "begin_code.h"

#define TILESET_TEMPLATE_MAX 24

typedef struct rg_tileset {
	struct ag_object obj;
	pthread_mutex_t	lock;
	char template[TILESET_TEMPLATE_MAX];
	SDL_PixelFormat *fmt;
	SDL_Surface *icon;
	int flags;
	Uint32 max_sprites;
	TAILQ_HEAD(, rg_tile) tiles;
	TAILQ_HEAD(, rg_sketch) sketches;
	TAILQ_HEAD(, rg_pixmap) pixmaps;
	TAILQ_HEAD(, rg_feature) features;
	TAILQ_HEAD(, rg_anim) animations;
	TAILQ_HEAD(, rg_texture) textures;
} RG_Tileset;

#define RG_TILESET_NAME_MAX 32	/* MAX({TILE,SKETCH,PIXMAP}_NAME_MAX) */

__BEGIN_DECLS
void		 RG_TilesetInit(void *, const char *);
void		 RG_TilesetReinit(void *);
void		 RG_TilesetDestroy(void *);
int		 RG_TilesetLoad(void *, AG_Netbuf *);
int		 RG_TilesetSave(void *, AG_Netbuf *);
void		*RG_TilesetEdit(void *);

__inline__ int RG_TilesetInsertSprite(RG_Tileset *, SDL_Surface *);

__inline__ RG_Tile	 *RG_TilesetFindTile(RG_Tileset *, const char *);
__inline__ RG_Sketch *RG_TilesetFindSketch(RG_Tileset *, const char *);
__inline__ RG_Pixmap *RG_TilesetFindPixmap(RG_Tileset *, const char *);
__inline__ RG_Anim *RG_TilesetFindAnim(RG_Tileset *,
		                                    const char *);
RG_Pixmap *RG_TilesetResvPixmap(const char *, const char *);
RG_Tile *RG_TilesetResvTile(const char *, const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILESET_H_ */
