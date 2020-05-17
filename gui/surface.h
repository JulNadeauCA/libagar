/*
 * Copyright (c) 2009-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
			       /* Bits per pixel: | 1  2  4  8  16 24 32 64 |*/
typedef enum ag_surface_mode { /* --------------- |-------------------------|*/
	AG_SURFACE_PACKED,     /* Packed RGB(A)   |       SM    SM MD MD LG |*/
	AG_SURFACE_INDEXED,    /* Palettized      | SM SM SM SM             |*/
	AG_SURFACE_GRAYSCALE   /* Grayscale+Alpha |             SM    MD LG |*/
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
	Uint32 _pad;
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
#define AG_SURFACE_TRACE	0x80	/* Enable debugging */
#define AG_SAVED_SURFACE_FLAGS	(AG_SURFACE_COLORKEY | AG_SURFACE_ALPHA | \
                                 AG_SURFACE_ANIMATED)
	Uint w, h;			/* Size in pixels */
	Uint pitch;			/* Scanline byte length */
	Uint8 *_Nullable pixels;	/* Raw pixel data */
	AG_Rect clipRect;		/* Clipping rect for blit as dst */
	AG_AnimFrame *_Nullable frames;	/* Animation frames */
	Uint n;				/* Animation frame count */
	Uint padding;			/* Scanline end padding */
	AG_Pixel colorkey;		/* Color key pixel */
	AG_Component alpha;		/* Per-surface alpha */
#if AG_MODEL == AG_LARGE
	Uint8 _pad[6];
#elif AG_MODEL == AG_MEDIUM
	Uint8 _pad[3];
#else
	Uint8 _pad[7];
#endif
} AG_Surface;

/* Animation playback context */
typedef struct ag_anim_state {
	_Nonnull_Mutex AG_Mutex lock;
	AG_Surface *_Nonnull s;		/* Animated surface */
	Uint flags;
#define AG_ANIM_LOOP	 0x01		/* Loop playback */
#define AG_ANIM_PINGPONG 0x02		/* Loop in ping-pong fashion */
#define AG_ANIM_REVERSE	 0x04		/* Playback in reverse */
#define AG_ANIM_PLAYING  0x08		/* Animation is playing */
	int f;				/* Current frame# */
#ifdef AG_THREADS
	_Nullable_Thread AG_Thread th;	/* Animation thread */
#endif
} AG_AnimState;

/* Texture environment mode. See glTextEnv(3G). */
typedef enum ag_texture_env_mode {
	AG_TEXTURE_ENV_MODULATE,
	AG_TEXTURE_ENV_DECAL,
	AG_TEXTURE_ENV_BLEND,
	AG_TEXTURE_ENV_REPLACE		/* Needs OpenGL >= 1.1 */
} AG_TextureEnvMode;

/* Pixel arithmetic for alpha blending. See glBlendFunc(3G). */
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
extern const char *_Nonnull agSurfaceModeNames[]; /* AG_Surface modes */
extern const char *_Nonnull agAlphaFuncNames[];   /* AG_AlphaFunc modes */

extern AG_PixelFormat *_Nullable agSurfaceFmt;  /* Standard surface format */
extern AG_GrayscaleMode agGrayscaleMode;        /* Standard grayscale/RGB map */

void AG_PixelFormatIndexed(AG_PixelFormat *_Nonnull, int);
void AG_PixelFormatGrayscale(AG_PixelFormat *_Nonnull, int);
void AG_PixelFormatRGB(AG_PixelFormat *_Nonnull, int, AG_Pixel,AG_Pixel,AG_Pixel);
void AG_PixelFormatRGBA(AG_PixelFormat *_Nonnull, int,
                        AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel);

AG_PixelFormat *_Nullable AG_PixelFormatDup(const AG_PixelFormat *_Nonnull)
                                           _Warn_Unused_Result;

void                 AG_SurfaceInit(AG_Surface *_Nonnull,
                                    const AG_PixelFormat *_Nullable,
				    Uint,Uint, Uint);
AG_Surface *_Nonnull AG_SurfaceNew(const AG_PixelFormat *_Nonnull,
                                   Uint,Uint, Uint)
		                  _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceEmpty(void)
                                    _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceIndexed(Uint,Uint, int, Uint)
                                      _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceGrayscale(Uint,Uint, int, Uint)
                                        _Warn_Unused_Result;

AG_Surface *_Nonnull AG_SurfaceRGB(Uint,Uint, int, Uint,
                                   AG_Pixel,AG_Pixel,AG_Pixel)
				  _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceRGBA(Uint,Uint, int, Uint,
                                    AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel)
				   _Warn_Unused_Result;

