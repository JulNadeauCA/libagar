/*	$Csoft: ttf.h,v 1.6 2005/05/10 12:25:54 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_LOADER_TTF_H_
#define _AGAR_LOADER_TTF_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include "begin_code.h"

typedef struct ag_ttf_glyph {
	int stored;
#define TTF_CACHED_METRICS	0x10
#define TTF_CACHED_BITMAP	0x01
#define TTF_CACHED_PIXMAP	0x02
	FT_UInt index;
	FT_Bitmap bitmap;
	FT_Bitmap pixmap;
	int minx, maxx;
	int miny, maxy;
	int yoffset;
	int advance;
	Uint32 cached;
} AG_TTFGlyph;

typedef struct ag_ttf_font {
	FT_Face	face;
	int height;
	int ascent;
	int descent;
	int lineskip;
	int style;
	int glyph_overhang;
	float glyph_italics;
	int underline_offset;
	int underline_height;

	AG_TTFGlyph *current;
	AG_TTFGlyph cache[256];	/* Transform cache */
	AG_TTFGlyph scratch;
	
	int font_size_family;		/* For non-scalable formats */
} AG_TTFFont;

__BEGIN_DECLS
int		 AG_TTFInit(void);
void		 AG_TTFDestroy(void);
AG_TTFFont	*AG_TTFOpenFont(const char *, int);
void		 AG_TTFCloseFont(AG_TTFFont *);
int		 AG_TTFFindGlyph(AG_TTFFont *, Uint32, int);

__inline__ int	 AG_TTFGetFontStyle(AG_TTFFont *);
__inline__ void	 AG_TTFSetFontStyle(AG_TTFFont *, int);
#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02
#define TTF_STYLE_UNDERLINE	0x04

__inline__ int	 AG_TTFHeight(AG_TTFFont *);
__inline__ int	 AG_TTFAscent(AG_TTFFont *);
__inline__ int	 AG_TTFDescent(AG_TTFFont *);
__inline__ int	 AG_TTFLineSkip(AG_TTFFont *);
__inline__ int	 AG_TTFFaceFixedWidth(AG_TTFFont *);
__inline__ char	*AG_TTFFaceFamily(AG_TTFFont *);
__inline__ char	*AG_TTFFaceStyle(AG_TTFFont *);

int AG_TTFGlyphMetrics(AG_TTFFont *, Uint32 , int *, int *, int *, int *,
		      int *);
int AG_TTFSizeText(AG_TTFFont *, const char *, int *, int *);
int AG_TTFSizeUnicode(AG_TTFFont *, const Uint32 *, int *, int *);

SDL_Surface *AG_TTFRenderTextSolid(AG_TTFFont *, const char *, SDL_Color);
SDL_Surface *AG_TTFRenderUnicodeSolid(AG_TTFFont *, const Uint32 *,
		                      SDL_Color *, SDL_Color);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_TTF_H_ */
