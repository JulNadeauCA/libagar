/*	$Csoft: pixmap.h,v 1.2 2005/02/14 07:26:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include "begin_code.h"

#define PIXMAP_NAME_MAX	32

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	struct tileset *ts;		/* Back pointer to tileset */
	SDL_Surface *su;		/* Pixmap surface */
	SDL_Surface *bg;		/* Saved background (optimization) */
	u_int nrefs;			/* Number of tile references */
	float h, s, v, a;		/* Current pixel value */
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
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