#define AG_SurfaceStdRGB(w,h) \
        AG_SurfaceRGB((w),(h), agSurfaceFmt->BitsPerPixel, 0, \
	                       agSurfaceFmt->Rmask, \
			       agSurfaceFmt->Gmask, \
			       agSurfaceFmt->Bmask)

#define AG_SurfaceStdRGBA(w,h) \
        AG_SurfaceRGBA((w),(h), \
	               agSurfaceFmt->BitsPerPixel, 0, \
                       agSurfaceFmt->Rmask, \
	               agSurfaceFmt->Gmask, \
	               agSurfaceFmt->Bmask, \
	               agSurfaceFmt->Amask)

AG_Surface *_Nonnull AG_SurfaceFromPixelsRGB(const void *_Nonnull,
                                             Uint,Uint, Uint,
                                             AG_Pixel,AG_Pixel,AG_Pixel)
	                                    _Warn_Unused_Result;
AG_Surface *_Nonnull AG_SurfaceFromPixelsRGBA(const void *_Nonnull,
                                              Uint,Uint, Uint,
                                              AG_Pixel,AG_Pixel,AG_Pixel,AG_Pixel)
	                                     _Warn_Unused_Result;

#include <agar/config/have_opengl.h>

void AG_SurfaceSetAddress(AG_Surface *_Nonnull, Uint8 *_Nonnull);
void AG_SurfaceSetColors(AG_Surface *_Nonnull, AG_Color *_Nonnull, Uint, Uint);
void AG_SurfaceSetPalette(AG_Surface *_Nonnull, const AG_Palette *_Nonnull);
void AG_SurfaceCopyPixels(AG_Surface *_Nonnull, const Uint8 *_Nonnull);
void AG_SurfaceSetPixels(AG_Surface *_Nonnull, const AG_Color *_Nonnull);

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

void  AG_FillRect(AG_Surface *_Nonnull, const AG_Rect *_Nullable,
                  const AG_Color *_Nonnull);

Uint8 AG_SurfaceGet8(const AG_Surface *_Nonnull, int,int)
                    _Pure_Attribute;
void  AG_SurfacePut8(AG_Surface *_Nonnull, int,int, Uint8);

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

void AG_GetColor32(AG_Color *_Nonnull, Uint32, const AG_PixelFormat *_Nonnull);
void AG_GetColor32_RGB8(Uint32, const AG_PixelFormat *_Nonnull,
                        Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull);
void AG_GetColor32_RGB16(Uint32, const AG_PixelFormat *_Nonnull,
                         Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull);
void AG_GetColor32_RGBA8(Uint32, const AG_PixelFormat *_Nonnull,
                         Uint8 *_Nonnull, Uint8 *_Nonnull,
                         Uint8 *_Nonnull, Uint8 *_Nonnull);
void AG_GetColor32_RGBA16(Uint32, const AG_PixelFormat *_Nonnull,
                          Uint16 *_Nonnull, Uint16 *_Nonnull,
                          Uint16 *_Nonnull, Uint16 *_Nonnull);
void AG_GetColor32_Gray(AG_Color *_Nonnull, Uint32, AG_GrayscaleMode);
void AG_GetColor32_Gray8(Uint32, AG_GrayscaleMode,
                         Uint8 *_Nonnull, Uint8 *_Nonnull,
                         Uint8 *_Nonnull, Uint8 *_Nonnull);
void AG_GetColor32_Gray16(Uint32, AG_GrayscaleMode,
                          Uint16 *_Nonnull, Uint16 *_Nonnull,
                          Uint16 *_Nonnull, Uint16 *_Nonnull);

#if AG_MODEL == AG_LARGE
# define AG_SurfaceGet(S,x,y)                 AG_SurfaceGet64((S),(x),(y))
# define AG_SurfaceGet_At(S,p)                AG_SurfaceGet64_At((S),(p))
# define AG_GetColor(c,px,pf)                 AG_GetColor64((c),(px),(pf))
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
# define AG_GetColor_RGB16(px,pf,r,g,b)       AG_GetColor64_RGB16((px),(pf),(r),(g),(b))
# define AG_GetColor_RGBA16(px,pf,r,g,b,a)    AG_GetColor64_RGBA16((px),(pf),(r),(g),(b),(a))

Uint64 AG_MapPixel64_RGB8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8)
                         _Pure_Attribute;
Uint64 AG_MapPixel64_RGBA8(const AG_PixelFormat *_Nonnull, Uint8,Uint8,Uint8,Uint8)
                          _Pure_Attribute;

