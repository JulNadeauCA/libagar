/*	$Csoft: pixmap.h,v 1.15 2005/03/11 08:59:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include "begin_code.h"

#define PIXMAP_NAME_MAX	32

enum pixmap_mod_type {
	PIXMAP_PIXEL_REPLACE		/* Single pixel replace */
};
struct pixmap_mod {
	enum pixmap_mod_type type;
	Uint16 x, y;			/* Coordinates of pixel in pixmap */
	Uint32 val;			/* Previous value */
};
struct pixmap_undoblk {
	struct pixmap_mod *mods;	/* Undoable modifications */
	u_int		  nmods;
};

enum pixmap_blend_mode {
	PIXMAP_OVERLAY_ALPHA,
	PIXMAP_AVERAGE_ALPHA,
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
#define PIXMAP_BRUSH_ONESHOT 0x01	/* Don't mod the same pixel twice
					   in the same pass */
	char px_name[PIXMAP_NAME_MAX];	/* Pixmap reference */
	struct pixmap *px;		/* Resolved pixmap */
	TAILQ_ENTRY(pixmap_brush) brushes;
};

struct pixmap {
	char name[PIXMAP_NAME_MAX];
	int flags;
	int xorig, yorig;		/* Pixmap origin point */
	struct tileset *ts;		/* Back pointer to tileset */
	SDL_Surface *su;		/* Pixmap surface */
	u_int nrefs;			/* Number of tile references */
	struct pixmap_undoblk *ublks;	/* Undo blocks */
	u_int nublks, curblk;

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
struct toolbar	*pixmap_toolbar(struct tileview *, struct tile_element *);
void		 pixmap_update(struct tileview *, struct tile_element *);
void		 pixmap_scale(struct pixmap *, int, int, int, int);

void pixmap_mousebuttondown(struct tileview *, struct tile_element *,
			    int, int, int);
void pixmap_mousebuttonup(struct tileview *, struct tile_element *,
			  int, int, int);
void pixmap_mousemotion(struct tileview *, struct tile_element *,
		        int, int, int, int, int);
int pixmap_mousewheel(struct tileview *, struct tile_element *, int);
void pixmap_keydown(struct tileview *, struct tile_element *, int, int);
void pixmap_keyup(struct tileview *, struct tile_element *, int, int);

void pixmap_begin_undoblk(struct pixmap *);
void pixmap_undo(struct tileview *, struct tile_element *);
void pixmap_redo(struct tileview *, struct tile_element *);
void pixmap_register_mod(struct pixmap *, enum pixmap_mod_type, Uint16,
                          Uint16, Uint32);

int pixmap_put_pixel(struct tileview *, struct tile_element *, int, int,
                     Uint32, int);
void pixmap_apply_brush(struct tileview *, struct tile_element *, int, int,
			Uint32);
Uint32 pixmap_source_pixel(struct tileview *, struct tile_element *, int, int);
void pixmap_source_rgba(struct tileview *, struct tile_element *, int, int,
			Uint8 *, Uint8 *, Uint8 *, Uint8 *);

struct pixmap_brush *pixmap_insert_brush(struct pixmap *,
		                         enum pixmap_brush_type,
					 struct pixmap *);
void		     pixmap_remove_brush(struct pixmap *,
			                 struct pixmap_brush *);

void pixmap_open_menu(struct tileview *, int, int);
void pixmap_close_menu(struct tileview *);

__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_PIXMAP_H_ */
