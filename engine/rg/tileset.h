/*	$Csoft: tileset.h,v 1.1 2005/01/05 10:51:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILESET_H_
#define _AGAR_BG_TILESET_H_

struct tileset;
struct tile;
struct sketch;
struct feature;

#include <engine/vg/vg.h>
#include <engine/rg/tile.h>
#include <engine/rg/feature.h>

#include "begin_code.h"

#define SKETCH_NAME_MAX 32

struct sketch {
	char name[SKETCH_NAME_MAX];
	struct vg *vg;
	TAILQ_ENTRY(sketch) sketches;
};

struct tileset {
	struct object obj;
	pthread_mutex_t	lock;
	TAILQ_HEAD(, tile) tiles;
	TAILQ_HEAD(, sketch) sketches;
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
#endif	/* _AGAR_BG_TILESET_H_ */
