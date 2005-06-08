/*	$Csoft: animation.h,v 1.1 2005/03/24 04:02:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_ANIMATION_H_
#define _AGAR_RG_ANIMATION_H_
#include "begin_code.h"

#define RG_ANIMATION_NAME_MAX 32

enum rg_anim_insn_type {
	RG_ANIM_TILE,			/* Replace/blend with prev tile */
	RG_ANIM_DISPX,			/* Pixmap translation */
	RG_ANIM_ROTPX			/* Pixmap rotation */
};
#define RG_ANIM_LAST (RG_ANIM_ROTPX+1)

typedef struct rg_anim_insn {
	enum rg_anim_insn_type type;
	struct rg_tile *t;		/* Tile reference (for ANIM_*TILE) */
	struct rg_pixmap *px;		/* Pixmap reference (for ANIM_*PX) */
	struct rg_sketch *sk;		/* Sketch of path (for ANIM_MOVPX) */
	u_int delay;			/* Delay in milliseconds */
	union {
		struct {
			u_int alpha;		/* Per-surface source alpha */
		} tile; 
		struct {
		 	int dx, dy;		/* Displacement in pixels */
		} disPx;
		struct {
			u_int x, y;		/* Origin point */
			int theta;		/* Angle of rotation */
		} rotPx;
	} args;
#define in_tile args.tile
#define in_disPx args.disPx
#define in_rotPx args.rotPx
	TAILQ_ENTRY(rg_anim_insn) insns;
} RG_AnimInsn;

typedef struct rg_anim_frame {
	u_int name;
	u_int delay;
	SDL_Surface *su;
#ifdef HAVE_OPENGL
	GLuint texture;
#endif
} RG_AnimFrame;

typedef struct rg_anim {
	char name[RG_ANIMATION_NAME_MAX];
	int flags;
#define ANIMATION_SRCALPHA	0x01
#define ANIMATION_SRCCOLORKEY	0x02
	u_int w, h;
	struct rg_tileset *tileset;
	u_int nrefs;

	RG_AnimInsn  *insns;		/* Animation instructions */
	u_int	     ninsns;
	RG_AnimFrame *frames;		/* Generated frames */
	u_int	     nframes;
	u_int	     gframe;		/* Current frame (global) */

	TAILQ_ENTRY(rg_anim) animations;
} RG_Anim;

__BEGIN_DECLS
void RG_AnimInit(RG_Anim *, struct rg_tileset *, const char *, int);
void RG_AnimDestroy(RG_Anim *);
int  RG_AnimLoad(RG_Anim *, AG_Netbuf *);
void RG_AnimSave(RG_Anim *, AG_Netbuf *);

void RG_AnimScale(RG_Anim *, u_int, u_int);
void RG_AnimGenerate(RG_Anim *);

#ifdef EDITION
AG_Window *RG_AnimEdit(RG_Anim *);
#endif

u_int	RG_AnimInsertInsn(RG_Anim *, enum rg_anim_insn_type);
void	RG_AnimRemoveInsn(RG_Anim *, u_int);
u_int	RG_AnimInsertFrame(RG_Anim *);
void	RG_AnimRemoveFrame(RG_Anim *, u_int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_ANIMATION_H_ */
