/*	Public domain	*/

#ifndef _AGAR_RG_ANIMATION_H_
#define _AGAR_RG_ANIMATION_H_

#include <agar/config/have_opengl.h>

#include <agar/rg/begin.h>

#define RG_ANIMATION_NAME_MAX	32
#define RG_ANIMATION_FRAMES_MAX	(0xffffffff-1)

struct ag_window;

enum rg_anim_insn_type {
	RG_ANIM_TILE,			/* Replace/blend with prev tile */
	RG_ANIM_DISPX,			/* Pixmap translation */
	RG_ANIM_ROTPX			/* Pixmap rotation */
};
#define RG_ANIM_LAST (RG_ANIM_ROTPX+1)

/* Source animation instruction */
typedef struct rg_anim_insn {
	enum rg_anim_insn_type type;
	struct rg_tile *t;		/* Tile reference (for ANIM_*TILE) */
	struct rg_pixmap *px;		/* Pixmap reference (for ANIM_*PX) */
#if 0
	struct rg_sketch *sk;		/* Sketch of path (for ANIM_MOVPX) */
#endif
	Uint delay;			/* Delay in milliseconds */
	union {
		struct {
			Uint alpha;		/* Per-surface source alpha */
		} tile; 
		struct {
		 	int dx, dy;		/* Displacement in pixels */
		} disPx;
		struct {
			Uint x, y;		/* Origin point */
			int theta;		/* Angle of rotation */
		} rotPx;
	} args;
#ifndef _AGAR_RG_PUBLIC_H_
#define in_tile args.tile
#define in_disPx args.disPx
#define in_rotPx args.rotPx
#endif
	AG_TAILQ_ENTRY(rg_anim_insn) insns;
} RG_AnimInsn;

/* Generated animation frame */
typedef struct rg_anim_frame {
	Uint name;
	Uint delay;
	AG_Surface *su;
	Uint texture;				/* For OpenGL */
} RG_AnimFrame;

/* Animation structure */
typedef struct rg_anim {
	char name[RG_ANIMATION_NAME_MAX];	/* User identifier */
	Uint32 main_id;				/* Default ID mapping */
	Uint flags;
#define RG_ANIM_SRCALPHA	0x01
#define RG_ANIM_SRCCOLORKEY	0x02
#define RG_ANIM_DUPED_FLAGS	(RG_ANIM_SRCALPHA|RG_ANIM_SRCCOLORKEY)
	Uint w, h;				/* Sprite geometry */
	struct rg_tileset *tileset;		/* Parent tileset */
	Uint nrefs;				/* Reference count */

	RG_AnimInsn  *insns;			/* Animation instructions */
	Uint	     ninsns;
	RG_AnimFrame *frames;			/* Generated frames */
	Uint	     nframes;
	Uint	     gframe;			/* Current frame (global) */

	AG_SLIST_HEAD_(rg_anim_variant) vars;	/* Transformed variants */
	AG_TAILQ_ENTRY(rg_anim) animations;
} RG_Anim;

/* Cached, transformed animation variant */
typedef struct rg_anim_variant {
	RG_TransformChain transforms;		/* Applied transforms */
	RG_Anim *anim;				/* Transformed anim */
	Uint32 last_drawn;			/* Time last draw occured */
	AG_SLIST_ENTRY(rg_anim_variant) vars;
} RG_AnimVariant;

#ifdef AG_DEBUG
#define RG_ANIM_FRAME(anim,frame) RG_AnimGetFrame((anim),(frame))
#else
#define RG_ANIM_FRAME(anim,frame) &(anim)->frames[frame]
#endif

__BEGIN_DECLS
void RG_AnimInit(RG_Anim *, struct rg_tileset *, const char *, Uint);
void RG_AnimDestroy(RG_Anim *);
int  RG_AnimLoad(RG_Anim *, AG_DataSource *);
void RG_AnimSave(RG_Anim *, AG_DataSource *);
struct ag_window *RG_AnimEdit(RG_Anim *);

void RG_AnimScale(RG_Anim *, Uint, Uint);
void RG_AnimGenerate(RG_Anim *);
Uint RG_AnimInsertInsn(RG_Anim *, enum rg_anim_insn_type);
void RG_AnimRemoveInsn(RG_Anim *, Uint);

Uint RG_AnimInsertFrame(RG_Anim *, AG_Surface *);
void RG_AnimRemoveFrame(RG_Anim *, Uint);

static __inline__ RG_AnimFrame *
RG_AnimGetFrame(RG_Anim *anim, Uint frame)
{
	if (frame >= anim->nframes) {
		AG_FatalError("%s: no such frame %u", anim->name, frame);
	}
	return (&anim->frames[frame]);
}
__END_DECLS

#include <agar/rg/close.h>
#endif	/* _AGAR_RG_ANIMATION_H_ */
