/*	$Csoft: tile.h,v 1.8 2005/02/11 04:50:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILE_H_
#define _AGAR_BG_TILE_H_
#include "begin_code.h"

#define TILE_NAME_MAX 32
#define TILE_SIZE_MIN 2
#define TILE_SIZE_MAX 1024

enum tile_element_type {
	TILE_FEATURE,
	TILE_PIXMAP
};

struct tile_element {
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
	} data;
#define tel_feature data.feature
#define tel_pixmap  data.pixmap
	TAILQ_ENTRY(tile_element) elements;
};

TAILQ_HEAD(tile_elementq, tile_element);

struct tile {
	char name[TILE_NAME_MAX];
	struct tileset *ts;
	SDL_Surface *su;
	Uint8 flags;
#define TILE_SRCCOLORKEY 0x01		/* Colorkey source */
#define TILE_SRCALPHA	 0x02		/* Alpha source */
#define TILE_DIRTY	 0x04		/* Mark for redraw */
	Uint8 used;
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
	TAILQ_ENTRY(tile) tiles;
};

struct tileview;

__BEGIN_DECLS
void	 	tile_init(struct tile *, struct tileset *, const char *);
void		tile_scale(struct tileset *, struct tile *, Uint16, Uint16,
		           Uint8);
void		tile_generate(struct tile *);
struct window  *tile_edit(struct tileset *, struct tile *);
void		tile_destroy(struct tile *);
void		tile_save(struct tile *, struct netbuf *);
int		tile_load(struct tileset *, struct tile *, struct netbuf *);

struct tile_element *tile_add_pixmap(struct tile *, struct pixmap *, int, int);
struct tile_element *tile_add_feature(struct tile *, void *, int, int);
void		     tile_remove_feature(struct tile *, void *, int);
void		     tile_remove_pixmap(struct tile *, struct pixmap *, int);

void	tile_open_element(struct tileview *, struct tile_element *,
	                  struct window *);
void	tile_close_element(struct tileview *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_TILE_H_ */
