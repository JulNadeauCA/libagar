/*	$Csoft: tileset.h,v 1.3 2005/01/31 08:40:35 vedge Exp $	*/
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
#include <engine/rg/prim.h>

#include "begin_code.h"

#define SKETCH_NAME_MAX 32
#define PIXMAP_NAME_MAX	32

struct sketch {
	char name[SKETCH_NAME_MAX];
	struct vg *vg;
	u_int nrefs;
	TAILQ_ENTRY(sketch) sketches;
};

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	SDL_Surface *su;
	u_int nrefs;
	TAILQ_ENTRY(pixmap) pixmaps;
};

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