void AG_GetColor64(AG_Color *_Nonnull, Uint64, const AG_PixelFormat *_Nonnull);
void AG_GetColor64_Gray(AG_Color *_Nonnull, Uint64, AG_GrayscaleMode);
void AG_GetColor64_Gray16(Uint64, AG_GrayscaleMode,
                          Uint16 *_Nonnull, Uint16 *_Nonnull,
                          Uint16 *_Nonnull, Uint16 *_Nonnull);
#else /* !AG_LARGE */

# define AG_SurfaceGet(S,x,y)                 AG_SurfaceGet32((S),(x),(y))
# define AG_SurfaceGet_At(S,p)                AG_SurfaceGet32_At((S),(p))
# define AG_GetColor(c,px,pf)                 AG_GetColor32((c),(px),(pf))
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
# define AG_GetColor_RGB16(px,pf,r,g,b)       AG_GetColor32_RGB16((px),(pf),(r),(g),(b))
# define AG_GetColor_RGBA16(px,pf,r,g,b,a)    AG_GetColor32_RGBA16((px),(pf),(r),(g),(b),(a))
#endif /* AG_LARGE */

void AG_AnimStateInit(AG_AnimState *_Nonnull, AG_Surface *_Nonnull);
void AG_AnimStateDestroy(AG_AnimState *_Nonnull);
void AG_AnimSetLoop(AG_AnimState *_Nonnull, int);
void AG_AnimSetPingPong(AG_AnimState *_Nonnull, int);
int  AG_AnimPlay(AG_AnimState *_Nonnull);
void AG_AnimStop(AG_AnimState *_Nonnull);

int  AG_SurfaceAddFrame(AG_Surface *_Nonnull, const AG_Surface *_Nonnull,
                        const AG_Rect *_Nullable, AG_AnimDispose, Uint, Uint);

/*
 * Inlinables
 */
#ifdef AG_INLINE_SURFACE
# define AG_INLINE_HEADER
# include <agar/gui/inline_surface.h>
#else /* !AG_INLINE_SURFACE */

int    ag_pixel_format_is_supported(AG_SurfaceMode, int) _Const_Attribute;
int    ag_surface_clipped(const AG_Surface *_Nonnull, int,int) _Pure_Attribute;

Uint32 ag_map_pixel32(const AG_PixelFormat *_Nonnull, const AG_Color *_Nonnull)
                     _Pure_Attribute;
# if AG_MODEL == AG_LARGE						/* LG */
Uint64 ag_map_pixel64(const AG_PixelFormat *_Nonnull, const AG_Color *_Nonnull)
                     _Pure_Attribute;
Uint64 ag_map_pixel64_rgb16(const AG_PixelFormat *_Nonnull,
                            Uint16,Uint16,Uint16) _Pure_Attribute;
Uint64 ag_map_pixel64_rgba16(const AG_PixelFormat *_Nonnull,
                             Uint16,Uint16,Uint16,Uint16) _Pure_Attribute;

void ag_get_color64_rgb16(Uint64, const AG_PixelFormat *_Nonnull,
                          Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull);
void ag_get_color64_rgba16(Uint64, const AG_PixelFormat *_Nonnull,
                           Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull,
                           Uint16 *_Nonnull);
void ag_get_color64_rgb8(Uint64, const AG_PixelFormat *_Nonnull,
                         Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull);
void ag_get_color64_rgba8(Uint64, const AG_PixelFormat *_Nonnull,
                          Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull,
                          Uint8 *_Nonnull);

Uint64 ag_surface_get64_at(const AG_Surface *_Nonnull, const Uint8 *_Nonnull)
                          _Pure_Attribute;
Uint64 ag_surface_get64(const AG_Surface *_Nonnull, int,int) _Pure_Attribute;

void ag_surface_put64_at(AG_Surface *_Nonnull, Uint8 *_Nonnull, Uint64);
void ag_surface_put64(AG_Surface *_Nonnull, int,int, Uint64);

# endif /* AG_LARGE */

Uint32 ag_surface_get32_at(const AG_Surface *_Nonnull, const Uint8 *_Nonnull)
                          _Pure_Attribute;
Uint32 ag_surface_get32(const AG_Surface *_Nonnull, int,int) _Pure_Attribute;

void ag_surface_put32_at(AG_Surface *_Nonnull, Uint8 *_Nonnull, Uint32);
void ag_surface_put32(AG_Surface *_Nonnull, int,int, Uint32);
void ag_surface_blend_at(AG_Surface *_Nonnull, Uint8 *_Nonnull,
                         const AG_Color *_Nonnull, AG_AlphaFn);
