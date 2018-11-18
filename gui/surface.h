/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _AGAR_GUI_SURFACE_H_
#define _AGAR_GUI_SURFACE_H_

#include <agar/gui/geometry.h>
#include <agar/gui/colors.h>
#include <agar/gui/begin.h>
                                 /* Bits per pixel: | 1 2 4 8  16 24 32 64 | */
typedef enum ag_surface_mode {   /* --------------- |----------------------| */
	AG_SURFACE_PACKED,       /* Packed RGB(A)   |       X  X  X  X  LG | */
	AG_SURFACE_INDEXED,      /* Palettized      | X X X X              | */
	AG_SURFACE_GRAYSCALE     /* Grayscale+Alpha |                X  LG | */
} AG_SurfaceMode;

typedef enum ag_grayscale_mode {
	AG_GRAYSCALE_BT709,		/* ITU-R Recommendation BT.709 */
	AG_GRAYSCALE_RMY,		/* R-Y algorithm */
	AG_GRAYSCALE_Y			/* Y-Grayscale (YIQ / NTSC) */
} AG_GrayscaleMode;

/* Palette of AG_Color values */
typedef struct ag_palette {
	AG_Color *_Nonnull colors;	/* Color array */
	Uint              nColors;	/* Total allocated colors */
	Uint availFirst, availLast;	/* Range of unused colors */
} AG_Palette;

/* Pixel storage information */
typedef struct ag_pixel_format {
	AG_SurfaceMode mode;		/* Image type */
	int BitsPerPixel;		/* Depth (bits/pixel) */
	int BytesPerPixel;		/* Depth (bytes/pixel) */
	int PixelsPerByte;		/* Pixels per byte (or 0 = >8bpp) */
	union {
		AG_Palette *_Nullable palette;  /* Colormap for Indexed */
		AG_GrayscaleMode graymode;      /* Grayscale-RGB method */
		struct {
			/*
			 * Number of bits lost by packing each component
			 * into our native representation.
			 */
			Uint8 Rloss, Gloss, Bloss, Aloss;
			/*
			 * Number of bits at the right of each component.
			 */
			Uint8 Rshift, Gshift, Bshift, Ashift;
			/*
			 * Pixel-wide mask over each component.
			 */
			AG_Pixel Rmask, Gmask, Bmask, Amask;
		};
	};
} AG_PixelFormat;

/* Animation frame disposal method */
typedef enum ag_anim_dispose {
	AG_DISPOSE_UNSPECIFIED,	/* No disposal specified */
	AG_DISPOSE_DO_NOT,	/* Keep previous frame's pixels */
	AG_DISPOSE_BACKGROUND,	/* Blend transparent pixels against BG
	                           (as opposed to pixels of previous frame) */
	AG_DISPOSE_PREVIOUS,	/* Restore to previous content */
} AG_AnimDispose;

typedef enum ag_anim_frame_type {
	AG_ANIM_FRAME_NONE,   /* No-op */
	AG_ANIM_FRAME_PIXELS, /* Combine rectangle of pixels (or entire graphic) */
	AG_ANIM_FRAME_COLORS, /* Replace n contiguous palette entries */
	AG_ANIM_FRAME_BLEND,  /* Blend entire graphic uniformly with a color */
	AG_ANIM_FRAME_MOVE,   /* Move a sub-rectangle of pixels by offset */
	AG_ANIM_FRAME_DATA,   /* Data block (audio, subtitles, annotations, etc) */
	AG_ANIM_FRAME_LAST
} AG_AnimFrameType;

/* Animation frame */
typedef struct ag_anim_frame {
	AG_AnimFrameType type;		/* Type of instruction */
	Uint flags;
#define AG_ANIM_FRAME_USER_INPUT 0x01	/* User input required for dispose? */
	AG_AnimDispose dispose;		/* Previous frame/rect disposal mode */
	Uint delay;			/* Delay in milliseconds */
	union {
		struct {
			Uint8 *_Nonnull p;	/* New pixels to combine */
			Uint16 x,y,w,h;		/* Destination rectangle */
		} pixels;
		struct {
			AG_Color *_Nonnull c;	/* Update colors in palette */
			Uint count;		/* Colors in array */
			Uint index;		/* Index of first entry */
		} colors;
		struct {
			AG_Color c1,c2;	/* Colors for uniform blend */
		} blend;
		struct {
			Uint16 x,y,w,h;	/* Rectangle to move */
			int xo,yo;	/* Move offsets */
		} move;
		struct {
			char *_Nullable header;	/* Header (type, size) */
			void *_Nonnull p;	/* Data block */
		} data;
	};
} AG_AnimFrame;

/* Graphics surface (or animation) */
typedef struct ag_surface {
	AG_PixelFormat format;		/* Pixel format */
	Uint flags;
#define AG_SURFACE_COLORKEY	0x01	/* Enable color key for blit as src */
#define AG_SURFACE_ALPHA	0x02	/* Enable alpha for blit as src */
#define AG_SURFACE_GL_TEXTURE	0x04	/* Use directly as OpenGL texture */
#define AG_SURFACE_MAPPED	0x08	/* Disallow AG_SurfaceFree() */
#define AG_SURFACE_STATIC	0x10	/* Don't free() in AG_SurfaceFree() */
#define AG_SURFACE_EXT_PIXELS	0x20	/* Pixels are allocated externally */
#define AG_SURFACE_ANIMATED	0x40	/* Is an animation */
#define AG_SAVED_SURFACE_FLAGS	(AG_SURFACE_COLORKEY|AG_SURFACE_ALPHA| \
                                 AG_SURFACE_ANIMATED)
	Uint w, h;			/* Size in pixels */
	Uint pitch, padding;		/* Scanline byte length, end padding */
	Uint8 *_Nullable pixels;	/* Raw pixel data */
	AG_Rect clipRect;		/* Clipping rect for blit as dst */
	AG_Pixel colorkey;		/* Color key pixel */
	AG_Component alpha;		/* Per-surface alpha */
	AG_AnimFrame *_Nullable frames;	/* Animation frames */
	Uint n;				/* Animation frame count */
} AG_Surface;

