/*	$Csoft: pixmap.h,v 1.5 2005/02/18 11:40:12 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include "begin_code.h"

#define PIXMAP_NAME_MAX	32

enum pixmap_umod_type {
	PIXMAP_PIXEL_REPLACE		/* Single pixel replace */
};

struct pixmap_umod {
	enum pixmap_umod_type type;
	Uint16 x, y;			/* Coordinates of pixel in pixmap */
	Uint32 val;			/* Previous value */
};

struct pixmap_undoblk {
	struct pixmap_umod *umods;	/* Undoable modifications */
	unsigned int	   numods;
};

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	struct tileset *ts;	/* Back pointer to tileset */
	SDL_Surface *su;	/* Pixmap surface */
	SDL_Surface *bg;	/* Saved background (optimization) */
	u_int nrefs;		/* Number of tile references */

	float h, s, v, a;		/* Current editor pixel value */
	enum pixmap_blend_mode {
		PIXMAP_BLEND_SRCALPHA,	/* Blend (specified alpha) */
		PIXMAP_BLEND_DSTALPHA,	/* Blend (destination alpha) */
		PIXMAP_BLEND_MIXALPHA,	/* Blend (mixed src+dest alpha) */
		PIXMAP_NO_BLENDING	/* Overwrite alpha value */
	} blend_mode;

	struct pixmap_undoblk *ublks;	/* Blocks of undoable modifications */
	unsigned int	      nublks;
	unsigned int	     curblk;	/* Current undo block (for redo) */

	TAILQ_ENTRY(pixmap) pixmaps;
};

__BEGIN_DECLS
void		 pixmap_init(struct pixmap *, struct tileset *, int);
void		 pixmap_destroy(struct pixmap *);
struct window	*pixmap_edit(struct tileview *, struct tile_element *);
void		 pixmap_update(struct tileview *, struct tile_element *);
void		 pixmap_scale(struct pixmap *, int, int, int, int);

void pixmap_mousebuttondown(struct tileview *, struct tile_element *, int, int,
			    int);
void pixmap_mousebuttonup(struct tileview *, struct tile_element *, int, int,
			  int);
void pixmap_mousemotion(struct tileview *, struct tile_element *, int, int, int,
			int, int);
int  pixmap_mousewheel(struct tileview *, struct tile_element *, int);

void pixmap_begin_undoblk(struct pixmap *);
void pixmap_undo(struct tileview *, struct tile_element *);
void pixmap_redo(struct tileview *, struct tile_element *);
void pixmap_register_umod(struct pixmap *, enum pixmap_umod_type, Uint16,
                          Uint16, Uint32);
void pixmap_put_pixel(struct tileview *, struct tile_element *, int, int,
                      Uint32);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
