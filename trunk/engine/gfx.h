/*	$Csoft: gfx.h,v 1.20 2004/05/10 05:17:34 vedge Exp $	*/
/*	Public domain	*/

#include <engine/transform.h>

#include "begin_code.h"

struct object;
struct noderef;

struct gfx_anim {
	SDL_Surface	 **frames;
	Uint32		  nframes;
	Uint32		maxframes;
	Uint32		    frame;		/* Shared frame# */
};

struct gfx_cached_sprite {
	SDL_Surface	 *su;			/* Modified sprite */
	Uint32		  last_drawn;		/* Time last draw occured */
	struct transformq transforms;		/* Applied transforms */
	SLIST_ENTRY(gfx_cached_sprite) sprites;
};

struct gfx_cached_anim {
	struct gfx_anim	  *anim;		/* Modified anim */
	Uint32		   last_drawn;		/* Time last draw occured */
	struct transformq  transforms;		/* Applied transforms */
	SLIST_ENTRY(gfx_cached_anim) anims;
};

struct gfx_spritecl {
	SLIST_HEAD(,gfx_cached_sprite)	sprites;
};

struct gfx_animcl {
	SLIST_HEAD(,gfx_cached_anim)	anims;
};

struct gfx {
	char *name;
	enum {
		GFX_SHARED,			/* Managed, shared graphics */
		GFX_PRIVATE			/* Object private graphics */
	} type;

	SDL_Surface		 **sprites;	/* Static images */
	struct gfx_spritecl	 *csprites;	/* Sprite transform cache */
	Uint32			  nsprites;
	Uint32			maxsprites;
	struct gfx_anim		 **anims;	/* Animations */
	struct gfx_animcl	 *canims;	/* Anim transform cache */
	Uint32			  nanims;
	Uint32			maxanims;

	struct map	 **submaps;		/* Generated maps */
	Uint32		  nsubmaps;
	Uint32		maxsubmaps;

	pthread_mutex_t	 used_lock;
	Uint32		 used;			/* Reference count */
#define GFX_MAX_USED	 (0xffffffff-1)
	TAILQ_ENTRY(gfx) gfxs;			/* Art pool */
};

#define SPRITE(ob, i)	((struct object *)(ob))->gfx->sprites[(i)]
#define ANIM(ob, i)	((struct object *)(ob))->gfx->anims[(i)]

#define GFX_ANIM_FRAME(r, an) \
    (an)->frames[((r)->r_anim.flags & NODEREF_PVT_FRAME) ? \
                 (r)->r_anim.frame : an->frame]

TAILQ_HEAD(gfxq, gfx);
extern struct gfxq gfxq;
extern pthread_mutex_t gfxq_lock;

__BEGIN_DECLS
struct gfx *gfx_alloc_pvt(void *, const char *);
struct gfx *gfx_fetch_shd(const char *);
void	    gfx_init(struct gfx *, int, const char *);
void	    gfx_destroy(struct gfx *);
void	    gfx_unused(struct gfx *);
void	    gfx_wire(struct gfx *);
void	    gfx_scan_alpha(SDL_Surface *);

Uint32		 gfx_insert_sprite(struct gfx *, SDL_Surface *);
struct map	*gfx_insert_fragments(struct gfx *, SDL_Surface *);
Uint32		 gfx_insert_submap(struct gfx *, struct map *);
struct gfx_anim	*gfx_insert_anim(struct gfx *);
Uint32		 gfx_insert_anim_frame(struct gfx_anim *, SDL_Surface *);

#ifdef DEBUG
struct window	*gfx_debug_window(void);
#endif
__END_DECLS

#include "close_code.h"
