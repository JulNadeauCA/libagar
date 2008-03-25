/*	Public domain	*/

#ifndef _AGAR_GI_GI_H_
#define _AGAR_GI_GI_H_

#ifdef _AGAR_INTERNAL
#include <config/have_sdl.h>
#else
#include <agar/config/have_sdl.h>
#endif

#include "begin_code.h"

/* RGBA color value */
typedef struct gi_color {
	Uint8 r, g, b, a;
} GI_Color;

/* Color palette */
typedef struct gi_palette {
	Uint     ncolors;
	GI_Color *colors;
} GI_Palette;

#include "close_code.h"

#ifdef _AGAR_INTERNAL
# include <gi/gi_surface.h>
#else
# include <agar/gi/gi_surface.h>
#endif

/* Generic display device operations */
typedef struct gi_class {
	const AG_ObjectClass _inherit;

	int  (*GetDisplay)(void *, int w, int h, int depth, Uint flags);
#define GI_FULLSCREEN	0x01
#define GI_RESIZABLE	0x02
#define GI_NOFRAME	0x04
	int  (*ResizeDisplay)(void *, int w, int h);
	void (*ClearDisplay)(void *, GI_Color C);
	void (*LockDisplay)(void *);
	void (*UnlockDisplay)(void *);

	void (*SetCaption)(void *, const char *cap);
	void (*GetCaption)(void *, char *dst, size_t len);
	void (*SetIcon)(void *, const char *iconid);
	void (*GetIcon)(void *, char *dst, size_t len);
	
	GI_Color (*GetPixel)(void *, GI_Pos x);
	void     (*PutPixel)(void *, GI_Pos x, GI_Color c);

} GI_Class;

/* Display device */
typedef struct gi {
	struct ag_object _inherit;
	enum {
		GI_DISPLAY_DUMMY,	/* Dummy driver */
		GI_DISPLAY_GL,		/* OpenGL rendering context */
		GI_DISPLAY_HARDWARE_FB,	/* Direct hardware framebuffer */
		GI_DISPLAY_SOFTWARE_FB,	/* Software framebuffer */
		GI_DISPLAY_NETWORK,	/* Remote display over the network */
		GI_DISPLAY_ENCODER,	/* Encoder to image/video format */
		GI_DISPLAY_MISC		/* Other */
	} type;

	GI_Surface *fb;
	Uint8 *fb;			/* Framebuffer */

	Uint w, h, Bpp, bpp;		/* Geometry and depth */
	Uint pitch;			/* Scanline in bytes */
	Uint32 caps;
#define GI_CAP_HW_SURFACES	0x001	/* Hardware surfaces supported */
#define GI_CAP_HW_BLITS		0x002	/* Some blits are accelerated */
#define GI_CAP_HW_RECT_FILL	0x004	/* Rectangle fills are accelerated */
#define GI_CAP_HW_TRIANGLES	0x008	/* Triangle rasterization (1st gen) */
#define GI_CAP_HW_TEXTURING	0x010	/* Texturing (1st gen) */
#define GI_CAP_HW_TL		0x020	/* Transformation/Lighting (2nd gen) */
#define GI_CAP_HW_VPROG		0x040	/* Vertex programmability (3rd gen) */
#define GI_CAP_HW_FPROG		0x080	/* Fragment programmability (4th gen) */
#define GI_CAP_VBL_SYNCING	0x100	/* Wait on vblank is supported */

	Uint32 flags;
#define GI_DISPLAY_WINDOWED	0x200	/* Display is some window context */
#define GI_DISPLAY_RESIZABLE	0x400	/* Can be resized by user */

	int refreshRate;		/* Refresh rate (-1 = unknown) */
	struct {
		Uint32 upperBeam;	/* Upper scanline position */
		Uint32 lowerBeam;	/* Lower scanline position */
	} vblank;
} GI;

__BEGIN_DECLS
extern GI_Class giClass;

void GI_InitSubsystem(void);
void GI_DestroySubsystem(void);

static __inline__ GI_Color
GI_ColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	GI_Color C;
	C.r = r;
	C.g = g;
	C.b = b;
	C.a = 255;
	return (C);
}

static __inline__ GI_Color
GI_ColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	GI_Color C;
	C.r = r;
	C.g = g;
	C.b = b;
	C.a = a;
	return (C);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_GI_GI_H_ */