/* Animation playback context */
typedef struct ag_anim_state {
	_Nonnull AG_Mutex lock;
	AG_Surface *_Nonnull s;		/* Animated surface */
	Uint flags;
#define AG_ANIM_LOOP	 0x01		/* Loop playback */
#define AG_ANIM_PINGPONG 0x02		/* Loop in ping-pong fashion */
#define AG_ANIM_REVERSE	 0x04		/* Playback in reverse */
	int play;			/* Animation is playing */
	int f;				/* Current frame# */
	_Nullable AG_Thread th;		/* Animation thread */
} AG_AnimState;

typedef enum ag_alpha_func {
	AG_ALPHA_OVERLAY,		/* a <- MIN(src.a + dst.a, 1) */
	AG_ALPHA_ZERO,			/* a <- 0 */
	AG_ALPHA_ONE,			/* a <- 1 */
	AG_ALPHA_SRC,			/* a <- src.a */
	AG_ALPHA_DST,			/* a <- dst.a */
	AG_ALPHA_ONE_MINUS_DST,		/* a <- 1-dst.a */
	AG_ALPHA_ONE_MINUS_SRC,		/* a <- 1-src.a */
	AG_ALPHA_LAST
} AG_AlphaFn;

/* Flags for AG_SurfaceExportBMP () */
#define AG_EXPORT_BMP_NO_32BIT 0x01	/* Don't export a 32-bit BMP even when
                                           surface has an alpha channel */
#define AG_EXPORT_BMP_LEGACY   0x02	/* Export to legacy BMP format */

/* Flags for AG_SurfaceExportPNG() */
#define AG_EXPORT_PNG_ADAM7	0x01		/* Enable Adam7 interlacing */

/* Flags for AG_SurfaceExportJPEG() */
#define AG_EXPORT_JPEG_JDCT_ISLOW 0x01	/* Slow, accurate integer DCT */
#define AG_EXPORT_JPEG_JDCT_IFAST 0x02	/* Faster, less accurate integer DCT */
#define AG_EXPORT_JPEG_JDCT_FLOAT 0x04	/* Floating-point method */

__BEGIN_DECLS
extern const char *_Nonnull agAlphaFuncNames[]; /* AG_AlphaFunc name strings */
extern AG_PixelFormat *_Nullable agSurfaceFmt;  /* Standard surface format */
extern AG_GrayscaleMode agGrayscaleMode;        /* Standard grayscale/RGB map */

void AG_PixelFormatIndexed(AG_PixelFormat *_Nonnull, int);
void AG_PixelFormatGrayscale(AG_PixelFormat *_Nonnull, int);
void AG_PixelFormatRGBA(AG_PixelFormat *_Nonnull, int,
                        AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel);

AG_PixelFormat *_Nullable AG_PixelFormatDup(const AG_PixelFormat *_Nonnull)
                                           _Warn_Unused_Result;

void AG_SurfaceInit(AG_Surface *_Nonnull, const AG_PixelFormat *_Nullable,
                    Uint,Uint, Uint);

AG_Surface *_Nonnull AG_SurfaceNew(const AG_PixelFormat *_Nonnull,
                                   Uint,Uint, Uint)
		                  _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceEmpty(void)
                                    _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceIndexed(Uint,Uint, int, Uint)
                                      _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceRGB(Uint,Uint, int, Uint,
                                   AG_Pixel,AG_Pixel,AG_Pixel)
				  _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceRGBA(Uint,Uint, int, Uint,
                                    AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel)
				   _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceFromPixelsRGB(const void *_Nonnull,
                                             Uint,Uint, Uint,
                                             AG_Pixel,AG_Pixel,AG_Pixel)
	                                    _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceFromPixelsRGBA(const void *_Nonnull,
                                              Uint,Uint, Uint,
                                              AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel)
	                                     _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceStdGL(Uint, Uint)
                                    _Warn_Unused_Result;

void AG_SurfaceSetAddress(AG_Surface *_Nonnull, Uint8 *_Nonnull);
void AG_SurfaceSetColors(AG_Surface *_Nonnull, AG_Color *_Nonnull, Uint, Uint);
void AG_SurfaceSetPalette(AG_Surface *_Nonnull, const AG_Palette *_Nonnull);
void AG_SurfaceSetAvailColors(AG_Surface *_Nonnull, Uint,Uint);
void AG_SurfaceCopyPixels(AG_Surface *_Nonnull, const Uint8 *_Nonnull);
void AG_SurfaceSetPixels(AG_Surface *_Nonnull, AG_Color);

void AG_SurfaceBlit(const AG_Surface *_Nonnull, const AG_Rect *_Nullable,
		    AG_Surface *_Nonnull, int,int);

AG_Surface *_Nonnull AG_SurfaceDup(const AG_Surface *_Nonnull)
	                          _Warn_Unused_Result;

AG_Surface *_Nonnull AG_SurfaceConvert(const AG_Surface *_Nonnull,
                                       const AG_PixelFormat *_Nonnull)
	                              _Warn_Unused_Result;

AG_Surface *_Nonnull AG_SurfaceScale(const AG_Surface *_Nonnull,
                                     Uint,Uint, Uint)
				    _Warn_Unused_Result;

int  AG_SurfaceResize(AG_Surface *_Nonnull, Uint, Uint);
void AG_SurfaceCopy(AG_Surface *_Nonnull, const AG_Surface *_Nonnull);
void AG_SurfaceFree(AG_Surface *_Nonnull);

