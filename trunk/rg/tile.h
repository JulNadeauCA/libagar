/*	Public domain	*/

#ifndef _AGAR_RG_TILE_H_
#define _AGAR_RG_TILE_H_
#include "begin_code.h"

#define RG_TILE_NAME_MAX	 32
#define RG_TILE_CLASS_MAX	 16
#define RG_TILE_ELEMENT_NAME_MAX 32
#define RG_TILE_SIZE_MIN	 2
#define RG_TILE_SIZE_MAX	 1024

struct ag_window;

enum rg_tile_element_type {
	RG_TILE_FEATURE,
	RG_TILE_PIXMAP,
	RG_TILE_SKETCH
};

enum rg_snap_mode {
	RG_SNAP_NONE,
	RG_SNAP_TO_GRID
};

struct rg_tile_variant;
struct rg_tileview;

typedef struct rg_tile_element {
	char name[RG_TILE_ELEMENT_NAME_MAX];
	enum rg_tile_element_type type;
	int visible;
	union {
		struct {
			struct rg_feature *ft;
			int x, y;
		} feature;
		struct {
			struct rg_pixmap *px;
			int x, y;
			int alpha;
		} pixmap;
		struct {
			struct rg_sketch *sk;
			int x, y;
			int alpha;
			float scale;
		} sketch;
	} data;
#ifndef _AGAR_RG_PUBLIC_H_
#define tel_feature data.feature
#define tel_pixmap  data.pixmap
#define tel_sketch  data.sketch
#endif
	TAILQ_ENTRY(rg_tile_element) elements;
} RG_TileElement;

TAILQ_HEAD(rg_tile_elementq, rg_tile_element);

typedef struct rg_tile {
	char name[RG_TILE_NAME_MAX];	/* User description */
	char clname[RG_TILE_CLASS_MAX];	/* Category (application-specific) */
	Uint32 main_id;			/* Default ID mapping */
	struct rg_tileset *ts;
	SDL_Surface *su;		/* Generated surface */
#ifdef HAVE_OPENGL
	Uint texture;			/* Cached texture */
	float texcoords[4];
#endif
	int xOrig, yOrig;		/* Origin point */
	enum rg_snap_mode snap_mode;	/* Snapping mode (edition) */
	Uint *attrs;			/* Node attributes (edition) */
	int *layers;			/* Node layer offsets (edition) */
	Uint nw, nh;			/* Node dimensions */
	Uint8 flags;
#define RG_TILE_SRCCOLORKEY	0x01	/* Colorkey source */
#define RG_TILE_SRCALPHA	0x02	/* Alpha source */
#define RG_TILE_DIRTY		0x04	/* Mark for redraw */
	Uint nrefs;			/* Reference count */
	SDL_Color c;			/* Current RGB color (edition) */
	Uint32 pc;			/* Current pixel value (edition) */
	struct {
		int w;			/* Line width */
		Uint32 stipple;		/* Stipple bitmap pattern */
		enum rg_line_endpoint_style {
			RG_TILE_SQUARE_ENDPOINT,
			RG_TILE_ROUNDED_ENDPOINT
		} endpoint;
	} line;
	struct rg_tile_elementq elements;	/* Elements to combine */

	/* Pixel blending function */
	void (*blend_fn)(struct rg_tile *, SDL_Surface *, SDL_Rect *);

	SLIST_HEAD(,rg_tile_variant) vars;	/* Cached variants */
	TAILQ_ENTRY(rg_tile) tiles;
} RG_Tile;

/* Cached, transformed tile variant */
typedef struct rg_tile_variant {
	RG_TransformChain transforms;	/* Applied transforms */
	SDL_Surface *su;		/* Cached resulting surface */
#ifdef HAVE_OPENGL
	Uint texture;			/* Cached texture */
	float texcoords[4];
#endif
	Uint32 last_drawn;		/* Time last draw occured */
	SLIST_ENTRY(rg_tile_variant) vars;
} RG_TileVariant;

#define RG_TILE_ATTR2(t,x,y) (t)->attrs[(y)*(t)->nw + (x)]
#define RG_TILE_LAYER2(t,x,y) (t)->layers[(y)*(t)->nw + (x)]
#define RG_TILE_ATTRS(t) (AG_SPRITE((t)->ts,(t)->s).attrs)
#define RG_TILE_LAYERS(t) (AG_SPRITE((t)->ts,(t)->s).layers)

/* For Agar-MAP */
#define RG_TILE_BLOCK		0x001
#define RG_TILE_CLIMBABLE	0x002
#define RG_TILE_SLIPPERY	0x004
#define RG_TILE_JUMPABLE	0x008

__BEGIN_DECLS
extern const char *rgTileSnapModes[];

RG_Tile        *RG_TileNew(RG_Tileset *, const char *, Uint16, Uint16, Uint);
void	 	RG_TileInit(RG_Tile *, struct rg_tileset *, const char *);
void		RG_TileScale(struct rg_tileset *, RG_Tile *, Uint16, Uint16,
		             Uint, Uint8);
void		RG_TileGenerate(RG_Tile *);
struct ag_window *RG_TileEdit(struct rg_tileset *, RG_Tile *);
void		RG_TileDestroy(RG_Tile *);
void		RG_TileSave(RG_Tile *, AG_Netbuf *);
int		RG_TileLoad(RG_Tile *, AG_Netbuf *);
void		RG_TileOpenMenu(struct rg_tileview *, int, int);
void		RG_TileCloseMenu(struct rg_tileview *);

RG_TileElement *RG_TileFindElement(RG_Tile *, enum rg_tile_element_type,
		                   const char *);

RG_TileElement *RG_TileAddPixmap(RG_Tile *, const char *,
		                 struct rg_pixmap *, int, int);
RG_TileElement *RG_TileAddSketch(RG_Tile *, const char *,
			         struct rg_sketch *, int, int);
RG_TileElement *RG_TileAddFeature(RG_Tile *, const char *, void *,
				  int, int);

void		RG_TileDelFeature(RG_Tile *, void *, int);
void		RG_TileDelPixmap(RG_Tile *, struct rg_pixmap *, int);
void		RG_TileDelSketch(RG_Tile *, struct rg_sketch *, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TILE_H_ */
