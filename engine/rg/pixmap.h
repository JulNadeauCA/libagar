/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include "begin_code.h"

#define PIXMAP_NAME_MAX	32

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	struct tileset *ts;
	SDL_Surface *su;
	u_int nrefs;
	TAILQ_ENTRY(pixmap) pixmaps;
};

__BEGIN_DECLS
void		 pixmap_init(struct pixmap *, struct tileset *, int);
void		 pixmap_destroy(struct pixmap *);
struct window	*pixmap_edit(struct tileview *, struct tile_element *);
void		 pixmap_scale(struct pixmap *, int, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