AG_Surface *_Nonnull AG_SurfaceFromSDL(void *_Nonnull)
                                      _Warn_Unused_Result;
void *_Nullable      AG_SurfaceExportSDL(const AG_Surface *_Nonnull)
                                        _Warn_Unused_Result;

AG_Surface *_Nullable AG_SurfaceFromFile(const char *_Nonnull)
                                        _Warn_Unused_Result;
int                   AG_SurfaceExportFile(const AG_Surface *_Nonnull,
                                           const char *_Nonnull);

AG_Surface *_Nullable AG_SurfaceFromBMP(const char *_Nonnull)
                                       _Warn_Unused_Result;
AG_Surface *_Nullable AG_ReadSurfaceFromBMP(AG_DataSource *_Nonnull)
                                           _Warn_Unused_Result;
int                   AG_WriteSurfaceToBMP(AG_DataSource *_Nonnull,
                                           const AG_Surface *_Nonnull, Uint8);
int                   AG_SurfaceExportBMP(const AG_Surface *_Nonnull,
                                          const char *_Nonnull, Uint8);

AG_Surface *_Nullable AG_SurfaceFromJPEG(const char *_Nonnull)
                                        _Warn_Unused_Result;
AG_Surface *_Nullable AG_ReadSurfaceFromJPEG(AG_DataSource *_Nonnull)
                                            _Warn_Unused_Result;
int                   AG_SurfaceExportJPEG(const AG_Surface *_Nonnull,
                                           const char *_Nonnull, int, Uint8);

AG_Surface *_Nullable AG_SurfaceFromPNG(const char *_Nonnull)
                                       _Warn_Unused_Result;
AG_Surface *_Nullable AG_ReadSurfaceFromPNG(AG_DataSource *_Nonnull)
                                           _Warn_Unused_Result;
AG_Surface *_Nullable AG_SurfaceFromPNGs(const char *_Nonnull, int,int,
                                         AG_AnimDispose, Uint, Uint)
                                        _Warn_Unused_Result;
int                   AG_SurfaceExportPNG(const AG_Surface *_Nonnull,
                                          const char *_Nonnull, Uint);

void AG_SurfaceBlendRGB8(AG_Surface *_Nonnull, int, int,
                         Uint8,Uint8,Uint8,Uint8, AG_AlphaFn);
void AG_SurfaceBlendRGB8_At(AG_Surface *_Nonnull, Uint8 *_Nonnull,
			    Uint8,Uint8,Uint8,Uint8, AG_AlphaFn);

void  AG_FillRect(AG_Surface *_Nonnull, const AG_Rect *_Nullable, AG_Color);

Uint8 AG_SurfaceGet8(const AG_Surface *_Nonnull, int,int)
                    _Pure_Attribute;

Uint32 AG_MapPixel32_RGB8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8)
                         _Pure_Attribute;
Uint32 AG_MapPixel32_RGBA8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8,Uint8)
                          _Pure_Attribute;
Uint32 AG_MapPixel32_RGB16(const AG_PixelFormat *_Nonnull, Uint16,Uint16,Uint16)
                          _Pure_Attribute;
Uint32 AG_MapPixel32_RGBA16(const AG_PixelFormat *_Nonnull, Uint16,Uint16,Uint16,Uint16)
                           _Pure_Attribute;

AG_Pixel AG_MapPixelIndexed(const AG_PixelFormat *_Nonnull,
                            AG_Component,AG_Component,AG_Component,AG_Component)
                           _Pure_Attribute;
AG_Pixel AG_MapPixelGrayscale(const AG_PixelFormat *_Nonnull,
                              AG_Component,AG_Component,AG_Component,AG_Component)
                             _Pure_Attribute;

AG_Color AG_GetColor32(Uint32, const AG_PixelFormat *_Nonnull)
                      _Pure_Attribute;
void     AG_GetColor32_RGB8(Uint32, const AG_PixelFormat *_Nonnull,
                            Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull);
void     AG_GetColor32_RGB16(Uint32, const AG_PixelFormat *_Nonnull,
                             Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull);
void     AG_GetColor32_RGBA8(Uint32, const AG_PixelFormat *_Nonnull,
                             Uint8 *_Nonnull, Uint8 *_Nonnull,
                             Uint8 *_Nonnull, Uint8 *_Nonnull);
void     AG_GetColor32_RGBA16(Uint32, const AG_PixelFormat *_Nonnull,
                              Uint16 *_Nonnull, Uint16 *_Nonnull,
                              Uint16 *_Nonnull, Uint16 *_Nonnull);
 
AG_Color AG_GetColor32_Gray(Uint32, AG_GrayscaleMode)
                           _Const_Attribute;
void     AG_GetColor32_Gray8(Uint32, AG_GrayscaleMode,
                             Uint8 *_Nonnull, Uint8 *_Nonnull,
                             Uint8 *_Nonnull, Uint8 *_Nonnull);
void     AG_GetColor32_Gray16(Uint32, AG_GrayscaleMode,
                              Uint16 *_Nonnull, Uint16 *_Nonnull,
                              Uint16 *_Nonnull, Uint16 *_Nonnull);

#if AG_MODEL == AG_LARGE

