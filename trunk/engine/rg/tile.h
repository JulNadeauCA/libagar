/*	$Csoft: tile.h,v 1.1 2005/01/13 02:30:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILE_H_
#define _AGAR_BG_TILE_H_
#include "begin_code.h"

#define TILE_NAME_MAX 32

struct tile_feature {
	struct feature *ft;
	int x, y;
	int visible;
};

struct tile {
	char name[TILE_NAME_MAX];
	SDL_Surface *su;
	Uint8 flags;
#define TILE_CKEYING	0x01
#define TILE_BLENDING	0x02
	Uint8 used;
	struct tile_feature *features;
	u_int		    nfeatures;
	TAILQ_ENTRY(tile) tiles;
};

__BEGIN_DECLS
void	 	tile_init(struct tile *, const char *);
void		tile_scale(struct tile *, Uint16, Uint16, Uint8);
struct window  *tile_edit(struct tileset *, struct tile *);
void		tile_destroy(struct tile *);
void		tile_save(struct tile *, struct netbuf *);
int		tile_load(struct tileset *, struct tile *, struct netbuf *);

struct tile_feature *tile_add_feature(struct tile *, void *);
void		     tile_remove_feature(struct tile *, void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
