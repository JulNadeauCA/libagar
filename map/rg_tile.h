/*	Public domain	*/

#ifndef _AGAR_RG_TILE_H_
#define _AGAR_RG_TILE_H_

#include <agar/config/have_opengl.h>

#include <agar/map/begin.h>

#define RG_TILE_NAME_MAX	 128
#define RG_TILE_CLASS_MAX	 32
#define RG_TILE_ELEMENT_NAME_MAX 32
#define RG_TILE_SIZE_MIN	 2
#define RG_TILE_SIZE_MAX	 1024

struct ag_window;

enum rg_tile_element_type {
	RG_TILE_FEATURE,
	RG_TILE_PIXMAP,
/*	RG_TILE_SKETCH */
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
			struct rg_feature *_Nonnull ft;
			int x, y;
		} feature;
		struct {
			struct rg_pixmap *_Nonnull px;
			int x, y;
			int alpha;
			Uint32 _pad;
		} pixmap;
		struct {
			struct rg_sketch *_Nonnull sk;
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
	AG_TAILQ_ENTRY(rg_tile_element) elements;
} RG_TileElement;

AG_TAILQ_HEAD(rg_tile_elementq, rg_tile_element);

typedef struct rg_tile {
	char name[RG_TILE_NAME_MAX];	/* User description */
	char clname[RG_TILE_CLASS_MAX];	/* Category (application-specific) */
	Uint32 main_id;			/* Default ID mapping */
	Uint32 _pad1;
	struct rg_tileset *_Nonnull ts;	/* Back pointer to tileset */
	AG_Surface *_Nullable su;	/* Generated surface */

	Uint texture;			/* Cached texture (GL) */
	float texcoords[4];		/* Texture coords (GL) */
	
	int xOrig, yOrig;		/* Origin point */
	Uint snap_mode;			/* Snapping mode (edition) */
	Uint *_Nullable attrs;		/* Node attribute grid (edition) */
	int  *_Nullable layers;		/* Node layer offset grid (edition) */
	Uint nw, nh;			/* Node grid dimensions */
	Uint flags;
#define RG_TILE_SRCCOLORKEY	0x01	/* Colorkey source */
#define RG_TILE_SRCALPHA	0x02	/* Alpha source */
#define RG_TILE_DIRTY		0x04	/* Mark for redraw */
	Uint nRefs;			/* Reference count */
	AG_Color c;			/* Current RGB color (edition) */
	Uint32 pc;			/* Current pixel value (edition) */
	struct {
		int w;			/* Line width */
		Uint32 stipple;		/* Stipple bitmap pattern */
		enum rg_line_endpoint_style {
			RG_TILE_SQUARE_ENDPOINT,
			RG_TILE_ROUNDED_ENDPOINT
		} endpoint;
	} line;
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad2;
#endif
	struct rg_tile_elementq elements;	/* Elements to combine */

	/* Pixel blending function */
	void (*_Nonnull blend_fn)(struct rg_tile *_Nonnull,
	                          AG_Surface *_Nonnull, AG_Rect *_Nonnull);

	AG_SLIST_HEAD_(rg_tile_variant) vars;	/* Cached variants */
	AG_TAILQ_ENTRY(rg_tile) tiles;
} RG_Tile;

/* Cached, transformed tile variant */
typedef struct rg_tile_variant {
	RG_TransformChain transforms;	/* Applied transforms */
	AG_Surface *_Nonnull su;	/* Cached resulting surface */

	/* For OpenGL */
	Uint texture;			/* Cached texture */
	float texcoords[4];

	Uint32 last_drawn;		/* Time last draw occurred */
	AG_SLIST_ENTRY(rg_tile_variant) vars;
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
extern const char *_Nullable rgTileSnapModes[];

RG_Tile *_Nonnull RG_TileNew(struct rg_tileset *_Nonnull, const char *_Nullable,
                             Uint16,Uint16, Uint);

void RG_TileInit(RG_Tile *_Nonnull, struct rg_tileset *_Nonnull,
                 const char *_Nonnull);
void RG_TileScale(struct rg_tileset *_Nonnull, RG_Tile *_Nonnull, Uint16,Uint16);
void RG_TileGenerate(RG_Tile *_Nonnull);

struct ag_window *_Nullable RG_TileEdit(struct rg_tileset *_Nonnull,
                                        RG_Tile *_Nonnull);

void RG_TileDestroy(RG_Tile *_Nonnull);
void RG_TileSave(RG_Tile *_Nonnull, AG_DataSource *_Nonnull);
int  RG_TileLoad(RG_Tile *_Nonnull, AG_DataSource *_Nonnull);
void RG_TileOpenMenu(struct rg_tileview *_Nonnull, int,int);
void RG_TileCloseMenu(struct rg_tileview *_Nonnull);

RG_TileElement *_Nullable RG_TileFindElement(RG_Tile *_Nonnull,
                                             enum rg_tile_element_type,
                                             const char *_Nonnull);

RG_TileElement *_Nonnull RG_TileAddPixmap(RG_Tile *_Nonnull, const char *_Nullable,
                                          struct rg_pixmap *_Nonnull, int,int);
void                     RG_TileDelPixmap(RG_Tile *_Nonnull,
                                          struct rg_pixmap *_Nonnull, int);

RG_TileElement *_Nonnull RG_TileAddFeature(RG_Tile *_Nonnull, const char *_Nullable,
                                           void *_Nonnull, int,int);
void                     RG_TileDelFeature(RG_Tile *_Nonnull, void *_Nonnull, int);

#if 0
RG_TileElement *_Nonnull RG_TileAddSketch(RG_Tile *_Nonnull, const char *_Nullable,
                                          struct rg_sketch *_Nonnull, int,int);
void                     RG_TileDelSketch(RG_Tile *_Nonnull,
                                          struct rg_sketch *_Nonnull, int);
#endif
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_TILE_H_ */
