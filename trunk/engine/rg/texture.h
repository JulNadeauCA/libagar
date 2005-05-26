/*	$Csoft: texture.h,v 1.1 2005/03/24 04:02:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_TEXTURE_H_
#define _AGAR_RG_TEXTURE_H_
#include "begin_code.h"

#define TEXTURE_NAME_MAX 32

enum texture_wrap_mode {
	TEXTURE_REPEAT,
	TEXTURE_CLAMP,
	TEXTURE_CLAMP_TO_EDGE,
	TEXTURE_CLAMP_TO_BORDER
};

struct texture {
	char name[TEXTURE_NAME_MAX];
	char tileset[OBJECT_PATH_MAX];		/* Parent pixmap tileset name */
	char pixmap[PIXMAP_NAME_MAX];		/* Pixmap name */
	int flags;
	enum texture_wrap_mode wrap_s;		/* Wrap mode for s coordinate */
	enum texture_wrap_mode wrap_t;		/* Wrap mode for t coordinate */
	enum view_blend_func blend_func;	/* Blending function to use */
	Uint8 alpha;				/* Overall alpha value */
	TAILQ_ENTRY(texture) textures;
};

__BEGIN_DECLS
void texture_init(struct texture *, struct tileset *, const char *);
void texture_destroy(struct texture *);
int  texture_load(struct texture *, struct netbuf *);
void texture_save(struct texture *, struct netbuf *);

#ifdef EDITION
struct window *texture_edit(struct texture *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TEXTURE_H_ */
