/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_opengl.h>
#include <config/have_sdl.h>
#else
#include <agar/config/have_opengl.h>
#include <agar/config/have_sdl.h>
#endif

#if defined(HAVE_OPENGL) && defined(HAVE_SDL)
#include "begin_code.h"

typedef struct gi_gl {
	struct gi _inherit;
	Uint32 flags;
#define GI_GL_DOUBLEBUFFER	0x01		/* Enable double buffering */
} GI_GL;

#define GIGL(p) ((GI_GL *)(p))

__BEGIN_DECLS
extern const GI_Class giGLClass;
__END_DECLS

#include "close_code.h"
#endif /* HAVE_OPENGL && HAVE_SDL */
