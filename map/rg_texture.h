/*	Public domain	*/

#ifndef _AGAR_RG_TEXTURE_H_
#define _AGAR_RG_TEXTURE_H_
#include <agar/map/begin.h>

#define RG_TEXTURE_NAME_MAX 32

struct ag_window;

enum texture_wrap_mode {
	RG_TEXTURE_REPEAT,
	RG_TEXTURE_CLAMP,
	RG_TEXTURE_CLAMP_TO_EDGE,
	RG_TEXTURE_CLAMP_TO_BORDER
};

typedef struct rg_texture {
	char name[RG_TEXTURE_NAME_MAX];
	char tileset[AG_OBJECT_PATH_MAX];	/* Parent tileset */
	char tile[RG_PIXMAP_NAME_MAX];		/* Source tile */
	RG_Tile *_Nullable t;			/* Set by RG_TextureFind() */
	int flags;
	Uint wrap_s;				/* Wrap mode for s coordinate */
	Uint wrap_t;				/* Wrap mode for t coordinate */
	int blend_func;				/* Blending function to use
						   (ag_blend_func) */
	Uint8 alpha;				/* Overall alpha value */
	Uint8 _pad[7];
	AG_TAILQ_ENTRY(rg_texture) textures;
} RG_Texture;

__BEGIN_DECLS
void RG_TextureInit(RG_Texture *_Nonnull, const char *_Nonnull);
void RG_TextureDestroy(RG_Texture *_Nonnull);
int  RG_TextureLoad(RG_Texture *_Nonnull, AG_DataSource *_Nonnull);
void RG_TextureSave(RG_Texture *_Nonnull, AG_DataSource *_Nonnull);

RG_Texture *_Nullable RG_TextureFind(struct rg_tileset *_Nonnull,
                                     const char *_Nonnull);

struct ag_window *_Nonnull RG_TextureEdit(void *_Nonnull, RG_Texture *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_TEXTURE_H_ */
