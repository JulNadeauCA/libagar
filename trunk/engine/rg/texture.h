/*	$Csoft: texture.h,v 1.3 2005/06/07 03:05:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TEXTURE_H_
#define _AGAR_RG_TEXTURE_H_
#include "begin_code.h"

#define RG_TEXTURE_NAME_MAX 32

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
	RG_Tile *t;				/* Set by RG_TextureFind() */
	int flags;
	enum texture_wrap_mode wrap_s;		/* Wrap mode for s coordinate */
	enum texture_wrap_mode wrap_t;		/* Wrap mode for t coordinate */
	enum ag_blend_func blend_func;	/* Blending function to use */
	Uint8 alpha;				/* Overall alpha value */
	TAILQ_ENTRY(rg_texture) textures;
} RG_Texture;

__BEGIN_DECLS
void RG_TextureInit(RG_Texture *, struct rg_tileset *, const char *);
void RG_TextureDestroy(RG_Texture *);
int  RG_TextureLoad(RG_Texture *, AG_Netbuf *);
void RG_TextureSave(RG_Texture *, AG_Netbuf *);
__inline__ RG_Texture *RG_TextureFind(struct rg_tileset *, const char *);

#ifdef EDITION
AG_Window *RG_TextureEdit(RG_Texture *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TEXTURE_H_ */