# define AG_SurfaceGet(S,x,y)                 AG_SurfaceGet64((S),(x),(y))
# define AG_SurfaceGet_At(S,p)                AG_SurfaceGet64_At((S),(p))
# define AG_GetColor(px,pf)                   AG_GetColor64((px),(pf))
# define AG_SurfacePut(S,x,y,px)              AG_SurfacePut64((S),(x),(y),(px))
# define AG_SurfacePut_At(S,p,px)             AG_SurfacePut64_At((S),(p),(px))
# define AG_MapPixel(pf,px)                   AG_MapPixel64((pf),(px))
# define AG_MapPixel_RGB(pf,r,g,b)            AG_MapPixel64_RGB16((pf),(r),(g),(b))
# define AG_MapPixel_RGBA(pf,r,g,b,a)         AG_MapPixel64_RGBA16((pf),(r),(g),(b),(a))
# define AG_MapPixel_RGB8(pf,r,g,b)           AG_MapPixel64_RGB8((pf),(r),(g),(b))
# define AG_MapPixel_RGBA8(pf,r,g,b,a)        AG_MapPixel64_RGBA8((pf),(r),(g),(b),(a))
# define AG_MapPixel_RGB16(pf,r,g,b)          AG_MapPixel64_RGB16((pf),(r),(g),(b))
# define AG_MapPixel_RGBA16(pf,r,g,b,a)       AG_MapPixel64_RGBA16((pf),(r),(g),(b),(a))
# define AG_SurfaceBlendRGB(S,x,y,r,g,b,a,fn) AG_SurfaceBlendRGB16((S),(x),(y),(r),(g),(b),(a),(fn))
# define AG_GetColor_RGB8(px,pf,r,g,b)        AG_GetColor64_RGB8((px),(pf),(r),(g),(b))
# define AG_GetColor_RGBA8(px,pf,r,g,b,a)     AG_GetColor64_RGBA8((px),(pf),(r),(g),(b),(a))

Uint64 AG_MapPixel64_RGB8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8)
                         _Pure_Attribute;
Uint64 AG_MapPixel64_RGBA8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8,Uint8)
                          _Pure_Attribute;
AG_Color AG_GetColor64_Gray(Uint64, AG_GrayscaleMode)
                           _Const_Attribute;
void     AG_GetColor64_Gray16(Uint64, AG_GrayscaleMode,
                              Uint16 *_Nonnull, Uint16 *_Nonnull,
                              Uint16 *_Nonnull, Uint16 *_Nonnull);
#else /* !AG_LARGE */

# define AG_SurfaceGet(S,x,y)                 AG_SurfaceGet32((S),(x),(y))
# define AG_SurfaceGet_At(S,p)                AG_SurfaceGet32_At((S),(p))
# define AG_GetColor(px,pf)                   AG_GetColor32((px),(pf))
# define AG_SurfacePut(S,x,y,px)              AG_SurfacePut32((S),(x),(y),(px))
# define AG_SurfacePut_At(S,p,px)             AG_SurfacePut32_At((S),(p),(px))
# define AG_MapPixel(pf,px)                   AG_MapPixel32((pf),(px))
# define AG_MapPixel_RGB(pf,r,g,b)            AG_MapPixel32_RGB8((pf),(r),(g),(b))
# define AG_MapPixel_RGBA(pf,r,g,b,a)         AG_MapPixel32_RGBA8((pf),(r),(g),(b),(a))
# define AG_MapPixel_RGB8(pf,r,g,b)           AG_MapPixel32_RGB8((pf),(r),(g),(b))
# define AG_MapPixel_RGBA8(pf,r,g,b,a)        AG_MapPixel32_RGBA8((pf),(r),(g),(b),(a))
# define AG_MapPixel_RGB16(pf,r,g,b)          AG_MapPixel32_RGB16((pf),(r),(g),(b))
# define AG_MapPixel_RGBA16(pf,r,g,b,a)       AG_MapPixel32_RGBA16((pf),(r),(g),(b),(a))
# define AG_SurfaceBlendRGB(s,x,y,r,g,b,a,fn) AG_SurfaceBlendRGB8((s),(x),(y),(r),(g),(b),(a),(fn))
# define AG_GetColor_RGB8(px,pf,r,g,b)        AG_GetColor32_RGB8((px),(pf),(r),(g),(b))
# define AG_GetColor_RGBA8(px,pf,r,g,b,a)     AG_GetColor32_RGBA8((px),(pf),(r),(g),(b),(a))

#endif /* !AG_LARGE */

void AG_AnimStateInit(AG_AnimState *_Nonnull, AG_Surface *_Nonnull);
void AG_AnimStateDestroy(AG_AnimState *_Nonnull);
void AG_AnimSetLoop(AG_AnimState *_Nonnull, int);
void AG_AnimSetPingPong(AG_AnimState *_Nonnull, int);
int  AG_AnimPlay(AG_AnimState *_Nonnull);
void AG_AnimStop(AG_AnimState *_Nonnull);

int AG_SurfaceAddFrame(AG_Surface *_Nonnull, const AG_Surface *_Nonnull,
                       const AG_Rect *_Nullable, AG_AnimDispose, Uint, Uint);

/* Test if a given mode and bits/pixel combination is supported. */
static __inline__ int _Const_Attribute
AG_PixelFormatIsSupported(AG_SurfaceMode mode, int BitsPerPixel)
{
	switch (mode) {
	case AG_SURFACE_PACKED:
		switch (BitsPerPixel) {
		case 8:
		case 16:
		case 24:
		case 32:
		case 48:
		case 64:
			return (1);
		}
		break;
	case AG_SURFACE_INDEXED:
		switch (BitsPerPixel) {
		case 1:
		case 2:
		case 4:
		case 8:
			return (1);
		}
		break;
	case AG_SURFACE_GRAYSCALE:
		switch (BitsPerPixel) {
		case 32:
		case 64:
			return (1);
		}
		break;
	}
	return (0);
}

/* Test if x,y lies inside surface's clipping rectangle */
static __inline__ int _Pure_Attribute
AG_SurfaceClipped(const AG_Surface *_Nonnull S, int x, int y)
{
	return (x <  S->clipRect.x ||
	        x >= S->clipRect.x + S->clipRect.w ||
	        y <  S->clipRect.y ||
	        y >= S->clipRect.y + S->clipRect.h);
}

