/*	$Csoft: tile.h,v 1.6 2005/02/05 02:55:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILE_H_
#define _AGAR_BG_TILE_H_
#include "begin_code.h"

#define TILE_NAME_MAX 32
#define TILE_SIZE_MIN 2
#define TILE_SIZE_MAX 1024

struct tile_feature {
	struct feature *ft;
	int x, y;
	int visible;
	TAILQ_ENTRY(tile_feature) features;
};

struct tile {
	char name[TILE_NAME_MAX];
	SDL_Surface *su;
	Uint8 flags;
#define TILE_SRCCOLORKEY 0x01		/* Colorkey source */
#define TILE_SRCALPHA	 0x02		/* Alpha source */
#define TILE_DIRTY	 0x04		/* Mark for redraw */
	Uint8 used;
	SDL_Color c;
	Uint32 pc;
	struct {
		int w;			/* Line width */
		
	} line;
	TAILQ_HEAD(,tile_feature) features;
	TAILQ_ENTRY(tile) tiles;
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
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
