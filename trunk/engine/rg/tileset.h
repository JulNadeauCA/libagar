/*	$Csoft: tileset.h,v 1.5 2005/02/11 04:50:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TILESET_H_
#define _AGAR_RG_TILESET_H_

struct tileset;
struct tile;
struct sketch;
struct pixmap;
struct feature;

#include <engine/vg/vg.h>
#include <engine/rg/tile.h>
#include <engine/rg/feature.h>
#include <engine/rg/pixmap.h>
#include <engine/rg/sketch.h>
#include <engine/rg/prim.h>

#include "begin_code.h"

struct tileset {
	struct object obj;
	pthread_mutex_t	lock;
	SDL_PixelFormat *fmt;
	SDL_Surface *icon;
	TAILQ_HEAD(, tile) tiles;
	TAILQ_HEAD(, sketch) sketches;
	TAILQ_HEAD(, pixmap) pixmaps;
	TAILQ_HEAD(, feature) features;
};

__BEGIN_DECLS
struct tileset	 *tileset_new(void *, const char *);
void		  tileset_init(void *, const char *);
void		  tileset_reinit(void *);
void		  tileset_destroy(void *);
int		  tileset_load(void *, struct netbuf *);
int		  tileset_save(void *, struct netbuf *);
struct window	 *tileset_edit(void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILESET_H_ */
