/*	$Csoft: tileset.h,v 1.1 2005/01/05 10:51:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILE_H_
#define _AGAR_BG_TILE_H_
#include "begin_code.h"

#define TILE_NAME_MAX 32

struct tile {
	char name[TILE_NAME_MAX];
	SDL_Surface *su;
	Uint8 flags;
#define TILE_CKEYING	0x01
#define TILE_BLENDING	0x02
	Uint8 used;
	struct tile_feature {
		struct feature *ft;
		int x, y;
		int visible, suppressed;
	} *features;
	u_int nfeatures;
	TAILQ_ENTRY(tile) tiles;
};

__BEGIN_DECLS
void	 	tile_init(struct tile *, const char *, Uint16, Uint16, Uint8);
struct window  *tile_edit(struct tileset *, struct tile *);
struct tile    *tile_insert(struct tileset *, const char *, Uint16, Uint16,
		            Uint8);
void		tile_remove(struct tileset *, struct tile *);
void		tile_destroy(struct tile *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