void ag_surface_blend(AG_Surface *_Nonnull, int,int, const AG_Color *_Nonnull,
                      AG_AlphaFn);
void ag_surface_blend_rgb16(AG_Surface *_Nonnull, int,int,
                            Uint16,Uint16,Uint16,Uint16, AG_AlphaFn);
void ag_surface_blend_rgb16_at(AG_Surface *_Nonnull, Uint8 *_Nonnull,
                               Uint16,Uint16,Uint16,Uint16, AG_AlphaFn);
int  ag_pixel_format_compare(const AG_PixelFormat *_Nonnull,
                             const AG_PixelFormat *_Nonnull);
void ag_pixel_format_free(AG_PixelFormat *_Nonnull);

void ag_surface_set_alpha(AG_Surface *_Nonnull, Uint, AG_Component);
void ag_surface_set_colorkey(AG_Surface *_Nonnull, Uint, AG_Pixel);

# define AG_PixelFormatIsSupported(m,bpp)	ag_pixel_format_is_supported((m),(bpp))
# define AG_SurfaceClipped(S,x,y)		ag_surface_clipped((S),(x),(y))
# define AG_MapPixel32(pf,c)			ag_map_pixel32((pf),(c))
# define AG_MapPixel64_RGB16(pf,r,g,b)		ag_map_pixel64_rgb16((pf),(r),(g),(b))
# define AG_MapPixel64_RGBA16(pf,r,g,b,a)	ag_map_pixel64_rgba16((pf),(r),(g),(b),(a))
# define AG_MapPixel64(pf,c)			ag_map_pixel64((pf),(c))
# define AG_GetColor64_RGB16(px,pf,r,g,b)	ag_get_color64_rgb16((px),(pf),(r),(g),(b))
# define AG_GetColor64_RGBA16(px,pf,r,g,b,a)	ag_get_color64_rgba16((px),(pf),(r),(g),(b),(a))
# define AG_GetColor64_RGB8(px,pf,r,g,b)	ag_get_color64_rgb8((px),(pf),(r),(g),(b))
# define AG_GetColor64_RGBA8(px,pf,r,g,b,a)	ag_get_color64_rgba8((px),(pf),(r),(g),(b),(a))
# define AG_SurfaceGet32_At(S,p)		ag_surface_get32_at((S),(p))
# define AG_SurfaceGet32(S,x,y)			ag_surface_get32((S),(x),(y))
# define AG_SurfaceGet64_At(S,p)		ag_surface_get64_at((S),(p))
# define AG_SurfaceGet64(S,x,y)			ag_surface_get64((S),(x),(y))
# define AG_SurfacePut32_At(S,p,px)		ag_surface_put32_at((S),(p),(px))
# define AG_SurfacePut32(S,x,y,px)		ag_surface_put32((S),(x),(y),(px))
# define AG_SurfacePut64_At(S,p,px)		ag_surface_put64_at((S),(p),(px))
# define AG_SurfacePut64(S,x,y,px)		ag_surface_put64((S),(x),(y),(px))
# define AG_SurfaceBlend_At(S,p,c,fn)		ag_surface_blend_at((S),(p),(c),(fn))
# define AG_SurfaceBlend(S,x,y,c,fn)		ag_surface_blend((S),(x),(y),(c),(fn))
# define AG_SurfaceBlendRGB16(S,x,y,r,g,b,a,fn)	ag_surface_blend_rgb16((S),(x),(y),(r),(g),(b),(a),(fn))
# define AG_SurfaceBlendRGB16_At(S,p,r,g,b,a,fn) ag_surface_blend_rgb16_at((S),(p),(r),(g),(b),(a),(fn))
# define AG_PixelFormatCompare(a,b)		ag_pixel_format_compare((a),(b))
# define AG_PixelFormatFree(pf)			ag_pixel_format_free(pf)
# define AG_SurfaceSetAlpha(S,fl,alpha)		ag_surface_set_alpha((S),(fl),(alpha))
# define AG_SurfaceSetColorKey(S,fl,ckey)	ag_surface_set_colorkey((S),(fl),(ckey))
#endif /* !AG_INLINE_SURFACE */

#ifdef AG_LEGACY