/* Write pixel x,y in a 1-, 2-, 4-, or 8-bit surface. */
static __inline__ void
AG_SurfacePut8(AG_Surface *_Nonnull S, int x, int y, Uint8 px)
{
	Uint8 *p = S->pixels + y*S->pitch +
	                       x/S->format.PixelsPerByte;
#ifdef AG_DEBUG
	if (x < 0 || x >= S->w ||
	    y < 0 || y >= S->h)
		AG_FatalError("Illegal SurfacePut8");
#endif
	switch (S->format.BitsPerPixel) {
	case 8:
		*p = px;
		break;
	case 4:
#ifdef AG_DEBUG
		if (px > 0x0f) { AG_FatalError("Bad 4-bit pixel"); }
#endif
		switch (x % 2) {
		case 0: *p = (*p & 0xf0) | px;		break;
		case 1:	*p = (*p & 0x0f) | px << 4;	break;
		}
		break;
	case 2:
#ifdef AG_DEBUG
		if (px > 0x03) { AG_FatalError("Bad 2-bit pixel"); }
#endif
		switch (x % 4) {
		case 0:	*p = (*p & 0xfc) | px;		break;
		case 1:	*p = (*p & 0xf3) | px << 2;	break;
		case 2:	*p = (*p & 0xcf) | px << 4;	break;
		case 3:	*p = (*p & 0x3f) | px << 6;	break;
		}
		break;
	case 1:
#ifdef AG_DEBUG
		if (px > 0x01) { AG_FatalError("Bad 1-bit pixel"); }
#endif
		switch (x % 8) {
		case 0:	*p = (*p & 0xfe) | px;		break;
		case 1:	*p = (*p & 0xfd) | px << 1;	break;
		case 2:	*p = (*p & 0xfb) | px << 2;	break;
		case 3:	*p = (*p & 0xf7) | px << 3;	break;
		case 4:	*p = (*p & 0xef) | px << 4;	break;
		case 5:	*p = (*p & 0xdf) | px << 5;	break;
		case 6:	*p = (*p & 0xbf) | px << 6;	break;
		case 7:	*p = (*p & 0x7f) | px << 7;	break;
		}
		break;
	}
}

/*
 * Map an AG_Color to a 32-bit wide pixel value.
 * If the pixel format is >32-bit then return a 32-bit approximation.
 */
static __inline__ Uint32 _Pure_Attribute
AG_MapPixel32(const AG_PixelFormat *_Nonnull pf, AG_Color c)
{
#if AG_MODEL == AG_LARGE
	return AG_MapPixel32_RGBA16(pf, c.r, c.g, c.b, c.a);
#else
	return AG_MapPixel32_RGBA8(pf, c.r, c.g, c.b, c.a);
#endif
}

#if AG_MODEL == AG_LARGE
/*
 * 64-bit Wide Pixel Access Methods
 */
# define AG_EXTRACT_COMPONENT(rv, mask, shift, loss, bits)		\
	tmp = (px & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (bits - (loss << 1)));

/* Map 16-bit RGB(A) components and a 16-bit alpha to a 64-bit pixel. */
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64_RGB16(const AG_PixelFormat *_Nonnull pf,
   Uint16 r, Uint16 g, Uint16 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		return (r >> pf->Rloss) << pf->Rshift |
		       (g >> pf->Gloss) << pf->Gshift |
		       (b >> pf->Bloss) << pf->Bshift |
		      ((0xffff >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf, r,g,b,0xffff);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf, r,g,b,0xffff);
	}
}
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64_RGBA16(const AG_PixelFormat *_Nonnull pf,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		return (r >> pf->Rloss) << pf->Rshift |
		       (g >> pf->Gloss) << pf->Gshift |
		       (b >> pf->Bloss) << pf->Bshift |
		      ((a >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf, r,g,b,a);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf, r,g,b,a);
	}
}

/* Map an AG_Color to a 64-bit pixel. */
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64(const AG_PixelFormat *_Nonnull pf, AG_Color c)
{
	return AG_MapPixel64_RGBA16(pf, c.r, c.g, c.b, c.a);
}

/* Extract 16-bit RGB(A) components from a 64-bit pixel. */
static __inline__ void
AG_GetColor64_RGB16(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED: {
		Uint64 tmp;
		AG_EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		AG_EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		AG_EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		break;
	}
	case AG_SURFACE_INDEXED: {
		AG_Color *c = &pf->palette->colors[px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		break;
	}
	case AG_SURFACE_GRAYSCALE: {
		Uint16 dummy;
		AG_GetColor64_Gray16(px, pf->graymode, r,g,b, &dummy);
		break;
	}
	/* pf->mode */
	}
}
static __inline__ void
AG_GetColor64_RGBA16(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b,
    Uint16 *_Nonnull a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED: {
		Uint64 tmp;
		AG_EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		AG_EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		AG_EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		AG_EXTRACT_COMPONENT(*a, pf->Amask, pf->Ashift, pf->Aloss, 16);
		break;
	}
	case AG_SURFACE_INDEXED: {
		AG_Color *c = &pf->palette->colors[px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		*a = c->a;
		break;
	}
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor64_Gray16(px, pf->graymode, r,g,b,a);
		break;
	}
}

/* Extract compressed 8-bit RGB(A) components from a 64-bit pixel. */
static __inline__ void
AG_GetColor64_RGB8(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b)
{
	Uint16 R,G,B;

	AG_GetColor64_RGB16(px, pf, &R,&G,&B);
	*r = AG_16to8(R);
	*g = AG_16to8(G);
	*b = AG_16to8(B);
}
static __inline__ void
AG_GetColor64_RGBA8(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b,
    Uint8 *_Nonnull a)
{
	Uint16 R,G,B,A;

	AG_GetColor64_RGBA16(px, pf, &R,&G,&B,&A);
	*r = AG_16to8(R);
	*g = AG_16to8(G);
	*b = AG_16to8(B);
	*a = AG_16to8(A);
}

