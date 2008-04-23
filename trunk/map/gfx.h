/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <map/gfx_transform.h>
#include <config/have_opengl.h>
#else
#include <agar/map/gfx_transform.h>
#include <agar/config/have_opengl.h>
#endif

#include "begin_code.h"

struct ag_object;
struct ag_gfx;

typedef struct ag_anim {
	AG_Surface **frames;
#ifdef HAVE_OPENGL
	Uint *textures;
	float texcoords[4];
#endif
	Uint32 nframes;
	Uint32 maxframes;
	Uint32 frame;			/* Current frame# */
} AG_Anim;

typedef struct ag_cached_sprite {
	AG_Surface *su;
#ifdef HAVE_OPENGL
	Uint texture;
	float texcoords[4];
#endif
	Uint32 last_drawn;			/* Time last draw occured */
	struct ag_transformq transforms;	/* Applied transforms */
	AG_SLIST_ENTRY(ag_cached_sprite) sprites;
} AG_CachedSprite;

typedef struct ag_cached_anim {
	struct ag_anim *anim;			/* Modified anim */
	Uint32 last_drawn;			/* Time last draw occured */
	struct ag_transformq transforms;	/* Applied transforms */
	AG_SLIST_ENTRY(ag_cached_anim) anims;
} AG_CachedAnim;

typedef struct ag_anim_cache {
	AG_SLIST_HEAD(,ag_cached_anim) anims;
} AG_AnimCache;

enum ag_gfx_snap_mode {
	AG_GFX_SNAP_NOT,
	AG_GFX_SNAP_TO_GRID
};

#define AG_SPRITE_NAME_MAX 24
#define AG_SPRITE_CLASS_MAX 16

typedef struct ag_sprite {
	char name[AG_SPRITE_NAME_MAX];
	char clname[AG_SPRITE_CLASS_MAX];
	struct ag_gfx *pgfx;
	Uint32 index;
	AG_Surface *su;
	int xOrig, yOrig;			/* Origin point */
	enum ag_gfx_snap_mode snap_mode; 	/* Default snapping mode */
	Uint *attrs;				/* Default node attributes */
	int *layers;				/* Node layer offsets */
#ifdef HAVE_OPENGL
	Uint texture;
	float texcoords[4];
#endif
	AG_SLIST_HEAD(,ag_cached_sprite) csprites; /* Transform cache */
} AG_Sprite;

typedef struct ag_gfx {
	void *pobj;

	AG_Sprite *sprites;		/* Images */
	Uint32    nsprites;

	AG_Anim	      *anims;		/* Animations */
	AG_AnimCache *canims;		/* Anim transform cache. XXX move */
	Uint32	      nanims;

	Uint32 used;			/* Reference count */
#define AG_GFX_MAX_USED	 (0xffffffff-1)	/* Maximum number of references */
} AG_Gfx;

#define AG_SPRITE(ob, i)	AGOBJECT(ob)->gfx->sprites[(i)]
#define AG_ANIM(ob, i)		AGOBJECT(ob)->gfx->anims[(i)]

#define AG_BAD_SPRITE(ob, i)	(AGOBJECT(ob)->gfx == NULL || \
				 (i) >= AGOBJECT(ob)->gfx->nsprites || \
				 AG_SPRITE((ob),(i)).su == NULL)

#define AG_BAD_ANIM(ob, i)	(AGOBJECT(ob)->gfx == NULL || \
				 (i) >= AGOBJECT(ob)->gfx->nanims || \
				 AG_ANIM((ob),(i)).nframes == 0)
#define AG_ANIM_FRAME(r, an)	(an)->frames[(an)->frame]
#define AG_ANIM_TEXTURE(r, an) (an)->textures[(an)->frame]

#define AG_SPRITE_ATTR2(s,x,y) (s)->attrs[(y)*AG_SpriteGetWtiles(s) + (x)]
#define AG_SPRITE_LAYER2(s,x,y) (s)->layers[(y)*AG_SpriteGetWtiles(s) + (x)]
#define AG_SPRITE_ATTRS(s) (AG_SPRITE((s)->ts,(s)->s).attrs)
#define AG_SPRITE_LAYERS(s) (AG_SPRITE((s)->ts,(s)->s).layers)

extern const char *mapSnapModeNames[];

__BEGIN_DECLS
AG_Gfx	*AG_GfxNew(void *);
void	 AG_GfxInit(AG_Gfx *);
void	 AG_GfxDestroy(AG_Gfx *);
int	 AG_HasTransparency(AG_Surface *);
int	 AG_GfxLoad(struct ag_object *);
int	 AG_GfxSave(struct ag_object *, AG_DataSource *);
void	 AG_GfxUsed(void *);
int	 AG_GfxUnused(void *);
int	 AG_GfxLoadFromDEN(void *, const char *);

void		 AG_GfxAllocSprites(AG_Gfx *, Uint32);
void		 AG_GfxAllocAnims(AG_Gfx *, Uint32);
Uint32		 AG_GfxAddSprite(AG_Gfx *, AG_Surface *);
Uint32		 AG_GfxAddAnim(AG_Gfx *);
Uint32		 AG_GfxAddAnimFrame(AG_Anim *, AG_Surface *);

void	 AG_SpriteInit(AG_Gfx *, Uint32);
int	 AG_SpriteFind(AG_Gfx *, const char *, Uint32 *);
void	 AG_SpriteDestroy(AG_Gfx *, Uint32);
void	 AG_AnimInit(AG_Gfx *, Uint32);
void	 AG_AnimDestroy(AG_Gfx *, Uint32);
void	 AG_SpriteSetName(AG_Gfx *, Uint32, const char *);
void	 AG_SpriteSetClass(AG_Gfx *, Uint32, const char *);
void	 AG_SpriteSetSurface(AG_Gfx *, Uint32, AG_Surface *);
void	 AG_SpriteSetOrigin(AG_Sprite *, int, int);
void	 AG_SpriteSetSnapMode(AG_Sprite *, enum ag_gfx_snap_mode);
void	 AG_SpriteUpdate(AG_Sprite *);
void	 AG_SpriteGetNodeAttrs(AG_Sprite *, Uint *, Uint *);
Uint	 AG_SpriteGetWtiles(AG_Sprite *);
__END_DECLS

#include "close_code.h"
