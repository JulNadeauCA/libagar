/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_sdl.h>
#else
#include <agar/config/have_sdl.h>
#endif
#ifdef HAVE_SDL
#include "begin_code.h"

#define GISDL(p) ((GI_SDL *)(p))

typedef struct gi_sdl {
	struct gi _inherit;
	SDL_Surface *s;			/* Display surface */
	SDL_VideoInfo *vinfo;		/* Video information */
	int depth;			/* Allocated depth */
} GI_SDL;

__BEGIN_DECLS
extern const GI_Class giSDLClass;
__END_DECLS

#include "close_code.h"
#endif /* HAVE_SDL */