/* Extract an AG_Color from a 64-bit pixel */
static __inline__ AG_Color _Pure_Attribute
AG_GetColor64(Uint64 px, const AG_PixelFormat *_Nonnull pf)
{
	AG_Color C;

	switch (pf->mode) {
	case AG_SURFACE_PACKED: {
		Uint64 tmp;
		AG_EXTRACT_COMPONENT(C.r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		AG_EXTRACT_COMPONENT(C.g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		AG_EXTRACT_COMPONENT(C.b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		AG_EXTRACT_COMPONENT(C.a, pf->Amask, pf->Ashift, pf->Aloss, 16);
		break;
	}
	case AG_SURFACE_INDEXED:
		C = pf->palette->colors[px % pf->palette->nColors];
		break;
	case AG_SURFACE_GRAYSCALE:
		C = AG_GetColor64_Gray(px, pf->graymode);
		break;
	}
	return (C);
}

#undef AG_EXTRACT_COMPONENT
#endif /* AG_LARGE */

/*
 * Return the (8- to 64-bit) pixel at address p in surface S.
 * If S is > 32-bit, return a compressed 32-bit approximation.
 */
static __inline__ Uint32 _Pure_Attribute
AG_SurfaceGet32_At(const AG_Surface *_Nonnull S, const Uint8 *_Nonnull p)
{
	switch (S->format.BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:
	case 48:
		return AG_MapPixel32(&S->format,
		    AG_GetColor64(*(Uint64 *)p, &S->format));
#endif
	case 32:
		return (*(Uint32 *)p);
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		return ((p[0] << 16) +
		        (p[1] << 8) +
		         p[2]);
#else
		return  (p[0] +
		        (p[1] << 8) +
		        (p[2] << 16));
#endif
	case 16:
		return (*(Uint16 *)p);
	}
	return (*p);
}
	
/*
 * Return the (1- to 64-bit) pixel at coordinates x,y in surface S.
 * If S is > 32-bit, return a compressed 32-bit approximation.
 */
static __inline__ Uint32 _Pure_Attribute
AG_SurfaceGet32(const AG_Surface *_Nonnull S, int x, int y)
{
#ifdef AG_DEBUG
	if (x < 0 || y < 0 || x >= S->w || y >= S->h)
		AG_FatalError("Illegal SurfaceGet32");
#endif
	if (S->format.BitsPerPixel < 8) {
		return (Uint32)AG_SurfaceGet8(S, x,y);
	} else {
		return AG_SurfaceGet32_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel);
	}
}

#if AG_MODEL == AG_LARGE
/*
 * Return the (8- to 64-bit) pixel at address p in surface S.
 */
static __inline__ Uint64 _Pure_Attribute
AG_SurfaceGet64_At(const AG_Surface *_Nonnull S, const Uint8 *_Nonnull p)
{
	switch (S->format.BitsPerPixel) {
	case 64:
		return (*(Uint64 *)p);
	case 32:
		return (*(Uint32 *)p);
	case 24:
# if AG_BYTEORDER == AG_BIG_ENDIAN
		return ((p[0] << 16) +
		        (p[1] << 8) +
		         p[2]);
# else
		return  (p[0] +
		        (p[1] << 8) +
		        (p[2] << 16));
# endif
	case 16:
		return (*(Uint16 *)p);
	}
	return (*p);
}

/* Return the (1- to 64-bit) pixel at coordinates x,y in surface S. */
static __inline__ Uint64 _Pure_Attribute
AG_SurfaceGet64(const AG_Surface *_Nonnull S, int x, int y)
{
# ifdef AG_DEBUG
	if (x < 0 || y < 0 || x >= S->w || y >= S->h) {
		AG_SetError("Illegal SurfaceGet64(%d,%d > %ux%u)", x,y,
		    S->w, S->h);
		AG_FatalError(NULL);
	}
# endif
	if (S->format.BitsPerPixel < 8) {
		return (Uint64)AG_SurfaceGet8(S, x,y);
	} else {
		return AG_SurfaceGet64_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel);
	}
}
#endif /* AG_LARGE */

/* Write to the (8- to 32-bit) pixel at address p in surface S. */
static __inline__ void
AG_SurfacePut32_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p, Uint32 px)
{
	switch (S->format.BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:                                             /* Decompressed */
		*(Uint64 *)p = AG_MapPixel64(&S->format,
		    AG_GetColor32(px, &S->format));
		break;
#endif
	case 32:
		*(Uint32 *)p = px;
		break;
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#else
		p[0] = (px)       & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px >> 16) & 0xff;
#endif
		break;
	case 16:
		*(Uint16 *)p = (Uint16)px;
		break;
	default:
		*p = (Uint8)px;
		break;
	}
}

/* Write to the (1- to 64-bit) pixel at coordinates x,y in surface S. */
static __inline__ void
AG_SurfacePut32(AG_Surface *_Nonnull S, int x, int y, Uint32 px)
{
	if (S->format.BitsPerPixel < 8) {
		AG_SurfacePut8(S, x,y, (Uint8)px);
	} else {
		AG_SurfacePut32_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel, px);
	}
}

#if AG_MODEL == AG_LARGE
/* Write the pixel at address p in an (8- to 64-bpp) surface S. */
static __inline__ void
AG_SurfacePut64_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p, Uint64 px)
{
	switch (S->format.BitsPerPixel) {
	case 64:
		*(Uint64 *)p = px;
		break;
	case 32:
		*(Uint32 *)p = px;
		break;
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#else
		p[0] = (px)       & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px >> 16) & 0xff;
#endif
		break;
	case 16:
		*(Uint16 *)p = (Uint16)px;
		break;
	default:
		*p = (Uint8)px;
		break;
	}
}

