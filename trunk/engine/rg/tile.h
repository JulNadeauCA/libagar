/*	$Csoft: tile.h,v 1.2 2005/01/17 02:19:28 vedge Exp $	*/
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
#define TILE_SRCCOLORKEY 0x01
#define TILE_SRCALPHA	 0x02
	Uint8 used;
	struct tile_feature *features;
	u_int		    nfeatures;
	TAILQ_ENTRY(tile) tiles;
};

enum tile_blend_mode {
	TILE_BLEND_DSTALPHA,
	TILE_BLEND_SRCALPHA,
	TILE_BLEND_MIXALPHA
};

__BEGIN_DECLS
void	 	tile_init(struct tile *, const char *);
void		tile_scale(struct tileset *, struct tile *, Uint16, Uint16,
		           Uint8);
void		tile_generate(struct tile *);
struct window  *tile_edit(struct tileset *, struct tile *);
void		tile_destroy(struct tile *);
void		tile_save(struct tile *, struct netbuf *);
int		tile_load(struct tileset *, struct tile *, struct netbuf *);

struct tile_feature *tile_add_feature(struct tile *, void *);
void		     tile_remove_feature(struct tile *, void *);

__inline__ void	tile_put_pixel(struct tile *, int, int, Uint32);
__inline__ void	tile_blend_rgb(struct tile *, int, int, enum tile_blend_mode,
		               Uint8, Uint8, Uint8, Uint8);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
