/*	$Csoft: gfx.h,v 1.22 2004/12/17 03:17:51 vedge Exp $	*/
/*	Public domain	*/

#include <engine/transform.h>

#include "begin_code.h"

struct object;
struct noderef;

struct gfx_anim {
	SDL_Surface **frames;
#ifdef HAVE_OPENGL
	GLuint *textures;
	GLfloat *texcoords;
#endif
	Uint32 nframes;
	Uint32 maxframes;
	Uint32 frame;		/* Shared frame# */
};

struct gfx_cached_sprite {
	SDL_Surface *su;
#ifdef HAVE_OPENGL
	GLuint texture;
	GLfloat texcoord;
#endif
	Uint32 last_drawn;			/* Time last draw occured */
	struct transformq transforms;		/* Applied transforms */
	SLIST_ENTRY(gfx_cached_sprite) sprites;
};

struct gfx_cached_anim {
	struct gfx_anim	*anim;			/* Modified anim */
	Uint32 last_drawn;			/* Time last draw occured */
	struct transformq transforms;		/* Applied transforms */
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
		GFX_PRIVATE			/* Object-managed graphics */
	} type;

	SDL_Surface	**sprites;
#ifdef HAVE_OPENGL
	GLuint		  *textures;
	GLfloat		  *texcoords;
#endif
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
#define GFX_MAX_USED	 (0xffffffff-1)		/* Maximum #references */

	TAILQ_ENTRY(gfx) gfxs;			/* Art pool */
};

#define SPRITE(ob, i)		((struct object *)(ob))->gfx->sprites[(i)]
#define SPRITE_TEX(ob, i)	((struct object *)(ob))->gfx->textures[(i)]
#define ANIM(ob, i)		((struct object *)(ob))->gfx->anims[(i)]

#define GFX_ANIM_FRAME(r, an) \
    (an)->frames[((r)->r_anim.flags & NODEREF_PVT_FRAME) ? \
                 (r)->r_anim.frame : an->frame]

TAILQ_HEAD(gfxq, gfx);
extern struct gfxq gfxq;
extern pthread_mutex_t gfxq_lock;

__BEGIN_DECLS
struct gfx	*gfx_alloc_pvt(void *, const char *);
struct gfx	*gfx_fetch_shd(const char *);
void		 gfx_init(struct gfx *, int, const char *);
void		 gfx_destroy(struct gfx *);
void		 gfx_unused(struct gfx *);
void		 gfx_wire(struct gfx *);
int		 gfx_transparent(SDL_Surface *);

void		 gfx_alloc_sprites(struct gfx *, Uint32);
void		 gfx_replace_sprite(struct gfx *, Uint32, SDL_Surface *);
void		 gfx_update_sprite(struct gfx *, Uint32);

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
