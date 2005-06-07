/*	$Csoft: tileset.h,v 1.9 2005/05/26 06:46:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TILESET_H_
#define _AGAR_RG_TILESET_H_

struct tileset;
struct tile;
struct sketch;
struct pixmap;
struct feature;
struct animation;
struct texture;

#include <engine/vg/vg.h>
#include <engine/rg/tile.h>
#include <engine/rg/feature.h>
#include <engine/rg/pixmap.h>
#include <engine/rg/sketch.h>
#include <engine/rg/animation.h>
#include <engine/rg/texture.h>
#include <engine/rg/prim.h>

#include "begin_code.h"

struct tileset {
	struct object obj;
	pthread_mutex_t	lock;
	SDL_PixelFormat *fmt;
	SDL_Surface *icon;
	int flags;
	Uint32 max_sprites;
	TAILQ_HEAD(, tile) tiles;
	TAILQ_HEAD(, sketch) sketches;
	TAILQ_HEAD(, pixmap) pixmaps;
	TAILQ_HEAD(, feature) features;
	TAILQ_HEAD(, animation) animations;
	TAILQ_HEAD(, texture) textures;
};

#define TILESET_NAME_MAX 32	/* MAX({TILE,SKETCH,PIXMAP}_NAME_MAX) */

__BEGIN_DECLS
struct tileset	 *tileset_new(void *, const char *);
void		  tileset_init(void *, const char *);
void		  tileset_reinit(void *);
void		  tileset_destroy(void *);
int		  tileset_load(void *, struct netbuf *);
int		  tileset_save(void *, struct netbuf *);
struct window	 *tileset_edit(void *);

__inline__ struct tile	 *tileset_find_tile(struct tileset *, const char *);
__inline__ struct sketch *tileset_find_sketch(struct tileset *, const char *);
__inline__ struct pixmap *tileset_find_pixmap(struct tileset *, const char *);
__inline__ struct animation *tileset_find_animation(struct tileset *,
		                                    const char *);
struct pixmap *tileset_resolve_pixmap(const char *, const char *);
struct tile *tileset_resolve_tile(const char *, const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILESET_H_ */
