/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include <agar/map/begin.h>

#define RG_PIXMAP_NAME_MAX 72

struct ag_window;
struct ag_toolbar;
struct rg_pixmap;
struct rg_tileview;

enum rg_pixmap_mod_type {
	RG_PIXMAP_PIXEL_REPLACE		/* Single pixel replace */
};

typedef struct rg_pixmap_mod {
	enum rg_pixmap_mod_type type;
	Uint16 x, y;			/* Coordinates of pixel in pixmap */
	Uint32 val;			/* Previous value */
} RG_PixmapMod;

typedef struct rg_pixmap_undoblk {
	struct rg_pixmap_mod *_Nonnull mods;	/* Undoable modifications */
	Uint		              nmods;
	Uint32 _pad;
} RG_PixmapUndoBlk;

enum rg_pixmap_blend_mode {
	RG_PIXMAP_OVERLAY_ALPHA,        /* dA = sA+dA */
	RG_PIXMAP_AVERAGE_ALPHA,        /* dA = (sA+dA)/2 */
	RG_PIXMAP_DEST_ALPHA,           /* dA = dA */
	RG_PIXMAP_NO_BLENDING           /* No blending done */
};

enum rg_brush_type {
	RG_PIXMAP_BRUSH_MONO,  /* Monochromatic (use current color) */
	RG_PIXMAP_BRUSH_RGB    /* Replace by brush color */
};

typedef struct rg_brush {
	char name[RG_PIXMAP_NAME_MAX];
	enum rg_brush_type type;
	int flags;
#define RG_PIXMAP_BRUSH_ONESHOT 0x01		/* Don't modify the same pixel
						   twice in the same pass */
	char px_name[RG_PIXMAP_NAME_MAX];	/* Pixmap reference */
	struct rg_pixmap *_Nullable px;		/* Resolved pixmap */
	AG_TAILQ_ENTRY(rg_brush) brushes;
} RG_Brush;

typedef struct rg_pixmap {
	char name[RG_PIXMAP_NAME_MAX];
	Uint flags;
	int xorig, yorig;		/* Pixmap origin point */
	Uint nRefs;			/* Number of tile references */
	struct rg_tileset *_Nonnull ts; /* Back pointer to tileset */
	AG_Surface *_Nullable su;	/* Pixmap surface */

	struct rg_pixmap_undoblk *_Nonnull ublks;	/* Undo buffer */
	Uint nublks, curblk;

	float h, s, v, a;			/* Current pixel value */
	RG_Brush *_Nullable curbrush;		/* Active brush */
	enum rg_pixmap_blend_mode blend_mode;	/* Current blending method */
	Uint32 _pad;
	AG_TAILQ_HEAD_(rg_brush) brushes;	/* Brush references */
	AG_TAILQ_ENTRY(rg_pixmap) pixmaps;
} RG_Pixmap;

__BEGIN_DECLS
RG_Pixmap *_Nonnull RG_PixmapNew(struct rg_tileset *_Nonnull,
                                 const char *_Nullable, int);

void RG_PixmapInit(RG_Pixmap *_Nonnull, struct rg_tileset *_Nonnull, int);
void RG_PixmapDestroy(RG_Pixmap *_Nonnull);
int  RG_PixmapLoad(RG_Pixmap *_Nonnull, AG_DataSource *_Nonnull);
void RG_PixmapSave(RG_Pixmap *_Nonnull, AG_DataSource *_Nonnull);
void RG_PixmapScale(RG_Pixmap *_Nonnull, int, int);

struct ag_window *_Nullable RG_PixmapEdit(struct rg_tileview *_Nonnull,
                                          RG_TileElement *_Nonnull);
struct ag_toolbar *_Nonnull RG_PixmapToolbar(struct rg_tileview *_Nonnull,
                                             RG_TileElement *_Nonnull);

void RG_PixmapButtondown(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                         int,int, int,int, int);
void RG_PixmapButtonup(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                       int,int, int,int, int);
void RG_PixmapMotion(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                     int,int, int,int, int);
int  RG_PixmapWheel(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull, int);
void RG_PixmapKeydown(struct rg_tileview *_Nonnull, int);
void RG_PixmapKeyup(struct rg_tileview *_Nonnull);

void RG_PixmapBeginUndoBlk(RG_Pixmap *_Nonnull);
void RG_PixmapUndo(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull);
void RG_PixmapRedo(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull);

int    RG_PixmapPutPixel(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                         int,int, Uint32, int);
void   RG_PixmapApplyBrush(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                           int,int, Uint32);
Uint32 RG_PixmapSourcePixel(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                            int,int);
void   RG_PixmapSourceRGBA(struct rg_tileview *_Nonnull, RG_TileElement *_Nonnull,
                           int,int,
                           Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull,
			   Uint8 *_Nonnull);

RG_Brush *_Nonnull RG_PixmapAddBrush(RG_Pixmap *_Nonnull, enum rg_brush_type,
                                     RG_Pixmap *_Nullable);
void               RG_PixmapDelBrush(RG_Pixmap *_Nonnull, RG_Brush *_Nonnull);

void RG_PixmapOpenMenu(struct rg_tileview *_Nonnull, int,int);
void RG_PixmapCloseMenu(struct rg_tileview *_Nonnull);

static __inline__ void
RG_PixmapSetBlendingMode(RG_Pixmap *_Nonnull pixmap,
    enum rg_pixmap_blend_mode bmode)
{
	pixmap->blend_mode = bmode;
}
static __inline__ void
RG_PixmapSetBrush(RG_Pixmap *_Nonnull pixmap, RG_Brush *_Nullable brush)
{
	pixmap->curbrush = brush;
}
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_PIXMAP_H_ */