/* Write the pixel at x,y in a (1- to 64-bit) surface S. */
static __inline__ void
AG_SurfacePut64(AG_Surface *_Nonnull S, int x, int y, Uint64 px)
{
	if (S->format.BitsPerPixel < 8) {
		AG_SurfacePut8(S, x,y, (Uint8)px);
	} else {
		AG_SurfacePut64_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel, px);
	}
}
#endif /* AG_LARGE */

/*
 * Blend a color c with the pixel at address p and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
static __inline__ void
AG_SurfaceBlend_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p, AG_Color c,
    AG_AlphaFn fn)
{
	AG_Pixel px = AG_SurfaceGet_At(S,p);
	AG_Color pc;

	if ((S->flags & AG_SURFACE_COLORKEY) &&		/* No blending needed */
	    (px == S->colorkey)) {
		AG_SurfacePut_At(S, p, AG_MapPixel(&S->format, c));
		return;
	}
	pc = AG_GetColor(px, &S->format);
	switch (fn) {
	case AG_ALPHA_OVERLAY:
		pc.a = ((pc.a + c.a) > AG_COLOR_LAST) ? AG_COLOR_LAST :
		                                        (pc.a + c.a);
		break;
	case AG_ALPHA_DST:        /* pc.a = pc.a */        break;
	case AG_ALPHA_SRC:           pc.a = c.a;           break;
	case AG_ALPHA_ZERO:          pc.a = 0;             break;
	case AG_ALPHA_ONE_MINUS_DST: pc.a = AG_COLOR_LAST - pc.a; break;
	case AG_ALPHA_ONE_MINUS_SRC: pc.a = AG_COLOR_LAST - c.a;  break;
	case AG_ALPHA_ONE: default:  pc.a = AG_COLOR_LAST;        break;
	}
	AG_SurfacePut_At(S, p,
	    AG_MapPixel_RGBA(&S->format,
	        pc.r + (((c.r - pc.r)*c.a) >> AG_COMPONENT_BITS),
	        pc.g + (((c.g - pc.g)*c.a) >> AG_COMPONENT_BITS),
	        pc.b + (((c.b - pc.b)*c.a) >> AG_COMPONENT_BITS),
	        pc.a));
}

/*
 * Blend a color c with the pixel at x,y and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
static __inline__ void
AG_SurfaceBlend(AG_Surface *_Nonnull S, int x, int y, AG_Color c, AG_AlphaFn fn)
{
	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    c, fn);
}

#if AG_MODEL == AG_LARGE
/*
 * Blend 16-bit RGBA components with the pixel at x,y and overwrite it with
 * the result. No clipping. Target pixel alpha is set according to fn.
 */
static __inline__ void
AG_SurfaceBlendRGB16(AG_Surface *_Nonnull S, int x, int y,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
{
	AG_Color c = { r,g,b,a };

	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    c, fn);
}

/*
 * Blend 16-bit RGBA components with the pixel at address p and overwrite it
 * with the result.  No clipping is done. Set target alpha according to fn.
 */
static __inline__ void
AG_SurfaceBlendRGB16_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
{
	AG_Color c = { r,g,b,a };

	AG_SurfaceBlend_At(S,p, c, fn);
}
#endif /* AG_LARGE */

/* Initialize a packed-pixel format from RGB component masks (no alpha channel) */
static __inline__ void
AG_PixelFormatRGB(AG_PixelFormat *_Nonnull pf, int BitsPerPixel,
    Uint64 Rmask, Uint64 Gmask, Uint64 Bmask)
{
	AG_PixelFormatRGBA(pf, BitsPerPixel, Rmask, Gmask, Bmask, 0);
}

/*
 * Test whether two pixel formats are identical.
 * In indexed mode, include both palettes in the comparison.
 */
static __inline__ int _Pure_Attribute
AG_PixelFormatCompare(const AG_PixelFormat *_Nonnull a,
                      const AG_PixelFormat *_Nonnull b)
{
	if (a->mode != b->mode ||
	    a->BitsPerPixel != b->BitsPerPixel) {
		return (1);
	}
	switch (a->mode) {
	case AG_SURFACE_PACKED:
		return !(a->Rmask == b->Rmask &&
			 a->Gmask == b->Gmask &&
			 a->Bmask == b->Bmask &&
			 a->Amask == b->Amask);
	case AG_SURFACE_INDEXED:
		if (a->palette->nColors != b->palette->nColors) {
			return (1);
		}
		return memcmp(a->palette->colors, b->palette->colors,
		              a->palette->nColors * sizeof(AG_Color));
	case AG_SURFACE_GRAYSCALE:
		return !(a->graymode == b->graymode);
	}
}

/* Release all resources possibly allocated by an AG_PixelFormat. */
static __inline__ void
AG_PixelFormatFree(AG_PixelFormat *_Nonnull pf)
{
	if (pf->mode == AG_SURFACE_INDEXED) {
		AG_Free(pf->palette->colors);
		AG_Free(pf->palette);
	}
}

/* Set the source alpha flag and per-surface alpha. */
static __inline__ void
AG_SurfaceSetAlpha(AG_Surface *_Nonnull S, Uint flags, AG_Component alpha)
{
	if (flags & AG_SURFACE_ALPHA) {
		S->flags |= AG_SURFACE_ALPHA;
	} else {
		S->flags &= ~(AG_SURFACE_ALPHA);
	}
	S->alpha = alpha;
}

