/*	$Csoft: tile.h,v 1.16 2005/04/21 07:58:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILE_H_
#define _AGAR_BG_TILE_H_
#include "begin_code.h"

#define TILE_NAME_MAX 32
#define TILE_ELEMENT_NAME_MAX 32
#define TILE_SIZE_MIN 2
#define TILE_SIZE_MAX 1024

enum tile_element_type {
	TILE_FEATURE,
	TILE_PIXMAP,
	TILE_SKETCH
};

struct tile_element {
	char name[TILE_ELEMENT_NAME_MAX];
	enum tile_element_type type;
	int visible;
	union {
		struct {
			struct feature *ft;
			int x, y;
		} feature;
		struct {
			struct pixmap *px;
			int x, y;
			int alpha;
		} pixmap;
		struct {
			struct sketch *sk;
			int x, y;
			int alpha;
		} sketch;
	} data;
#define tel_feature data.feature
#define tel_pixmap  data.pixmap
#define tel_sketch  data.sketch
	TAILQ_ENTRY(tile_element) elements;
};

TAILQ_HEAD(tile_elementq, tile_element);

struct tile {
	char name[TILE_NAME_MAX];
	struct tileset *ts;
	SDL_Surface *su;
	Uint32 sprite;			/* Matching sprite index */
	Uint8 flags;
#define TILE_SRCCOLORKEY 0x01		/* Colorkey source */
#define TILE_SRCALPHA	 0x02		/* Alpha source */
#define TILE_DIRTY	 0x04		/* Mark for redraw */
	
	u_int nrefs;
	SDL_Color c;			/* Current color (rgb) */
	Uint32 pc;			/* Current color (pixel value) */
	struct {
		int w;			/* Line width */
		Uint32 stipple;		/* Stipple bitmap pattern */
		enum line_endpoint_style {
			TILE_SQUARE_ENDPOINT,
			TILE_ROUNDED_ENDPOINT
		} endpoint;
	} line;
	struct tile_elementq elements;
	void (*blend_fn)(struct tile *, SDL_Surface *, SDL_Rect *);
	TAILQ_ENTRY(tile) tiles;
};

struct tileview;

__BEGIN_DECLS
void	 	tile_init(struct tile *, struct tileset *, const char *);
void		tile_scale(struct tileset *, struct tile *, Uint16, Uint16,
		           u_int, Uint8);
void		tile_generate(struct tile *);
struct window  *tile_edit(struct tileset *, struct tile *);
void		tile_destroy(struct tile *);
void		tile_save(struct tile *, struct netbuf *);
int		tile_load(struct tileset *, struct tile *, struct netbuf *);

struct tile_element *tile_find_element(struct tile *, enum tile_element_type,
		                       const char *);

struct tile_element *tile_add_pixmap(struct tile *, const char *,
		                     struct pixmap *, int, int);
struct tile_element *tile_add_sketch(struct tile *, const char *,
			             struct sketch *, int, int);
struct tile_element *tile_add_feature(struct tile *, const char *, void *,
				      int, int);

void		     tile_remove_feature(struct tile *, void *, int);
void		     tile_remove_pixmap(struct tile *, struct pixmap *, int);
void		     tile_remove_sketch(struct tile *, struct sketch *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