# define AG_SRCCOLORKEY       AG_SURFACE_COLORKEY
# define AG_SRCALPHA          AG_SURFACE_ALPHA
# define AG_SURFACE_GLTEXTURE AG_SURFACE_GL_TEXTURE
# define AG_ALPHA_TRANSPARENT AG_TRANSPARENT
# define AG_ALPHA_OPAQUE      AG_OPAQUE
# define AG_BlendFn           AG_AlphaFn
# define AG_AnimNew           AG_SurfaceNew
# define AG_AnimEmpty         AG_SurfaceEmpty
# define AG_AnimIndexed       AG_SurfaceIndexed
# define AG_AnimRGB           AG_SurfaceRGB
# define AG_AnimRGBA          AG_SurfaceRGBA
# define AG_AnimSetPalette    AG_SurfaceSetColors
# define AG_AnimDup           AG_SurfaceDup
# define AG_AnimResize        AG_SurfaceResize
# define AG_AnimFree          AG_SurfaceFree
# define AG_AnimSetColorkey   AG_SurfaceSetColorkey
# define AG_AnimSetAlpha      AG_SurfaceSetAlpha
# define AG_AnimSetOrigFPS(s,f)
# define AG_AnimSetFPS(ast,f)
/* Pre-1.6 surface pixel access routines. */
# define AG_CLIPPED_PIXEL(s, x,y) AG_SurfaceClipped((s), (x),(y))
# define AG_GET_PIXEL(s, p)       AG_SurfaceGet32_At((s), (p))
# define AG_GET_PIXEL2(s, x,y)    AG_SurfaceGet32((s), (x),(y))
# define AG_PUT_PIXEL(s, p, c)    AG_SurfacePut32_At((s), (p), (c))
# define AG_PUT_PIXEL2(s, x,y, c) AG_SurfacePut32((s), (x),(y), (c))
# define AG_PUT_PIXEL2_CLIPPED(s, x,y, c) \
	if (!AG_SurfaceClipped((s), (x),(y))) \
		AG_SurfacePut32((s), (x),(y), (c))
# define AG_BLEND_RGBA(s, p, r,g,b,a, fn) \
         AG_SurfaceBlendRGB8_At((s), (p), (r),(g),(b),(a), (fn))
# define AG_BLEND_RGBA2(s, x,y, r,g,b,a, fn) \
         AG_SurfaceBlendRGB8((s), (x),(y), (r),(g),(b),(a), (fn))
# define AG_BLEND_RGBA2_CLIPPED(s, x,y, r,g,b,a, fn) \
	if (!AG_SurfaceClipped((s), (x),(y))) \
		AG_SurfaceBlendRGB8((s), (x),(y), (r),(g),(b),(a), (fn))
void        AG_SurfaceLock(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_SurfaceUnlock(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
Uint32      AG_MapRGB(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32_RGB8(pf, r,g,b) */
Uint32      AG_MapRGBA(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32_RGBA8(pf, r,g,b,a) */
Uint32      AG_MapPixelRGB(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32_RGB8(pf, r,g,b) */
Uint32      AG_MapPixelRGBA(const AG_PixelFormat *_Nonnull, Uint8, Uint8, Uint8, Uint8) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32_RGBA8(pf, r,g,b,a) */
Uint32      AG_MapColorRGB(const AG_PixelFormat *_Nonnull, AG_Color) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32(pf, c); */
Uint32      AG_MapColorRGBA(const AG_PixelFormat *_Nonnull, AG_Color) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_MapPixel32(pf, c) */
void        AG_GetRGB(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_GetPixelRGB(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       => AG_GetColor32_RGB8(px, pf, r,g,b) */
void        AG_GetRGBA(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
void        AG_GetPixelRGBA(Uint32, const AG_PixelFormat *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       => AG_GetColor32_RGBA8(px, pf, r,g,b,a) */
AG_Color    AG_GetColorRGB(Uint32, const AG_PixelFormat *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       => AG_GetColor32(px, pf), c.a=AG_OPAQUE */
AG_Color    AG_GetColorRGBA(Uint32, const AG_PixelFormat *_Nonnull) DEPRECATED_ATTRIBUTE;
/*       => AG_GetColor32(px, pf) */
AG_Surface *_Nonnull AG_DupSurface(AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE;
/*                => AG_SurfaceDup(s) */
int         AG_SamePixelFmt(const AG_Surface *_Nonnull, const AG_Surface *_Nonnull) DEPRECATED_ATTRIBUTE _Pure_Attribute;
/*       => AG_PixelFormatCompare(a,b) */
int         AG_ScaleSurface(const AG_Surface *_Nonnull, Uint16, Uint16, AG_Surface *_Nonnull *_Nullable) DEPRECATED_ATTRIBUTE;
/*       => AG_SurfaceScale(s, w,h, 0) */
AG_Surface *_Nonnull AG_SurfaceStdGL(Uint, Uint) _Warn_Unused_Result DEPRECATED_ATTRIBUTE;
/*                => AG_SurfaceStdRGBA(w,h) */

#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SURFACE_H_ */