/* Set the source colorkey flag and per-surface colorkey. */
static __inline__ void
AG_SurfaceSetColorKey(AG_Surface *_Nonnull S, Uint flags, AG_Pixel colorkey)
{
	if (flags & AG_SURFACE_COLORKEY) {
		S->flags |= AG_SURFACE_COLORKEY;
	} else {
		S->flags &= ~(AG_SURFACE_COLORKEY);
	}
	S->colorkey = colorkey;
}

#ifdef AG_LEGACY
# define AG_SurfaceStdRGB(w,h) \
         AG_SurfaceRGB((w),(h), agSurfaceFmt->BitsPerPixel,0, \
             agSurfaceFmt->Rmask, agSurfaceFmt->Gmask, agSurfaceFmt->Bmask)
# define AG_SurfaceStdRGBA(w,h) \
         AG_SurfaceRGBA((w),(h), agSurfaceFmt->BitsPerPixel,0, \
	    agSurfaceFmt->Rmask, agSurfaceFmt->Gmask, agSurfaceFmt->Bmask, \
	    agSurfaceFmt->Amask)
/* -> AG_SurfaceNew(agSurfaceFmt, ...) */

typedef AG_Surface AG_Anim;
# define AG_AnimNew		AG_SurfaceNew
# define AG_AnimEmpty		AG_SurfaceEmpty
# define AG_AnimIndexed		AG_SurfaceIndexed
# define AG_AnimRGB		AG_SurfaceRGB
# define AG_AnimRGBA		AG_SurfaceRGBA
# define AG_AnimSetPalette	AG_SurfaceSetColors
# define AG_AnimDup		AG_SurfaceDup
# define AG_AnimResize		AG_SurfaceResize
# define AG_AnimFree		AG_SurfaceFree
# define AG_AnimSetColorkey	AG_SurfaceSetColorkey
# define AG_AnimSetAlpha	AG_SurfaceSetAlpha
# define AG_AnimSetOrigFPS(s,f)
# define AG_AnimSetFPS(ast,f)
/* -> (now handled in AG_Surface) */

typedef AG_AlphaFn AG_BlendFn;	/* less confusing */

/* Flags renamed for readability */
# define AG_SRCCOLORKEY		AG_SURFACE_COLORKEY
# define AG_SRCALPHA		AG_SURFACE_ALPHA
# define AG_SURFACE_GLTEXTURE   AG_SURFACE_GL_TEXTURE

/*
 * Pixel access routines before Agar 1.6 (which added support for 16-bit
 * component precision and 1-, 2- and 4-bpp palettized modes (which are x,y
 * addressable only).
 */
# define AG_CLIPPED_PIXEL(s, x,y) AG_SurfaceClipped((s), (x),(y))
# define AG_GET_PIXEL(s, p)       AG_SurfaceGet32_At((s), (p))
# define AG_GET_PIXEL2(s, x,y)    AG_SurfaceGet32((s), (x),(y))
# define AG_PUT_PIXEL(s, p, c)    AG_SurfacePut32_At((s), (p), (c))
# define AG_PUT_PIXEL2(s, x,y, c) AG_SurfacePut32((s), (x),(y), (c))
# define AG_PUT_PIXEL2_CLIPPED(s, x,y, c) \
	if (!AG_SurfaceClipped((s), (x),(y))) \
		AG_SurfacePut32((s), (x),(y), (c))
# define AG_BLEND_RGBA(s, p, r,g,b,a, fn)				\
         AG_SurfaceBlendRGB8_At((s), (p), (r),(g),(b),(a), (fn))
# define AG_BLEND_RGBA2(s, x,y, r,g,b,a, fn)				\
         AG_SurfaceBlendRGB8((s), (x),(y), (r),(g),(b),(a), (fn))
# define AG_BLEND_RGBA2_CLIPPED(s, x,y, r,g,b,a, fn)			\
	if (!AG_SurfaceClipped((s), (x),(y))) 				\
		AG_SurfaceBlendRGB8((s), (x),(y), (r),(g),(b),(a), (fn))
# define AG_ALPHA_TRANSPARENT AG_TRANSPARENT
# define AG_ALPHA_OPAQUE      AG_OPAQUE

void        AG_SurfaceLock(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_SurfaceUnlock(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
Uint32      AG_MapRGB(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32_RGB8(pf, r,g,b) */
Uint32      AG_MapRGBA(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32_RGBA8(pf, r,g,b,a) */
Uint32      AG_MapPixelRGB(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32_RGB8(pf, r,g,b) */
Uint32      AG_MapPixelRGBA(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32_RGBA8(pf, r,g,b,a) */
Uint32      AG_MapColorRGB(const AG_PixelFormat *_Nonnull, AG_Color) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32(pf, c); */
Uint32      AG_MapColorRGBA(const AG_PixelFormat *_Nonnull, AG_Color) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_MapPixel32(pf, c) */
void        AG_GetRGB(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_GetPixelRGB(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       -> AG_GetColor32_RGB8(pf, px, r,g,b) */
void        AG_GetRGBA(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_GetPixelRGBA(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       -> AG_GetColor32_RGBA8(pf, px, r,g,b,a) */
AG_Color    AG_GetColorRGB(Uint32, const AG_PixelFormat *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       -> AG_GetColor32(px, pf), c.a=0xffff */
AG_Color    AG_GetColorRGBA(Uint32, const AG_PixelFormat *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       -> AG_GetColor32(px, pf) */
AG_Surface *_Nonnull AG_DupSurface(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       -> AG_SurfaceDup(s) */
int         AG_SamePixelFmt(const AG_Surface *_Nonnull, const AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       -> AG_PixelFormatCompare(a,b) */
int         AG_ScaleSurface(const AG_Surface *_Nonnull, Uint16, Uint16, AG_Surface *_Nonnull *_Nullable) DEPRECATED_ATTRIBUTE;
/*       -> AG_SurfaceScale(s, w,h, 0) */
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SURFACE_H_ */
