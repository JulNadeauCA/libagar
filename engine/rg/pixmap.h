/*	$Csoft: pixmap.h,v 1.7 2005/02/21 09:43:00 vedge Exp $	*/
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

enum pixmap_blend_mode {
	PIXMAP_SPECIFIC_ALPHA,
	PIXMAP_SPEC_AND_DEST_ALPHA,
	PIXMAP_BRUSH_ALPHA,
	PIXMAP_BRUSH_AND_DEST_ALPHA,
	PIXMAP_DEST_ALPHA,
	PIXMAP_NO_BLENDING
};

enum pixmap_brush_type {
	PIXMAP_BRUSH_MONO,		/* Monochromatic (use current color) */
	PIXMAP_BRUSH_RGB		/* Replace by brush color */
};

struct pixmap_brush {
	char name[PIXMAP_NAME_MAX];
	enum pixmap_brush_type type;
	int flags;
	int xorig, yorig;			/* Origin of brush */
	char px_name[PIXMAP_NAME_MAX];		/* Pixmap reference */
	struct pixmap *px;			/* Resolved pixmap */
	TAILQ_ENTRY(pixmap_brush) brushes;
};

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	int xorig, yorig;		/* Pixmap origin (dflt to 0,0) */
	struct tileset *ts;		/* Back pointer to tileset */
	SDL_Surface *su;		/* Pixmap surface */
	u_int nrefs;			/* Number of tile references */
	struct pixmap_undoblk *ublks;	/* Undo blocks */
	unsigned int nublks, curblk;

	float h, s, v, a;			/* Current pixel value */
	struct pixmap_brush *curbrush;		/* Current brush */
	enum pixmap_blend_mode blend_mode;	/* Current blending method */
	TAILQ_HEAD(, pixmap_brush) brushes;	/* Brush references */
	TAILQ_ENTRY(pixmap) pixmaps;
};

__BEGIN_DECLS
void		 pixmap_init(struct pixmap *, struct tileset *, int);
void		 pixmap_destroy(struct pixmap *);
int		 pixmap_load(struct pixmap *, struct netbuf *);
void		 pixmap_save(struct pixmap *, struct netbuf *);
struct window	*pixmap_edit(struct tileview *, struct tile_element *);
void		 pixmap_update(struct tileview *, struct tile_element *);
void		 pixmap_scale(struct pixmap *, int, int, int, int);

void pixmap_mousebuttondown(struct tileview *, struct tile_element *,
			    int, int, int);
void pixmap_mousebuttonup(struct tileview *, struct tile_element *,
			  int, int, int);
void pixmap_mousemotion(struct tileview *, struct tile_element *,
		        int, int, int, int, int);
int  pixmap_mousewheel(struct tileview *, struct tile_element *, int);

void pixmap_begin_undoblk(struct pixmap *);
void pixmap_undo(struct tileview *, struct tile_element *);
void pixmap_redo(struct tileview *, struct tile_element *);
void pixmap_register_umod(struct pixmap *, enum pixmap_umod_type, Uint16,
                          Uint16, Uint32);
void pixmap_put_pixel(struct tileview *, struct tile_element *, int, int,
                      Uint32);
void pixmap_apply_brush(struct tileview *, struct tile_element *, int, int,
			Uint32);

struct pixmap_brush *pixmap_insert_brush(struct pixmap *,
		                         enum pixmap_brush_type,
					 struct pixmap *);
void		     pixmap_remove_brush(struct pixmap *,
			                 struct pixmap_brush *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
