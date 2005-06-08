/*	$Csoft: sketch.h,v 1.7 2005/06/07 06:49:25 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_SKETCH_H_
#define _AGAR_RG_SKETCH_H_
#include "begin_code.h"

#define RG_SKETCH_NAME_MAX	32

enum rg_sketch_mod_type {
	RG_SKETCH_VERTEX_DISPLACEMENT
};
struct rg_sketch_mod {
	enum rg_sketch_mod_type type;
	VG_Vtx *vtx;			/* Modified vertex */
	VG_Vtx pvtx;			/* Previous value */
};
struct rg_sketch_undoblk {
	struct rg_sketch_mod *mods;	/* Undoable modifications */
	u_int		     nmods;
};

typedef struct rg_sketch {
	char name[RG_SKETCH_NAME_MAX];
	int flags;
	struct rg_tileset *ts;
	VG *vg;
	u_int nrefs;
	float h, s, v, a;
	struct rg_sketch_undoblk *ublks;
	u_int nublks, curblk;
	TAILQ_ENTRY(rg_sketch) sketches;
} RG_Sketch;

__BEGIN_DECLS
void RG_SketchInit(RG_Sketch *, struct rg_tileset *, int);
void RG_SketchDestroy(RG_Sketch *);
int  RG_SketchLoad(RG_Sketch *, AG_Netbuf *);
void RG_SketchSave(RG_Sketch *, AG_Netbuf *);
void RG_SketchScale(RG_Sketch *, int, int, float, int, int);
void RG_SketchRender(RG_Tile *, RG_TileElement *);

AG_Window *RG_SketchEdit(struct rg_tileview *, RG_TileElement *);
AG_Window *RG_SketchEditElement(struct rg_tileview *, RG_TileElement *,
		                   VG_Element *);

AG_Window *RG_SketchSelect(struct rg_tileview *, RG_TileElement *,
 		             VG_Element *);
void RG_SketchUnselect(struct rg_tileview *, RG_TileElement *,
		     VG_Element *);

void RG_SketchKeyDown(struct rg_tileview *, RG_TileElement *, int, int);
void RG_SketchKeyUp(struct rg_tileview *, RG_TileElement *, int, int);
int  RG_SketchMouseWheel(struct rg_tileview *, RG_TileElement *, int);
void RG_SketchMouseButtonDown(struct rg_tileview *, RG_TileElement *, double,
	                    double, int);
void RG_SketchMouseButtonUp(struct rg_tileview *, RG_TileElement *, double,
		          double, int);
void RG_SketchMouseMotion(struct rg_tileview *, RG_TileElement *, double,
	                double, double, double, int);

void RG_SketchBeginUndoBlk(RG_Sketch *);
void RG_SketchUndo(struct rg_tileview *, RG_TileElement *);
void RG_SketchRedo(struct rg_tileview *, RG_TileElement *);

void RG_SketchOpenMenu(struct rg_tileview *, int, int);
void RG_SketchCloseMenu(struct rg_tileview *);
AG_Toolbar *RG_SketchToolbar(struct rg_tileview *, RG_TileElement *);

void RG_SketchDrawPolygon(RG_Tile *, VG *, VG_Element *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_SKETCH_H_ */
