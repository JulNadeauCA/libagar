/*	$Csoft: animation.h,v 1.4 2005/03/11 08:59:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_ANIMATION_H_
#define _AGAR_RG_ANIMATION_H_
#include "begin_code.h"

#define ANIMATION_NAME_MAX 32

enum anim_insn_type {
	ANIM_TILE,			/* Replace/blend with prev tile */
	ANIM_DISPX,			/* Pixmap translation */
	ANIM_ROTPX			/* Pixmap rotation */
};
#define ANIM_LAST (ANIM_ROTPX+1)

struct anim_insn {
	enum anim_insn_type type;
	struct tile *t;			/* Tile reference (for ANIM_*TILE) */
	struct pixmap *px;		/* Pixmap reference (for ANIM_*PX) */
	struct sketch *sk;		/* Sketch of path (for ANIM_MOVPX) */
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
	TAILQ_ENTRY(anim_insn) insns;
};

struct anim_frame {
	u_int name;
	u_int delay;
	SDL_Surface *su;
#ifdef HAVE_OPENGL
	GLuint texture;
#endif
};

struct animation {
	char name[ANIMATION_NAME_MAX];
	int flags;
#define ANIMATION_SRCALPHA	0x01
#define ANIMATION_SRCCOLORKEY	0x02
	u_int w, h;
	struct tileset *tileset;
	u_int nrefs;

	struct anim_insn  *insns;		/* Animation instructions */
	u_int		  ninsns;
	struct anim_frame *frames;		/* Generated frames */
	u_int	   	  nframes;
	u_int	          gframe;		/* Current frame (global) */

	TAILQ_ENTRY(animation) animations;
};

__BEGIN_DECLS
void animation_init(struct animation *, struct tileset *, const char *, int);
void animation_destroy(struct animation *);
int  animation_load(struct animation *, struct netbuf *);
void animation_save(struct animation *, struct netbuf *);

void animation_scale(struct animation *, u_int, u_int);
void animation_generate(struct animation *);

#ifdef EDITION
struct window *animation_edit(struct animation *);
#endif

u_int	anim_insert_insn(struct animation *, enum anim_insn_type);
void	anim_remove_insn(struct animation *, u_int);
u_int	anim_insert_frame(struct animation *);
void	anim_remove_frame(struct animation *, u_int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_ANIMATION_H_ */
