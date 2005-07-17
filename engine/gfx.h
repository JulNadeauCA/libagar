/*	$Csoft: gfx.h,v 1.32 2005/07/16 16:07:27 vedge Exp $	*/
/*	Public domain	*/

#include <engine/map/transform.h>

#include "begin_code.h"

struct object;
struct noderef;

struct gfx_anim {
	SDL_Surface **frames;
#ifdef HAVE_OPENGL
	GLuint *textures;
	GLfloat texcoords[4];
#endif
	Uint32 nframes;
	Uint32 maxframes;
	Uint32 frame;			/* Current frame# */
};

struct gfx_cached_sprite {
	SDL_Surface *su;
#ifdef HAVE_OPENGL
	GLuint texture;
	GLfloat texcoords[4];
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

struct gfx_animcl {
	SLIST_HEAD(,gfx_cached_anim)	anims;
};

enum gfx_snap_mode {
	GFX_SNAP_NOT,
	GFX_SNAP_TO_GRID
};

struct sprite {
	SDL_Surface *su;			/* Sprite surface */
	int xOrig, yOrig;			/* Origin point */
	enum gfx_snap_mode snap_mode;		/* Preferred snapping mode */
#ifdef HAVE_OPENGL
	GLuint texture;				/* Cached OpenGL texture */
	GLfloat texcoords[4];			/* Texture coordinates. */
#endif
	SLIST_HEAD(,gfx_cached_sprite) csprites; /* Transform cache */
};

struct gfx {
	struct sprite *sprites;		/* Images */
	Uint32	      nsprites;

	struct gfx_anim	   *anims;	/* Animations */
	struct gfx_animcl *canims;	/* Anim transform cache */
	Uint32		   nanims;

	struct map **submaps;		/* Maps */
	Uint32	    nsubmaps;
	Uint32	  maxsubmaps;

	Uint32 used;			/* Reference count */
#define GFX_MAX_USED	 (0xffffffff-1)	/* Maximum number of references */
};

#define SPRITE(ob, i)		((struct object *)(ob))->gfx->sprites[(i)]
#define ANIM(ob, i)		((struct object *)(ob))->gfx->anims[(i)]

#define BAD_SPRITE(ob, i)	(OBJECT(ob)->gfx == NULL || \
				 (i) >= OBJECT(ob)->gfx->nsprites || \
				 SPRITE((ob),(i)).su == NULL)

#define BAD_ANIM(ob, i)		(OBJECT(ob)->gfx == NULL || \
				 (i) >= (ob)->gfx->nanims || \
				 ANIM((ob),(i)).nframes == 0)
#define GFX_ANIM_FRAME(r, an)	(an)->frames[(an)->frame]
#define GFX_ANIM_TEXTURE(r, an) (an)->textures[(an)->frame]

extern const char *gfx_snap_names[];

__BEGIN_DECLS
struct gfx	*gfx_new(void *);
void		 gfx_init(struct gfx *);
void		 gfx_destroy(struct gfx *);
int		 gfx_transparent(SDL_Surface *);
int		 gfx_wire(void *, const char *);
int		 gfx_load(struct object *);
int		 gfx_save(struct object *, struct netbuf *);

void		 gfx_alloc_sprites(struct gfx *, Uint32);
void		 gfx_alloc_anims(struct gfx *, Uint32);
Uint32		 gfx_insert_sprite(struct gfx *, SDL_Surface *);
struct map	*gfx_insert_fragments(struct gfx *, SDL_Surface *);
Uint32		 gfx_insert_submap(struct gfx *, struct map *);
Uint32		 gfx_insert_anim(struct gfx *);
Uint32		 gfx_insert_anim_frame(struct gfx_anim *, SDL_Surface *);

void		 sprite_init(struct gfx *, Uint32);
void		 sprite_destroy(struct gfx *, Uint32);
void		 anim_init(struct gfx *, Uint32);
void		 anim_destroy(struct gfx *, Uint32);
__inline__ void	 sprite_set_surface(struct gfx *, Uint32, SDL_Surface *);
__inline__ void	 sprite_set_origin(struct sprite *, int, int);
__inline__ void	 sprite_set_snap_mode(struct sprite *, enum gfx_snap_mode);
__inline__ void	 sprite_update(struct sprite *);
__END_DECLS

#include "close_code.h"
