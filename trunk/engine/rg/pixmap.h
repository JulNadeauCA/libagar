/*	$Csoft: pixmap.h,v 1.16 2005/04/10 09:09:02 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_PIXMAP_H_
#define _AGAR_RG_PIXMAP_H_
#include "begin_code.h"

#define RG_PIXMAP_NAME_MAX	32

struct rg_pixmap;
struct rg_tileview;

enum rg_pixmap_mod_type {
	RG_PIXMAP_PIXEL_REPLACE		/* Single pixel replace */
};
struct rg_pixmap_mod {
	enum rg_pixmap_mod_type type;
	Uint16 x, y;			/* Coordinates of pixel in pixmap */
	Uint32 val;			/* Previous value */
};
struct rg_pixmap_undoblk {
	struct rg_pixmap_mod *mods;	/* Undoable modifications */
	u_int		     nmods;
};

enum rg_pixmap_blend_mode {
	RG_PIXMAP_OVERLAY_ALPHA,
	RG_PIXMAP_AVERAGE_ALPHA,
	RG_PIXMAP_DEST_ALPHA,
	RG_PIXMAP_NO_BLENDING
};

enum pixmap_brush_type {
	RG_PIXMAP_BRUSH_MONO,		/* Monochromatic (use current color) */
	RG_PIXMAP_BRUSH_RGB		/* Replace by brush color */
};

struct rg_pixmap_brush {
	char name[RG_PIXMAP_NAME_MAX];
	enum pixmap_brush_type type;
	int flags;
#define RG_PIXMAP_BRUSH_ONESHOT 0x01	/* Don't mod the same pixel twice
					   in the same pass */
	char px_name[RG_PIXMAP_NAME_MAX];	/* Pixmap reference */
	struct rg_pixmap *px;				/* Resolved pixmap */
	TAILQ_ENTRY(rg_pixmap_brush) brushes;
};

typedef struct rg_pixmap {
	char name[RG_PIXMAP_NAME_MAX];
	int flags;
	int xorig, yorig;		/* Pixmap origin point */
	struct rg_tileset *ts;		/* Back pointer to tileset */
	SDL_Surface *su;		/* Pixmap surface */
	u_int nrefs;			/* Number of tile references */
	struct rg_pixmap_undoblk *ublks; /* Undo blocks */
	u_int nublks, curblk;

	float h, s, v, a;			/* Current pixel value */
	struct rg_pixmap_brush *curbrush;	/* Current brush */
	enum rg_pixmap_blend_mode blend_mode;	/* Current blending method */
	TAILQ_HEAD(, rg_pixmap_brush) brushes;	/* Brush references */
	TAILQ_ENTRY(rg_pixmap) pixmaps;
} RG_Pixmap;

__BEGIN_DECLS
void		 RG_PixmapInit(RG_Pixmap *, struct rg_tileset *, int);
void		 RG_PixmapDestroy(RG_Pixmap *);
int		 RG_PixmapLoad(RG_Pixmap *, AG_Netbuf *);
void		 RG_PixmapSave(RG_Pixmap *, AG_Netbuf *);
AG_Window	*RG_PixmapEdit(struct rg_tileview *, RG_TileElement *);
AG_Toolbar	*RG_PixmapToolbar(struct rg_tileview *, RG_TileElement *);
void		 RG_PixmapScale(RG_Pixmap *, int, int, int, int);

void RG_PixmapMousebuttonDown(struct rg_tileview *, RG_TileElement *,
			      int, int, int);
void RG_PixmapMousebuttonUp(struct rg_tileview *, RG_TileElement *,
			    int, int, int);
void RG_PixmapMouseMotion(struct rg_tileview *, RG_TileElement *,
		          int, int, int, int, int);
int RG_PixmapMouseWheel(struct rg_tileview *, RG_TileElement *, int);
void RG_PixmapKeyDown(struct rg_tileview *, RG_TileElement *, int, int);
void RG_PixmapKeyUp(struct rg_tileview *, RG_TileElement *, int, int);

void RG_PixmapBeginUndoBlk(RG_Pixmap *);
void RG_PixmapUndo(struct rg_tileview *, RG_TileElement *);
void RG_PixmapRedo(struct rg_tileview *, RG_TileElement *);

int RG_PixmapPutPixel(struct rg_tileview *, RG_TileElement *, int, int,
                      Uint32, int);
void RG_PixmapApplyBrush(struct rg_tileview *, RG_TileElement *, int, int,
			 Uint32);
Uint32 RG_PixmapSourcePixel(struct rg_tileview *, RG_TileElement *, int, int);
void RG_PixmapSourceRGBA(struct rg_tileview *, RG_TileElement *, int, int,
			 Uint8 *, Uint8 *, Uint8 *, Uint8 *);

struct rg_pixmap_brush *RG_PixmapAddBrush(RG_Pixmap *, enum pixmap_brush_type,
					  RG_Pixmap *);
void		        RG_PixmapDelBrush(RG_Pixmap *,
		                          struct rg_pixmap_brush *);

void RG_PixmapOpenMenu(struct rg_tileview *, int, int);
void RG_PixmapCloseMenu(struct rg_tileview *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_PIXMAP_H_ */
