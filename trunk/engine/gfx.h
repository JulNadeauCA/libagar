/*	$Csoft: gfx.h,v 1.3 2003/06/21 06:39:44 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct object;
struct noderef;

struct gfx_anim {
	SDL_Surface	 **frames;
	Uint32		  nframes;
	Uint32		maxframes;
	Uint32		   frame;
	int		   delta;
	int		   delay;
};

struct gfx_cached_sprite {
	SDL_Surface	*su;			/* Modified sprite */
	Uint32		 last_drawn;		/* Time last draw occured */
	SLIST_HEAD(,transform)	transforms;	/* Applied transforms */
	SLIST_ENTRY(gfx_cached_sprite) sprites;
};

struct gfx_cached_anim {
	struct gfx_anim	*anim;			/* Modified anim */
	Uint32		 last_drawn;		/* Time last draw occured */
	SLIST_HEAD(,transform)	transforms;	/* Applied transforms */
	SLIST_ENTRY(gfx_cached_anim) anims;
};

struct gfx_spritecl {
	SLIST_HEAD(,gfx_cached_sprite)	sprites;
};

struct gfx_animcl {
	SLIST_HEAD(,gfx_cached_anim)	anims;
};

struct gfx {
	char		  *name;		/* Shared identifier */
	struct object	  *pobj;		/* For submap refs */

	SDL_Surface		 **sprites;	/* Static images */
	struct gfx_spritecl	 *csprites;	/* Sprite transform cache */
	Uint32			  nsprites;
	Uint32			maxsprites;
	struct gfx_anim		 **anims;	/* Animations */
	struct gfx_animcl	 *canims;	/* Anim transform cache */
	Uint32			  nanims;
	Uint32			maxanims;

	struct map	  *tile_map;	/* User map of source nodes */
	struct map	 **submaps;	/* Sprite fragment maps */
	Uint32		  nsubmaps;
	Uint32		maxsubmaps;

	pthread_mutex_t	 used_lock;
	Uint32		 used;		/* Reference count */
#define ART_MAX_USED	 (0xffffffff-1)
	TAILQ_ENTRY(gfx) gfxs;		/* Art pool */
};

#ifdef DEBUG
#define SPRITE(ob, i)	gfx_get_sprite((struct object *)(ob), (i))
#define ANIM(ob, i)	gfx_get_anim((struct object *)(ob), (i))
#else
#define SPRITE(ob, i)	((struct object *)(ob))->gfx->sprites[(i)]
#define ANIM(ob, i)	((struct object *)(ob))->gfx->anims[(i)]
#endif

__BEGIN_DECLS
struct gfx	*gfx_fetch(void *, const char *);
void		 gfx_unused(struct gfx *);
void		 gfx_wire(struct gfx *);
void		 gfx_scan_alpha(SDL_Surface *);

Uint32		 gfx_insert_sprite(struct gfx *, SDL_Surface *, int);
struct map	*gfx_insert_fragments(struct gfx *, SDL_Surface *);
Uint32		 gfx_insert_submap(struct gfx *, struct map *);

struct gfx_anim	*gfx_insert_anim(struct gfx *, int);
Uint32		 gfx_insert_anim_frame(struct gfx_anim *, SDL_Surface *);
void		 gfx_anim_tick(struct gfx_anim *, struct noderef *);

#ifdef DEBUG
__inline__ SDL_Surface		*gfx_get_sprite(struct object *, Uint32);
__inline__ struct gfx_anim	*gfx_get_anim(struct object *, Uint32);
#endif
__END_DECLS

#include "close_code.h"
